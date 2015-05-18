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
int main(int argc, char** argv) {
    
    gbt_init(&gbt);
    
    
    printf("GBT! Привет!");
    
    return (EXIT_SUCCESS);
}

