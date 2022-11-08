/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Intel Corporation
 */

#include "sample_util.h"

#include <getopt.h>
#include <inttypes.h>

enum sample_args_cmd {
  SAMPLE_ARG_UNKNOWN = 0,

  SAMPLE_ARG_P_PORT = 0x100, /* start from end of ascii */
  SAMPLE_ARG_R_PORT,
  SAMPLE_ARG_P_TX_IP,
  SAMPLE_ARG_R_TX_IP,
  SAMPLE_ARG_P_RX_IP,
  SAMPLE_ARG_R_RX_IP,
  SAMPLE_ARG_P_SIP,
  SAMPLE_ARG_R_SIP,
  SAMPLE_ARG_P_FWD_IP,
  SAMPLE_ARG_LOG_LEVEL,
  SAMPLE_ARG_DEV_AUTO_START,

  SAMPLE_ARG_TX_VIDEO_URL = 0x200,
  SAMPLE_ARG_RX_VIDEO_URL,
  SAMPLE_ARG_LOGO_URL,
  SAMPLE_ARG_WIDTH,
  SAMPLE_ARG_HEIGHT,
  SAMPLE_ARG_SESSIONS_CNT,
  SAMPLE_ARG_EXT_FRAME,
  SAMPLE_ARG_MAX,
};

static struct option sample_args_options[] = {
    {"p_port", required_argument, 0, SAMPLE_ARG_P_PORT},
    {"r_port", required_argument, 0, SAMPLE_ARG_R_PORT},
    {"p_tx_ip", required_argument, 0, SAMPLE_ARG_P_TX_IP},
    {"r_tx_ip", required_argument, 0, SAMPLE_ARG_R_TX_IP},
    {"p_rx_ip", required_argument, 0, SAMPLE_ARG_P_RX_IP},
    {"r_rx_ip", required_argument, 0, SAMPLE_ARG_R_RX_IP},
    {"p_sip", required_argument, 0, SAMPLE_ARG_P_SIP},
    {"r_sip", required_argument, 0, SAMPLE_ARG_R_SIP},
    {"p_fwd_ip", required_argument, 0, SAMPLE_ARG_P_FWD_IP},
    {"sessions_cnt", required_argument, 0, SAMPLE_ARG_SESSIONS_CNT},
    {"log_level", required_argument, 0, SAMPLE_ARG_LOG_LEVEL},
    {"dev_auto_start", no_argument, 0, SAMPLE_ARG_DEV_AUTO_START},

    {"tx_video_url", required_argument, 0, SAMPLE_ARG_TX_VIDEO_URL},
    {"rx_video_url", required_argument, 0, SAMPLE_ARG_TX_VIDEO_URL},
    {"logo_url", required_argument, 0, SAMPLE_ARG_LOGO_URL},
    {"width", required_argument, 0, SAMPLE_ARG_WIDTH},
    {"height", required_argument, 0, SAMPLE_ARG_HEIGHT},
    {"ext_frame", no_argument, 0, SAMPLE_ARG_EXT_FRAME},

    {0, 0, 0, 0}};

static int st_sample_parse_args(struct st_sample_context* ctx, int argc, char** argv) {
  int cmd = -1, optIdx = 0;
  struct st_init_params* p = &ctx->param;

  while (1) {
    cmd = getopt_long_only(argc, argv, "hv", sample_args_options, &optIdx);
    if (cmd == -1) break;
    dbg("%s, cmd %d %s\n", __func__, cmd, optarg);

    switch (cmd) {
      case SAMPLE_ARG_P_PORT:
        snprintf(p->port[ST_PORT_P], sizeof(p->port[ST_PORT_P]), "%s", optarg);
        p->num_ports++;
        break;
      case SAMPLE_ARG_R_PORT:
        snprintf(p->port[ST_PORT_R], sizeof(p->port[ST_PORT_R]), "%s", optarg);
        p->num_ports++;
        break;
      case SAMPLE_ARG_P_SIP:
        inet_pton(AF_INET, optarg, st_p_sip_addr(p));
        break;
      case SAMPLE_ARG_R_SIP:
        inet_pton(AF_INET, optarg, st_r_sip_addr(p));
        break;
      case SAMPLE_ARG_P_TX_IP:
        inet_pton(AF_INET, optarg, ctx->tx_dip_addr[ST_PORT_P]);
        break;
      case SAMPLE_ARG_R_TX_IP:
        inet_pton(AF_INET, optarg, ctx->tx_dip_addr[ST_PORT_R]);
        break;
      case SAMPLE_ARG_P_RX_IP:
        inet_pton(AF_INET, optarg, ctx->rx_sip_addr[ST_PORT_P]);
        break;
      case SAMPLE_ARG_R_RX_IP:
        inet_pton(AF_INET, optarg, ctx->rx_sip_addr[ST_PORT_R]);
        break;
      case SAMPLE_ARG_P_FWD_IP:
        inet_pton(AF_INET, optarg, ctx->fwd_dip_addr[ST_PORT_P]);
        break;
      case SAMPLE_ARG_LOG_LEVEL:
        if (!strcmp(optarg, "debug"))
          p->log_level = ST_LOG_LEVEL_DEBUG;
        else if (!strcmp(optarg, "info"))
          p->log_level = ST_LOG_LEVEL_INFO;
        else if (!strcmp(optarg, "notice"))
          p->log_level = ST_LOG_LEVEL_NOTICE;
        else if (!strcmp(optarg, "warning"))
          p->log_level = ST_LOG_LEVEL_WARNING;
        else if (!strcmp(optarg, "error"))
          p->log_level = ST_LOG_LEVEL_ERROR;
        else
          err("%s, unknow log level %s\n", __func__, optarg);
        break;
      case SAMPLE_ARG_DEV_AUTO_START:
        p->flags |= ST_FLAG_DEV_AUTO_START_STOP;
        break;
      case SAMPLE_ARG_TX_VIDEO_URL:
        snprintf(ctx->tx_url, sizeof(ctx->tx_url), "%s", optarg);
        break;
      case SAMPLE_ARG_RX_VIDEO_URL:
        snprintf(ctx->rx_url, sizeof(ctx->rx_url), "%s", optarg);
        break;
      case SAMPLE_ARG_LOGO_URL:
        snprintf(ctx->logo_url, sizeof(ctx->rx_url), "%s", optarg);
        break;
      case SAMPLE_ARG_WIDTH:
        ctx->width = atoi(optarg);
        break;
      case SAMPLE_ARG_HEIGHT:
        ctx->height = atoi(optarg);
        break;
      case SAMPLE_ARG_SESSIONS_CNT:
        ctx->sessions = atoi(optarg);
        break;
      case SAMPLE_ARG_EXT_FRAME:
        ctx->ext_frame = true;
        break;
      case '?':
        break;
      default:
        break;
    }
  };

  return 0;
}

