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

gbt_handlers_t __handlers = {
    .outFunc = testCallbackOut,
    .memRead = callbackMemWrite,
    .memWrite = callbackMemWrite,
    .memClear = callbackMemWrite
};

int main(int argc, char** argv) {
    
    uint8_t RXbuf[255];
    
    u_int8_t testArr_GET[] = {
        0x00,0xFF        
    };
    
    u_int8_t testArr_WRITE_MEM[] = {
        0x31,0xCE,
        0x00,0x00,0x00,0x01,0x01,
        0x04,0x00,0x00,0x02,0x01,0x07
        
    };
    
    gbt_init(&gbt, RXbuf, sizeof(RXbuf), &__handlers);
    gbt_addCallbackOut(&gbt,testCallbackOut);    
    
    gbt_in(&gbt, testArr_WRITE_MEM, sizeof(testArr_WRITE_MEM));
    
    printf("\nGBT! Привет!");
    
    return (EXIT_SUCCESS);
}

