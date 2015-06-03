/* 
 * File:   main.c
 * Author: roman
 *
 * Created on 18 мая 2015 г., 15:25
 */

#include <stdio.h>
#include <stdlib.h>
#include "gbt.h"

gbt_t gbt;
uint8_t __RXbuf[255];
uint8_t __TXbuf[255];
/*
 * 
 */

void testCallbackOut(uint8_t *buf, int32_t len){
    uint32_t cnt = len;
    uint8_t data;
    while (cnt--) {
        data = buf[cnt];
        if (data == GBT_ACK){
            printf("ACK:\n");
        }else if (data == GBT_NACK){
            printf("NACK:\n");
        }
        else{
            printf("%x:\n",buf[cnt]);
        }
    }    
}

uint32_t callbackMemWrite(uint32_t startAddress, uint8_t *buff, uint32_t len){
    uint32_t i;
    printf("\t:Mem WRITE:\n");
    printf("\tStartAddress: %d\n", startAddress);
    printf("\tData: ");
    for(i=0;i<len;i++){
        printf("%x ",buff[i]);
    }
    printf("\n");
    return i;
}

uint8_t* callbackMemRead(uint32_t startAddress, uint32_t *len){
    return __TXbuf;
}

uint8_t callbackMemClear(uint32_t startAddress,  uint8_t filler, uint32_t len){
    return 0;
}

gbt_handlers_t __handlers = {
    .outFunc = testCallbackOut,
    .memRead = callbackMemRead,
    .memWrite = callbackMemWrite,
    .memClear = callbackMemClear
};

int main(int argc, char** argv) {
      
    int cnt;
    cnt=sizeof(__TXbuf);
    
    while(cnt--){
        __TXbuf[cnt] = cnt;
    }
    
    uint8_t testArr_GET[] = {
        0x00,0xFF        
    };
    
    uint8_t testArr_WRITE_MEM[] = {
        0x31,0xCE,
        0x00,0x00,0x00,0x01,0x01,
        0x04,0x00,0x00,0x02,0x01,0x07
        
    };
    
    uint8_t testArr_READ_MEM[] = {
        GBT_CMD_READ_MEM, ~GBT_CMD_READ_MEM,
        0x00,0x00,0x00,0x01,0x01,
        0x10, 0x10
    };
    
    uint8_t testArr_ID[] = {
        0x10, 0x11
    };
    
    uint8_t testArr_GET_ID[] = {
        GBT_CMD_GET_ID, ~GBT_CMD_GET_ID
    };
    
    gbt_init(&gbt, __RXbuf, sizeof(__RXbuf), &__handlers);
    gbt_setPid(&gbt, testArr_ID, sizeof(testArr_ID));
    
    printf("Cmd GET\n");
    gbt_in(&gbt, testArr_GET, sizeof(testArr_GET));
    
    printf("\nCmd WRITE\n");
    gbt_in(&gbt, testArr_WRITE_MEM, sizeof(testArr_WRITE_MEM));
    
    printf("\nCmd READ\n");
    gbt_in(&gbt, testArr_READ_MEM, sizeof(testArr_READ_MEM));
    
    printf("\nCmd READ ID\n");
    gbt_in(&gbt, testArr_GET_ID, sizeof(testArr_GET_ID));
    
    printf("\nGBT! Привет!");
    
    return (EXIT_SUCCESS);
}

