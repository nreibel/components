#include "hc595.h"
#include "hc595_prv.h"
#include "hc595_cfg.h"
#include "port.h"
#include "os_cfg.h"

#include <util/delay.h>

void HC595_Init()
{
    Port_SetPinDataDirection(HC595_Pin_Serial, Output);
    Port_SetPinDataDirection(HC595_Pin_Clock, Output);
    Port_SetPinDataDirection(HC595_Pin_Latch, Output);

    Port_SetPinState(HC595_Pin_Serial, Low);
    Port_SetPinState(HC595_Pin_Clock, Low);
    Port_SetPinState(HC595_Pin_Latch, Low);

#if HC595_PIN_CLEAR == ON
    Port_SetPinDataDirection(HC595_Pin_Clear, Output);
    Port_SetPinState(HC595_Pin_Clear, High);
#endif
}

void HC595_Delay()
{
    _delay_us(1);
}

void HC595_RisingEdge(GPIO pin)
{
    Port_SetPinState(pin, High);
    HC595_Delay();
    Port_SetPinState(pin, Low);
}

void HC595_FallingEdge(GPIO pin)
{
    Port_SetPinState(pin, Low);
    HC595_Delay();
    Port_SetPinState(pin, High);
}

void HC595_ShiftBit(uint8_t bit)
{
    Port_SetPinState(HC595_Pin_Serial, bit ? High : Low);
    HC595_RisingEdge(HC595_Pin_Clock);
}

void HC595_Latch()
{
    HC595_RisingEdge(HC595_Pin_Latch);
}

void HC595_ShiftByte(uint8_t val)
{
    for (int i = 0 ; i < 8 ; i++)
    {
        // TODO : use macros
        HC595_ShiftBit(val >> 7);
        val = val << 1;
    }
}

#if HC595_DAISY_CHAIN_COUNT > 1
void HC595_ShiftWord(uint16_t val)
{
    for (int i = 0 ; i < 16 ; i++)
    {
        // TODO : use macros
        HC595_ShiftBit(val >> 15);
        val = val << 1;
    }
}
#endif

#if HC595_DAISY_CHAIN_COUNT > 3
void HC595_ShiftDWord(uint32_t val)
{
    for (int i = 0 ; i < 32 ; i++)
    {
        // TODO : use macros
        HC595_ShiftBit(val >> 31);
        val = val << 1;
    }
}
#endif

#if HC595_PIN_CLEAR == ON
void HC595_Clear()
{
    HC595_FallingEdge(HC595_Pin_Clear);
}
#endif
