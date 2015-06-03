#include "gbt.h"

/* Handler API */
void gbt_init(gbt_t *gbt, uint8_t *rxbuf, uint32_t rxBufLen, gbt_handlers_t *handlers) {
    gbt->state = STATE_WAIT_CMD;
    gbt->command = GBT_CMD_GET;
    gbt->recvBuf = rxbuf;
    gbt->recvBufLength = rxBufLen;
    gbt->recvLen = gbt->recvIndex = 0;
    gbt->handlers = handlers;
    gbt->pidBuf = 0;
    gbt->pidLen = 0;
}

void gbt_in(gbt_t *gbt, uint8_t *buf, uint32_t len) {
    int index = 0;
    while (index < len) {
        parcer(gbt, buf[index]);
        index++;
    }
}

void gbt_setPid(gbt_t *gbt, uint8_t *pidBuf, uint32_t len){
    gbt->pidBuf = pidBuf;
    gbt->pidLen = len;
}

/****** Private Functions *********/

static void parcer(gbt_t *gbt, uint8_t data) {

    uint32_t startAddress = 0;
    uint32_t lenData = 0;
    uint8_t *buf; 

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
                case GBT_CMD_GET_ID: 
                    gbt->state = STATE_CHECK_GID;
                    break;
                default:
                    sendNACK(gbt);
            }
            break;

            /*
             * STATE_CHECK_CMD_GET
             */
        case STATE_CHECK_CMD_GET:
            if (data == (uint8_t) ~GBT_CMD_GET) {
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
            if (data == (uint8_t) ~GBT_CMD_WRITE_MEM) {
                if (isRdpInactive(gbt)) {
                    sendACK(gbt);
                    gbt->state = STATE_CMD_WM_RECV_SADDR_CS;
                    setBuffNum(gbt, GBT_NUM_ADDR_CS);
                } else {
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
                if (!xorVerify(gbt)) {
                    gbt->dataStartAddress = gbt->recvBuf[0] << 24 | gbt->recvBuf[1] << 16 | gbt->recvBuf[2] << 8 | gbt->recvBuf[3];                    
                    sendACK(gbt);
                    gbt->state = STATE_CMD_WM_RECV_NUM_DATA;
                } else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }
            } else {
                gbt->state = STATE_CMD_WM_RECV_SADDR_CS;
            }
            break;

            /*
             * STATE_CMD_WM_RECV_NUM_DATA
             */
        case STATE_CMD_WM_RECV_NUM_DATA:
            setBuffNum(gbt, data + 2); /* Added receive num + size + checksum */
            putBuff(gbt, data);
            gbt->state = STATE_CMD_WM_RECV_DATA;
            break;
        case STATE_CMD_WM_RECV_DATA:
            if (putBuff(gbt, data)) {
                /* Данные получены */
                /* ??? */
                if (!xorVerify(gbt)) {
                    /* Запись полученных данных в память */
                    __memWrite(gbt, gbt->dataStartAddress, (gbt->recvBuf) + 1, gbt->recvLen - 2);
                    sendACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                } else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }
            } else {
                gbt->state = STATE_CMD_WM_RECV_DATA;
            }
            break;
            //----------------------------------------------------------------------

            /*
             * STATE_CHECK_CMD_RM
             */
        case STATE_CHECK_CMD_RM:
            if (data == (uint8_t) ~GBT_CMD_READ_MEM) {
                if (isRdpInactive(gbt)) {
                    sendACK(gbt);
                    gbt->state = STATE_CMD_RM_RECV_SADDR_CS;
                    setBuffNum(gbt, GBT_NUM_ADDR_CS);
                } else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }
            }
            break;

            /*
             * STATE_CMD_RM_RECV_SADDR_CS
             */
        case STATE_CMD_RM_RECV_SADDR_CS:
            if (putBuff(gbt, data)) {
                /* Запись стартового адреса TODO!!!*/
                if (!xorVerify(gbt)) {
                    gbt->dataStartAddress = gbt->recvBuf[0] << 24 | gbt->recvBuf[1] << 16 | gbt->recvBuf[2] << 8 | gbt->recvBuf[3];
                    //gbt_setStartAddress(gbt, startAddress);
                    sendACK(gbt);
                    setBuffNum(gbt, 2); /* Added receive num + checksum */
                    gbt->state = STATE_CMD_RM_RECV_NUM_DATA_CS;

                } else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }
            } else {
                gbt->state = STATE_CMD_RM_RECV_SADDR_CS;
            }
            break;

            /*
             * STATE_CMD_RM_RECV_NUM_DATA_CS
             */
        case STATE_CMD_RM_RECV_NUM_DATA_CS:

            if (putBuff(gbt, data)) {
                if (!xorVerify(gbt)) {
                    sendACK(gbt);
                    /****/
                    /*Read*/
                    gbt->dataLen=gbt->recvBuf[0];
                    buf = __memRead(gbt, gbt->dataStartAddress, &(gbt->dataLen));                                        
                    __outFunc(gbt, buf, gbt->dataLen);
                    sendACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                    /****/
                } else {
                    sendNACK(gbt);
                    gbt->state = STATE_WAIT_CMD;
                }
            } else {
                gbt->state = STATE_CMD_RM_RECV_NUM_DATA_CS;
            }
            break;
            
            
        case STATE_CHECK_GID:
            if (data == (uint8_t) ~GBT_CMD_GET_ID) {
               sendACK(gbt);
               sendPid(gbt);
               sendACK(gbt);
            }
            break;

    }
}