static struct st_sample_context* g_sample_ctx;
static void sample_sig_handler(int signo) {
  struct st_sample_context* ctx = g_sample_ctx;
  info("%s, signal %d\n", __func__, signo);

  switch (signo) {
    case SIGINT: /* Interrupt from keyboard */
      ctx->exit = true;
      if (ctx->st) st_request_exit(ctx->st);
      break;
  }

  return;
}

int st_sample_init(struct st_sample_context* ctx, int argc, char** argv, bool tx,
                   bool rx) {
  struct st_init_params* p = &ctx->param;
  memset(ctx, 0, sizeof(*ctx));

  g_sample_ctx = ctx;
  signal(SIGINT, sample_sig_handler);

  p->flags |= ST_FLAG_BIND_NUMA; /* default bind to numa */
  p->flags |= ST_FLAG_RX_SEPARATE_VIDEO_LCORE;
  p->log_level = ST_LOG_LEVEL_INFO; /* default to info */
  /* use different default port/ip for tx and rx */
  if (rx) {
    strncpy(p->port[ST_PORT_P], "0000:af:01.0", ST_PORT_MAX_LEN);
    inet_pton(AF_INET, "192.168.85.80", st_p_sip_addr(p));
  } else {
    strncpy(p->port[ST_PORT_P], "0000:af:01.1", ST_PORT_MAX_LEN);
    inet_pton(AF_INET, "192.168.85.81", st_p_sip_addr(p));
  }
  inet_pton(AF_INET, "239.168.85.20", ctx->tx_dip_addr[ST_PORT_P]);
  inet_pton(AF_INET, "239.168.85.20", ctx->rx_sip_addr[ST_PORT_P]);
  inet_pton(AF_INET, "239.168.85.21", ctx->fwd_dip_addr[ST_PORT_P]);

  if (!ctx->sessions) ctx->sessions = 1;
  ctx->framebuff_cnt = 3;
  ctx->width = 1920;
  ctx->height = 1080;
  ctx->fps = ST_FPS_P59_94;
  ctx->fmt = ST20_FMT_YUV_422_10BIT;
  ctx->input_fmt = ST_FRAME_FMT_YUV422RFC4175PG2BE10;
  ctx->output_fmt = ST_FRAME_FMT_YUV422RFC4175PG2BE10;
  ctx->st22p_input_fmt = ST_FRAME_FMT_YUV422PLANAR10LE;
  ctx->st22p_output_fmt = ST_FRAME_FMT_YUV422PLANAR10LE;
  ctx->udp_port = 20000;
  ctx->payload_type = 112;
  strncpy(ctx->tx_url, "test.yuv", sizeof(ctx->tx_url));
  strncpy(ctx->rx_url, "rx.yuv", sizeof(ctx->rx_url));

  strncpy(ctx->logo_url, "logo.yuv", sizeof(ctx->rx_url));
  ctx->logo_width = 200;
  ctx->logo_height = 200;

  st_sample_parse_args(ctx, argc, argv);

  if (tx) p->tx_sessions_cnt_max = ctx->sessions;
  if (rx) p->rx_sessions_cnt_max = ctx->sessions;
  /* always enable 1 port */
  if (!p->num_ports) p->num_ports = 1;

  return 0;
}

int st_sample_start(struct st_sample_context* ctx) {
  struct st_init_params* p = &ctx->param;

  /* create device */
  ctx->st = st_init(p);
  if (!ctx->st) {
    err("%s, st init fail\n", __func__);
    return -EIO;
  }

  return 0;
}

int st_sample_uinit(struct st_sample_context* ctx) {
  /* destroy device */
  if (ctx->st) {
    st_uninit(ctx->st);
    ctx->st = NULL;
  }
  return 0;
}

int st_sample_tx_init(struct st_sample_context* ctx, int argc, char** argv) {
  st_sample_init(ctx, argc, argv, true, false);
  return st_sample_start(ctx);
};

int st_sample_rx_init(struct st_sample_context* ctx, int argc, char** argv) {
  st_sample_init(ctx, argc, argv, false, true);
  return st_sample_start(ctx);
};

int st_sample_fwd_init(struct st_sample_context* ctx, int argc, char** argv) {
  st_sample_init(ctx, argc, argv, true, true);
  return st_sample_start(ctx);
};