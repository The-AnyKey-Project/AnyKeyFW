// SPDX-License-Identifier: GPL-3.0
/*
 * AnyKey hidraw communication example
 *
 * Copyright (c) 2021 Matthias Beckert
 */

#include "main.h"

static anykey_action_t _argp_cmdstr_to_action(char *cmd);
static error_t _argp_parser(int key, char *arg, struct argp_state *state);
static int _hidraw_send_buffer(int fd, uint8_t *buf, cli_args_t *args);
static int _hidraw_recv_buffer(int fd, uint8_t *buf, cli_args_t *args);
static void _out_req_printf(anykey_cmd_t cmd, char *params, cli_args_t *args);
static void _out_resp_printf(anykey_cmd_t cmd, char *params, cli_args_t *args);
static void _out_sprintf_buffer(uint8_t *dst, const char *fmt, uint8_t *src, uint32_t size,
                                const char *start, const char *stop);
static void _cb_set_layer(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_get_layer(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_set_contrast(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_get_contrast(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_get_flash_info(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_set_flash(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_get_flash(int fd, uint8_t *buf, cli_args_t *args);
static void _cb_cmd_error(int fd, uint8_t *buf, cli_args_t *args);

static char _arpg_doc[] =
    "Simple test program to verify HID raw communication between Anykey firmware and host pc";

static struct argp_option _argp_options[] = {
    {"device", 'D', "DEVIE", 0, "HID raw device to be used"},
    {"command", 'C', "CMD", 0, "Command to be send"},
    {"verbose", 'v', 0, 0, "Verbose output"},
    {"quiet", 'q', 0, 0, "No output"},
    {0, 0, 0, 0, "Additional options for 'set-layer' command"},
    {"layer", 'l', "NAME", 0, "Layer to be set"},
    {0, 0, 0, 0, "Additional options for 'set-contrast' command"},
    {"display", 'd', "ID", 0, "Display id (0..8), use 9 to address all displays"},
    {"contrast", 'c', "VALUE", 0, "Contrast to be set (0..255)"},
    {0, 0, 0, 0, "Additional options for 'get-contrast' command"},
    {"display", 'd', "ID", 0, "Display id(0..8), use 9 to address all displays"},
    {0, 0, 0, 0, "Additional options for 'set-flash' command"},
    {"file", 'f', "FILE", 0, "Input file, default is out.bin"},
    {0, 0, 0, 0, "Additional options for 'get-flash' command"},
    {"file", 'f', "FILE", 0, "Output file, default is out.bin"},
    {0},
};

static const char const *_argp_cmd_str[] = {
    "set-layer",      "get-layer", "set-contrast", "get-contrast",
    "get-flash-info", "set-flash", "get-flash",
};

static struct argp _argp = {_argp_options, _argp_parser, 0, _arpg_doc, 0, 0, 0};

static const action_callback action_callback_list[] = {
    _cb_set_layer,      _cb_get_layer, _cb_set_contrast, _cb_get_contrast,
    _cb_get_flash_info, _cb_set_flash, _cb_get_flash,    _cb_cmd_error,
};

static const char const *glcdidstrings[] = {
    "GLCD_DISP_1", "GLCD_DISP_2", "GLCD_DISP_3", "GLCD_DISP_4", "GLCD_DISP_5",
    "GLCD_DISP_6", "GLCD_DISP_7", "GLCD_DISP_8", "GLCD_DISP_9", "GLCD_DISP_MAX",
};

static uint32_t flash_size = 0;
static uint32_t sector_size = 0;

static anykey_action_t _argp_cmdstr_to_action(char *cmd)
{
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_SET_LAYER], cmd) == 0) return ANYKEY_CMD_SET_LAYER;
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_GET_LAYER], cmd) == 0) return ANYKEY_CMD_GET_LAYER;
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_SET_CONTRAST], cmd) == 0) return ANYKEY_CMD_SET_CONTRAST;
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_GET_CONTRAST], cmd) == 0) return ANYKEY_CMD_GET_CONTRAST;
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_GET_FLASH_INFO], cmd) == 0) return ANYKEY_CMD_GET_FLASH_INFO;
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_SET_FALSH], cmd) == 0) return ANYKEY_CMD_SET_FALSH;
  if (strcmp(_argp_cmd_str[ANYKEY_CMD_GET_FALSH], cmd) == 0) return ANYKEY_CMD_GET_FALSH;
  return ANYKEY_CMD_ERR;
}