static void __outFunc(gbt_t *gbt, uint8_t *buf, int32_t len) {
    if (gbt->handlers->outFunc) {
        gbt->handlers->outFunc(buf, len);
    }
}

static uint8_t* __memRead(gbt_t *gbt, uint32_t startAddress, uint32_t *len) {
    if (gbt->handlers->memRead) {
        return gbt->handlers->memRead(startAddress, len);
    } else return 0;
}

static uint32_t __memWrite(gbt_t *gbt, uint32_t startAddress, uint8_t *buff, uint32_t len) {
    if (gbt->handlers->memWrite) {
        return gbt->handlers->memWrite(startAddress, buff, len);
    } else return 0;
}

static void sendACK(gbt_t *gbt) {
    uint8_t ack = GBT_ACK;
    __outFunc(gbt, &ack, 1);
}

static void sendNACK(gbt_t *gbt) {
    uint8_t nack = GBT_NACK;
    __outFunc(gbt, &nack, 1);
}

/* Отослать длинну пакета */
static void sendLength(gbt_t *gbt, uint8_t len) {
    __outFunc(gbt, &len, 1);
}

static void sendVersion(gbt_t *gbt) {
    uint8_t ver = GBT_VERSION;
    __outFunc(gbt, &ver, 1);
}

static void sendCommandsList(gbt_t *gbt) {
    uint8_t pack[GBT_NUM_CMDS] = {GBT_CMD_GET, GBT_CMD_GET_ID, GBT_CMD_READ_MEM, GBT_CMD_WRITE_MEM, GBT_CMD_GO, GBT_CMD_ERASE};
    __outFunc(gbt, pack, GBT_NUM_CMDS);
}
/*
static void dummyOut(uint8_t *buf, int32_t len) {
}

static uint32_t dummyMemRW(uint32_t startAddress, uint8_t *buff, uint32_t len) {

}
*/
static uint8_t isRdpInactive(gbt_t *gbt) {

}

static uint32_t setBuffNum(gbt_t *gbt, uint32_t num) {
    gbt->recvLen = num > gbt->recvBufLength ? gbt->recvBufLength : num;
    gbt->recvIndex = 0;
    return gbt->recvLen;
}

static uint8_t putBuff(gbt_t *gbt, uint8_t data) {
    gbt->recvBuf[gbt->recvIndex] = data;
    gbt->recvIndex++;
    return gbt->recvIndex >= gbt->recvLen;
}

static uint8_t xorVerify(gbt_t *gbt) {
    uint8_t xorSum = 0;
    uint32_t num = gbt->recvLen;

    while (num--) {
        xorSum ^= gbt->recvBuf[num];
    }
    return xorSum;
}

static void sendPid(gbt_t *gbt){
    __outFunc(gbt, &gbt->pidLen, 1);
    __outFunc(gbt, gbt->pidBuf, gbt->pidLen);
}
