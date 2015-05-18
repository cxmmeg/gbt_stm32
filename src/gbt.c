#include "gbt.h"


#define __STATE_RECV_CMD (0)
#define __STATE_RECV_CMD_CMP (1)
#define __STATE_EXEC (2)

/* Handler API */
void gbt_init(gbt_t *gbt) {
    gbt->state = __STATE_RECV_CMD;
    gbt->command = GBT_CMD_GET;
    gbt->outFunc = dummyOut;
}

void gbt_in(gbt_t *gbt, uint8_t *buf, int *len) {
    int index = 0;
    while (index<*len) {
        parcer(gbt, buf[index]);
        index++;
    }
}

void gbt_addCallbackOut(gbt_t *gbt, callbackOut_t *callback) {
    gbt->outFunc = callback;
}

void gbt_setStartAddress(gbt_t *gbt, uint32_t addr){
    
}

/*
extern void gbt_out(uint8_t *buf, int len){
}
 */

/***************/

static void parcer(gbt_t *gbt, uint8_t data) {
    
    uint32_t startAddress=0;
    
    switch (gbt->state) {
            /*
             * STATE_WAIT_CMD
             */
        case STATE_WAIT_CMD:
            switch (data) {
                case GBT_CMD_GET:
                    gbt->state = STATE_CHECK_CMD_GET;
                    break;
                case GBT_CMD_WRITE_MEM:
                    gbt->state = STATE_CHECK_CMD_WM;
                    break;
                case GBT_CMD_READ_MEM:
                    gbt->state = STATE_CHECK_CMD_RM;
                    break;
                default:
                    sendNACK(gbt);
            }
            break;

            /*
             * STATE_CHECK_CMD_GET
             */
        case STATE_CHECK_CMD_GET:
            if (data == ~GBT_CMD_GET) {
                sendACK(gbt);
                sendLength(gbt, GBT_NUM_CMDS);
                sendVersion(gbt);
                sendCommandsList(gbt);
                sendACK(gbt);
            } else {
                sendNACK(gbt);
            }
            gbt->state = STATE_WAIT_CMD;
            break;

            /*
             * STATE_CHECK_CMD_WM
             */
        case STATE_CHECK_CMD_WM:
            if(data== ~GBT_CMD_WRITE_MEM){
                if(isRdpInactive(gbt)){
                    sendACK(gbt);
                    gbt->state = STATE_CMD_WM_RECV_SADDR_CS;
                    setBuffNum(gbt, GBT_NUM_ADDR_CS);
                }
                else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }        
            }            
            break;
            
            /*
             * STATE_CMD_WM_RECV_SADDR_CS
             */
        case STATE_CMD_WM_RECV_SADDR_CS:
            if (putBuff(gbt, data)) {
                /* Запись стартового адреса TODO!!!*/
                if (xorVerify(gbt->recvBuf, GBT_NUM_ADDR_CS - 1, gbt->recvBuf[GBT_NUM_ADDR_CS - 1])) {
                    startAddress = gbt->recvBuf[3] << 24 | gbt->recvBuf[2] << 16 | gbt->recvBuf[1] << 8 | gbt->recvBuf[0];
                    gbt_setStartAddress(gbt, startAddress);
                    gbt->state = STATE_CMD_WM_RECV_NUM_DATA;
                } else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }
            }
            else {
                gbt->state = STATE_CMD_WM_RECV_SADDR_CS;
            }
            break;
            
            /*
             * STATE_CMD_WM_RECV_NUM_DATA
             */
        case STATE_CMD_WM_RECV_NUM_DATA:
            
            break;
    }
}

static void sendACK(gbt_t *gbt) {
    uint8_t ack = GBT_ACK;
    gbt->outFunc(&ack, 1);
}

static void sendNACK(gbt_t *gbt) {
    uint8_t nack = GBT_NACK;
    gbt->outFunc(&nack, 1);
}

/* Отослать длинну пакета */
static void sendLength(gbt_t *gbt, uint8_t len) {
    gbt->outFunc(&len, 1);
}

static void sendVersion(gbt_t *gbt) {
    uint8_t ver = GBT_VERSION;
    gbt->outFunc(&ver, 1);
}

static void sendCommandsList(gbt_t *gbt) {
    uint8_t pack[GBT_NUM_CMDS] = {GBT_CMD_GET, GBT_CMD_GET_ID, GBT_CMD_READ_MEM, GBT_CMD_WRITE_MEM, GBT_CMD_GO, GBT_CMD_ERASE};
    gbt->outFunc(pack, GBT_NUM_CMDS);
}

static void dummyOut(uint8_t *buf, int32_t len) {
}

static uint8_t isRdpInactive(gbt_t *gbt){
    
}

static void setBuffNum(gbt_t *gbt, uint32_t num){
    gbt->recvLen = num;
    gbt->recvIndex = 0;
}

static uint8_t putBuff(gbt_t *gbt, uint8_t data){
    gbt->recvBuf[gbt->recvIndex] = data;
    gbt->recvIndex++;
    return gbt->recvIndex >= gbt->recvLen;
}

static uint8_t xorVerify(uint8_t *buff, uint32_t num, uint8_t checksum){
    uint8_t xorSum =0;
    while(num--){
        xorSum ^= buff[num];
    }
    if (xorSum ^ checksum == 0xff) return 1;
    return 0;
}
 