static error_t _argp_parser(int key, char *arg, struct argp_state *state)
{
  cli_args_t *arguments = state->input;

  switch (key)
  {
    case 'D':
      arguments->D = arg;
      break;
    case 'C':
      arguments->C = _argp_cmdstr_to_action(arg);
      break;
    case 'l':
      arguments->l = arg;
      break;
    case 'd':
    {
      int tmp = atoi(arg);
      arguments->d = (tmp < 0) ? 0 : ((tmp > 9) ? 9 : tmp);
      break;
    }
    case 'c':
    {
      int tmp = atoi(arg);
      arguments->c = (tmp < 0) ? 0 : ((tmp > 255) ? 255 : tmp);
      break;
    }
    case 'f':
      arguments->f = arg;
      break;
    case 'v':
      arguments->v = 1;
      break;
    case 'q':
      arguments->q = 1;
      break;
    case ARGP_KEY_ARG:
      return 0;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static int _hidraw_send_buffer(int fd, uint8_t *buf, cli_args_t *args)
{
  /*
   * buf[0] is not used for actual payload according to:
   * https://www.kernel.org/doc/html/latest/hid/hidraw.html
   */
  int res = write(fd, buf, USB_HID_RAW_EPSIZE + 1);

  if (res < 0)
  {
    char err[64];
    sprintf(err, "write() error: %d\n", errno);
    perror(err);
  }
  else
  {
    if (args->v)
    {
      printf("write() wrote %d bytes\n", (res - 1));
    }
  }
  return res;
}

static int _hidraw_recv_buffer(int fd, uint8_t *buf, cli_args_t *args)
{
  int res = read(fd, buf, USB_HID_RAW_EPSIZE);

  if (res < 0)
  {
    char err[64];
    sprintf(err, "read() error: %d\n", errno);
    perror(err);
  }
  else
  {
    if (args->v)
    {
      printf("read() read %d bytes\n", res);
    }
  }
  return res;
}

static void _out_req_printf(anykey_cmd_t cmd, char *params, cli_args_t *args)
{
  if (!args->q)
  {
    printf("Send: %s %s\n", _argp_cmd_str[cmd], params);
  }
}

static void _out_resp_printf(anykey_cmd_t cmd, char *params, cli_args_t *args)
{
  if (!args->q)
  {
    printf("Recv: %s %s\n", _argp_cmd_str[cmd], params);
  }
}

static void _out_sprintf_buffer(uint8_t *dst, const char *fmt, uint8_t *src, uint32_t size,
                                const char *start, const char *stop)
{
  uint32_t i = 0;

  sprintf(dst, "%s%s", dst, start);
  for (i = 0; i < size; i++)
  {
    sprintf(dst, fmt, dst, src[i]);
  }
  sprintf(dst, "%s%s", dst, stop);
}

static void _cb_set_layer(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_set_layer_req_t *req = (anykey_cmd_set_layer_req_t *)&buf[1];
  if (args->l == NULL)
  {
    perror("Please specify layer with -l\n");
    return;
  }

  req->cmd = args->C;
  memcpy(req->name, args->l, strlen(args->l));
  _out_req_printf(req->cmd, req->name, args);
  _hidraw_send_buffer(fd, buf, args);
}

static void _cb_get_layer(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_get_layer_req_t *req = (anykey_cmd_get_layer_req_t *)&buf[1];
  anykey_cmd_get_layer_resp_t *resp = (anykey_cmd_get_layer_resp_t *)buf;

  req->cmd = args->C;

  _out_req_printf(req->cmd, "\0", args);
  int res = _hidraw_send_buffer(fd, buf, args);

  if (res > 0)
  {
    res = _hidraw_recv_buffer(fd, buf, args);
    if (res > 0)
    {
      _out_resp_printf(resp->cmd, resp->name, args);
    }
  }
}

static void _cb_set_contrast(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_set_contrast_req_t *req = (anykey_cmd_set_contrast_req_t *)&buf[1];
  uint8_t i = 0;
  char params_printf[64];

  req->cmd = args->C;
  req->display = args->d;
  sprintf(params_printf, "%s", glcdidstrings[args->d]);
  for (i = 0; i < GLCD_DISP_MAX; i++)
  {
    req->contrast[i] = (args->d == i || args->d == GLCD_DISP_MAX) ? args->c : 0;
    sprintf(params_printf, "%s %d", params_printf, req->contrast[i]);
  }

  _out_req_printf(req->cmd, params_printf, args);
  _hidraw_send_buffer(fd, buf, args);
}

static void _cb_get_contrast(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_get_contrast_req_t *req = (anykey_cmd_get_contrast_req_t *)&buf[1];
  anykey_cmd_get_contrast_resp_t *resp = (anykey_cmd_get_contrast_resp_t *)buf;
  uint8_t i = 0;
  char params_printf[64];

  req->cmd = args->C;
  req->display = args->d;
  sprintf(params_printf, "%s", glcdidstrings[args->d]);

  _out_req_printf(req->cmd, params_printf, args);
  int res = _hidraw_send_buffer(fd, buf, args);

  if (res > 0)
  {
    res = _hidraw_recv_buffer(fd, buf, args);
    if (res > 0)
    {
      params_printf[0] = '\0';
      for (i = 0; i < GLCD_DISP_MAX; i++)
      {
        sprintf(params_printf, "%s %d", params_printf, resp->contrast[i]);
      }
      _out_resp_printf(resp->cmd, params_printf, args);
    }
  }
}

void _cb_get_flash_info(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_get_flash_info_req_t *req = (anykey_cmd_get_flash_info_req_t *)&buf[1];
  anykey_cmd_get_flash_info_resp_t *resp = (anykey_cmd_get_flash_info_resp_t *)buf;
  char params_printf[64];

  req->cmd = args->C;

  _out_req_printf(req->cmd, "\0", args);
  int res = _hidraw_send_buffer(fd, buf, args);

  if (res > 0)
  {
    res = _hidraw_recv_buffer(fd, buf, args);
    if (res > 0)
    {
      sprintf(params_printf, "Flash size: %d, Sector size: %d", resp->flash_size,
              resp->sector_size);
      _out_resp_printf(resp->cmd, params_printf, args);
      flash_size = resp->flash_size;
      sector_size = resp->sector_size;
    }
  }
}

static void _cb_set_flash(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_set_flash_req_t *req = (anykey_cmd_set_flash_req_t *)&buf[1];
  anykey_cmd_set_flash_resp_t *resp = (anykey_cmd_set_flash_resp_t *)buf;

  cli_args_t dummy_args = {.C = ANYKEY_CMD_GET_FLASH_INFO, .v = args->v, .q = args->q};
  char params_printf[256];

  _cb_get_flash_info(fd, buf, &dummy_args);

  if (flash_size && sector_size)
  {
    uint32_t sectors = flash_size / sector_size;
    uint32_t i = 0;
    uint8_t *sector = malloc(sector_size * sizeof(uint8_t));
    int input_fd = open(args->f, O_RDONLY);

    for (i = 0; i < sectors; i++)
    {
      read(input_fd, sector, sector_size * sizeof(uint8_t));

      uint16_t block_size = sizeof(req->buffer);
      uint16_t block_cnt = 0;
      uint16_t block_cnt_max = (sector_size - 1) / block_size + 1;
      uint16_t residual = sector_size;

      for (block_cnt = 0; block_cnt < block_cnt_max; block_cnt++)
      {
        buf[0] = 0;

        req->cmd = args->C;
        req->sector = i;
        req->block_cnt = block_cnt;
        req->block_size = (residual > block_size) ? block_size : residual;
        residual -= req->block_size;
        req->final_block = (residual) ? 0 : 1;
        memcpy(req->buffer, &sector[block_size * req->block_cnt], req->block_size);

        sprintf(params_printf, "Sector %d", req->sector);
        if (args->v)
        {
          sprintf(params_printf, "%s, Block %d, Size %d, Final %d, Payload: ", params_printf,
                  req->block_cnt, req->block_size, req->final_block);
          _out_sprintf_buffer(params_printf, "%s %02X", req->buffer, req->block_size, "[", " ]");
        }
        if (!req->block_cnt || args->v)
        {
          _out_req_printf(req->cmd, params_printf, args);
        }

        int res = _hidraw_send_buffer(fd, buf, args);
        if (res > 0)
        {
          res = _hidraw_recv_buffer(fd, buf, args);
          if (res > 0)
          {
            if (resp->sector != i || resp->block_cnt != block_cnt ||
                resp->final_block != ((residual) ? 0 : 1))
            {
              close(input_fd);
              free(sector);
              perror("Received broken block confirmation, abort!");
            }
            sprintf(params_printf, "Sector %d", resp->sector);
            if (args->v)
            {
              sprintf(params_printf, "%s, Block %d, Final %d", params_printf, resp->block_cnt,
                      resp->final_block);
            }
            if (!resp->block_cnt || args->v)
            {
              _out_resp_printf(resp->cmd, params_printf, args);
            }
          }
        }
      }
    }
    close(input_fd);
    free(sector);
  }
}

static void _cb_get_flash(int fd, uint8_t *buf, cli_args_t *args)
{
  anykey_cmd_get_flash_req_t *req = (anykey_cmd_get_flash_req_t *)&buf[1];
  anykey_cmd_get_flash_resp_t *resp = (anykey_cmd_get_flash_resp_t *)buf;

  cli_args_t dummy_args = {.C = ANYKEY_CMD_GET_FLASH_INFO, .v = args->v, .q = args->q};
  char params_printf[256];

  _cb_get_flash_info(fd, buf, &dummy_args);

  if (flash_size && sector_size)
  {
    uint32_t sectors = flash_size / sector_size;
    uint32_t i = 0;
    uint8_t *sector = malloc(sector_size * sizeof(uint8_t));
    int output_fd = open(args->f, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    for (i = 0; i < sectors; i++)
    {
      buf[0] = 0;

      req->cmd = args->C;
      req->sector = i;

      sprintf(params_printf, "Sector %d", req->sector);
      _out_req_printf(req->cmd, params_printf, args);
      int res = _hidraw_send_buffer(fd, buf, args);

      if (res > 0)
      {
        uint32_t sector_idx = 0;
        do
        {
          uint32_t n = 0;
          res = _hidraw_recv_buffer(fd, buf, args);
          if (res > 0)
          {
            memcpy(&sector[sector_idx], resp->buffer, resp->block_size);
            sector_idx += resp->block_size;
            sprintf(params_printf, "Sector %d", i);
            if (args->v)
            {
              sprintf(params_printf, "%s, Block %d, Size %d, Final %d, Payload: ", params_printf,
                      resp->block_cnt, resp->block_size, resp->final_block);
              _out_sprintf_buffer(params_printf, "%s %02X", resp->buffer, resp->block_size, "[",
                                  " ]");
            }
            if (resp->final_block || args->v)
            {
              _out_resp_printf(resp->cmd, params_printf, args);
            }
          }
        } while (!resp->final_block && res > 0);
        write(output_fd, sector, sector_size * sizeof(uint8_t));
      }
    }

    close(output_fd);
    free(sector);
  }
}

static void _cb_cmd_error(int fd, uint8_t *buf, cli_args_t *args)
{
  (void)fd;
  (void)buf;
  (void)args;
  perror("Unknown command, allowed commands are : \n");
  int i = 0;
  char params_printf[256];
  for (i = 0; i < ANYKEY_CMD_ERR; i++)
  {
    sprintf(params_printf, "  %s\n", _argp_cmd_str[i]);
    perror(params_printf);
  }
}

int main(int argc, char **argv)
{
  int fd;
  uint8_t buf[USB_HID_RAW_EPSIZE + 1];
  memset(buf, 0, sizeof(buf));

  cli_args_t arguments = {
      .D = NULL,
      .C = ANYKEY_CMD_ERR,
      .l = NULL,
      .d = GLCD_DISP_MAX,
      .c = 0,
      .f = "out.bin",
      .v = 0,
      .q = 0,
  };

  argp_parse(&_argp, argc, argv, 0, 0, &arguments);

  if (arguments.D == NULL)
  {
    perror("Please specify device with -D\n");
    return 1;
  }

  fd = open(arguments.D, O_RDWR);

  if (fd < 0)
  {
    perror("Unable to open device");
    return 1;
  }

  action_callback_list[arguments.C](fd, buf, &arguments);

  close(fd);
  return 0;
}
