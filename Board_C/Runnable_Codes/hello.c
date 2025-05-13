//*****************************************************************************
//
// hello.c - Simple hello world example.
//
// Copyright (c) 2013-2020 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.2.0.295 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "drivers/pinout.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include <keygen.h>
#include <encap.h>
#include <decap.h>
#include <stdio.h>

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Hello World (hello)</h1>
//!
//! A very simple ``hello world'' example.  It simply displays ``Hello World!''
//! on the UART and is a starting point for more complicated applications.
//!
//! Open a terminal with 115,200 8-N-1 to see the output for this demo.
//
//*****************************************************************************

//*****************************************************************************
//
// System clock rate in Hz.
//
//*****************************************************************************
uint32_t g_ui32SysClock;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, g_ui32SysClock);
}

//Variable Definitions
// #define len_tau_2 8192
#define len_tau_2 100
// #define d 594
#define d 100
#define n 1
// #define k 594 // k = d/n.
#define k 100
#define kappa_bytes 16
#define q 8192
#define n_bar 7
#define m_bar 7
#define h 98
#define p_bits 10
#define t_bits 7
#define b_bits 3
#define cipher_bits 5236
#define mu 43
#define sk_bits 16

// #define SHAKE128_RATE 168
// #define SHAKE256_RATE 136

uint8_t sigma[16] = {
    0x7C, 0x99, 0x35, 0xA0,
    0xB0, 0x76, 0x94, 0xAA,
    0x0C, 0x6D, 0x10, 0xE4,
    0xDB, 0x6B, 0x1A, 0xDD
};
uint8_t sk[16] = {
    0x91, 0x28, 0x22, 0x14,
    0x65, 0x4C, 0xB5, 0x5E,
    0x7C, 0x2C, 0xAC, 0xD5,
    0x39, 0x19, 0x60, 0x4D
};
uint8_t pk[5214] = {0};
uint8_t ct[5236] = {0};
uint8_t key[32] = {0};

int
main(void)
{   
    //Setting for UART
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240), 120000000);
    ConfigureUART();

    UARTprintf("Code Running!\n");
    kemKeygen(sigma, sk, pk);
    UARTprintf("KeyGen Done!\n");
    r5_cpa_kem_encap(ct, key, pk, sigma);
    UARTprintf("Enc Done!\n");
    r5_cpa_kem_decap(key, ct, sk);
    UARTprintf("Dec Done!\n");
    return 0;
}
