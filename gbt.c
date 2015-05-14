#include "gbt.h"

#define __STATE_RECV_CMD (0)
#define __STATE_RECV_CMD_CMP (1)
#define __STATE_EXEC (2)




/* Handler API */
void gbt_init(gbt_t *gbt){
  gbt->state = __STATE_RECV_CMD;
  gbt->command = GBT_CMD_GET;
  gbt->outFunc = dummyOut;
}

void gbt_in(gbt_t *gbt, unsigned char *buf, int *len){
  int index = 0;
  while (index<*len){
    parcer(gbt, buf[index]);
    index++;
  }
}

void gbt_addCallbackOut(gbt_t *gbt, callbackOut_t *callback){
  gbt->outFunc = callback;
}
/*
extern void gbt_out(unsigned char *buf, int len){
}
*/
/***************/

static void parcer(gbt_t *gbt, unsigned char byte){
  
  switch(gbt->state){
    /* Приём комманды */
  case __STATE_RECV_CMD:
    gbt->command=byte;
    gbt->state =  __STATE_RECV_CMD_CMP;
    break;
    
    /* Приём комплементарного байта комманды для проверки */
  case __STATE_RECV_CMD_CMP:
    /* Проверка пройдена */
    if ((gbt->command ^ byte)==0xFF){
      sendACK(gbt);
      gbt->state = __STATE_EXEC;
    } else {
      sendNACK(gbt);
      gbt->state = __STATE_RECV_CMD;
      break;
    }
  case __STATE_EXEC:
    /* Выполнение комманды */
    switch(gbt->command){
    case GBT_CMD_GET:
      sendLength(gbt, GBT_NUM_CMDS);
      sendVersion(gbt);
      sendCommandsList(gbt);
      sendACK(gbt);
      gbt->state = __STATE_RECV_CMD;
      break;
      /* Нет такой комманды */
    default:
      sendNACK(gbt);
      gbt->state = __STATE_RECV_CMD;
    }
    break;
    
    /* Нет такого состояния */
  default:
    sendNACK(gbt);
    gbt->state = __STATE_RECV_CMD;
  }
}

static void sendACK(gbt_t *gbt){
  unsigned char ack= GBT_ACK;
  gbt->outFunc(&ack, 1);
}

static void sendNACK(gbt_t *gbt){
  unsigned char nack= GBT_NACK;
  gbt->outFunc(&nack, 1);
}

/* Отослать длинну пакета */
static void sendLength(gbt_t *gbt, unsigned char len){
  gbt->outFunc(&len, 1);
}

static void sendVersion(gbt_t *gbt){
  unsigned char ver= GBT_VERSION;
  gbt->outFunc(&ver, 1);
}

static void sendCommandsList(gbt_t *gbt){
  unsigned char pack[GBT_NUM_CMDS] = {GBT_CMD_GET, GBT_CMD_GET_ID, GBT_CMD_READ_MEM, GBT_CMD_WRITE_MEM, GBT_CMD_GO, GBT_CMD_ERASE};
  gbt->outFunc(pack, GBT_NUM_CMDS);
}

static void dummyOut(unsigned char *buf, int len){
}