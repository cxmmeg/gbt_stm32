#ifndef __GBT_H
#define __GBT_H

#define GBT_VERSION (0xAA)

#define GBT_ACK (0x79)
#define GBT_NACK (0x1F)

#define GBT_NUM_CMDS (0x06)

#define GBT_CMD_GET (0x00)
#define GBT_CMD_GET_ID (0x02)
#define GBT_CMD_READ_MEM (0x11)
#define GBT_CMD_WRITE_MEM (0x31)
#define GBT_CMD_GO (0x21)
#define GBT_CMD_ERASE (0x43)

#define GBT_STAGE_READ_NUMBYTES_CHS (0x01)
#define GBT_STAGE_READ_BYTES_CHS (0x01)
#define GBT_STAGE_READ_START_ADDR_CHS (0x02)

typedef void (callbackOut_t)(unsigned char *buf, int len);

typedef struct {
  unsigned char state;
  unsigned char command;
  unsigned char stage;
  unsigned int recvIndex;
  unsigned int recvLen;
  unsigned char *recvBuf;
  
  callbackOut_t *outFunc;
} gbt_t;

void gbt_init(gbt_t *gbt);
void gbt_in(gbt_t *gbt, unsigned char *buf, int *len);
void gbt_addCallbackOut(gbt_t *gbt, callbackOut_t *callback);
/*extern void gbt_out(unsigned char *buf, int len);*/

/**********************/
static void dummyOut(unsigned char *buf, int len);
static void parcer(gbt_t *gbt, unsigned char byte);
static void sendACK(gbt_t *gbt);
static void sendNACK(gbt_t *gbt);
static void sendLength(gbt_t *gbt, unsigned char len);
static void sendVersion(gbt_t *gbt);
static void sendCommandsList(gbt_t *gbt);
#endif