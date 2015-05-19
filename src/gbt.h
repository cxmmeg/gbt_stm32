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
typedef uint32_t (callbackMemRW_t)(uint32_t startAddress, uint8_t *buff, uint32_t len);

typedef struct {
  callbackOut_t *outFunc;
  callbackMemRW_t *memRead;
  callbackMemRW_t *memWrite;
  callbackMemRW_t *memClear;
}gbt_handlers_t;

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
  uint32_t recvBufLength;
  gbt_handlers_t *handlers;

} gbt_t;



/**
 * Инициализация переменной gbt
 * @param gbt
 */
void gbt_init(gbt_t *gbt, uint8_t *rxbuf, uint32_t rxBufLen, gbt_handlers_t *handlers);
void gbt_in(gbt_t *gbt, uint8_t *buf, uint32_t len);

/**
 * Установка калбака выходного потока данных на последовательное устройство
 * @param gbt
 * @param callback обработчик в формате void (callbackOut_t)(uint8_t *buf, int32_t len)
 */
void gbt_addCallbackOut(gbt_t *gbt, callbackOut_t *callback);

/* Запись полученных данных в память */
/**
 * Запись полученных данных в память
 * @param gbt
 * @param buff начало буфера для записи
 * @param len длина записываемых данных
 */
uint32_t gbt_write(uint32_t startAddress, uint8_t *buff, uint32_t len);

/**********************/

static void __outFunc(gbt_t *gbt,uint8_t *buf, int32_t len);
static uint32_t __memRead(gbt_t *gbt, uint32_t startAddress, uint8_t *buff, uint32_t len);
static uint32_t __memWrite(gbt_t *gbt, uint32_t startAddress, uint8_t *buff, uint32_t len);

static void dummyOut(uint8_t *buf, int32_t len);
static void parcer(gbt_t *gbt, uint8_t byte);
static void sendACK(gbt_t *gbt);
static void sendNACK(gbt_t *gbt);
static void sendLength(gbt_t *gbt, uint8_t len);
static void sendVersion(gbt_t *gbt);
static void sendCommandsList(gbt_t *gbt);

static uint8_t isRdpInactive(gbt_t *gbt);

static uint32_t setBuffNum(gbt_t *gbt, uint32_t num);
static uint8_t putBuff(gbt_t *gbt, uint8_t data);
//static uint8_t xorVerify(uint8_t *buff, uint32_t num, uint8_t checksum);
static uint8_t xorVerify(gbt_t *gbt);

#endif