#ifndef __GBT_H
#define __GBT_H

#include <stdint.h>

#define GBT_VERSION (0xAA)

#define GBT_ACK (0x79)
#define GBT_NACK (0x1F)

#define GBT_NUM_CMDS (6)
#define GBT_NUM_ADDR_CS (5)

#define GBT_CMD_GET (0x00)
#define GBT_CMD_GET_ID (0x02)
#define GBT_CMD_READ_MEM (0x11)
#define GBT_CMD_WRITE_MEM (0x31)
#define GBT_CMD_GO (0x21)
#define GBT_CMD_ERASE (0x43)

#define GBT_STAGE_READ_NUMBYTES_CHS (0x01)
#define GBT_STAGE_READ_BYTES_CHS (0x01)
#define GBT_STAGE_READ_START_ADDR_CHS (0x02)

typedef void (callbackOut_t)(uint8_t *buf, int32_t len);

typedef enum {
    STATE_WAIT_CMD = 0,
    STATE_CHECK_CMD_GET,
    STATE_CMD_GET,
    STATE_CHECK_CMD_WM,
    STATE_CMD_WM,
    STATE_CMD_WM_RECV_SADDR_CS,
    STATE_CMD_WM_RECV_NUM_DATA,
    STATE_CMD_WM_RECV_DATA,
    STATE_CHECK_CMD_RM
} gbt_state_t;

typedef struct {
  gbt_state_t state;
  uint8_t command;
  uint32_t numState;
  uint32_t recvIndex;
  uint32_t recvLen;
  uint8_t *recvBuf;
  
  callbackOut_t *outFunc;
} gbt_t;



void gbt_init(gbt_t *gbt);
void gbt_in(gbt_t *gbt, uint8_t *buf, int32_t *len);
void gbt_addCallbackOut(gbt_t *gbt, callbackOut_t *callback);

void gbt_setStartAddress(gbt_t *gbt, uint32_t addr);

/**********************/
static void dummyOut(uint8_t *buf, int32_t len);
static void parcer(gbt_t *gbt, uint8_t byte);
static void sendACK(gbt_t *gbt);
static void sendNACK(gbt_t *gbt);
static void sendLength(gbt_t *gbt, uint8_t len);
static void sendVersion(gbt_t *gbt);
static void sendCommandsList(gbt_t *gbt);

static uint8_t isRdpInactive(gbt_t *gbt);

static void setBuffNum(gbt_t *gbt, uint32_t num);
static uint8_t putBuff(gbt_t *gbt, uint8_t data);
static uint8_t xorVerify(uint8_t *buff, uint32_t num, uint8_t checksum);

#endif