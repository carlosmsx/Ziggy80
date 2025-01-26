/*********************************************
 * Autor: Carlos Escobar
 * Jul-2023
 * Ref:
 *********************************************/

#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "z80pico.h"
#include "msx.h"
#include "pico/cyw43_arch.h"
//#define CHIPS_IMPL
//#include "chips/z80.h"
//#include "chips/ay38910.h"
//#define _CPC_FREQUENCY (4000000)

extern const uint8_t pacman[];

//Memoria ram
uint8_t *RAM;

//Selector de slot
uint8_t PPI_A8 = 0;
uint8_t PPI_AA = 0;

uint8_t InZ80(register uint16_t port)
{
    uint8_t data = PIO_InZ80(port);
    if ((port & 0xff) == 0xa9)
    {
        if (((PPI_AA & 0xf) == 2) && !gpio_get(18)) // si el scanline es 2 y se pulsa el boton conectado a gpio18
            data &= 0b10111111; // genero una pulsacion de la tecla A
    }
    return data; 
}

void OutZ80(register uint16_t port, register uint8_t data)
{
    switch(port & 0xff)
    {
        case 0x00:
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, data);
            return; //sale
        case 0xa8:
            PPI_A8 = data; //guardo el registro para saber qué slots/páginas están seleccionadas
            break;
        case 0xaa: 
            PPI_AA = data; //guardo el registro para recuperar el scanline 
            break;
    }
    PIO_OutZ80(port, data);
}

inline uint8_t GetSlot(register uint8_t page)
{
    switch (page)
    {
        case 0: //0000~3FFF
            return (PPI_A8 & 0x03);
        case 1: //4000~7FFF
            return (PPI_A8 & 0x0c)>>2; 
        case 2: //8000~BFFF
            return (PPI_A8 & 0x30)>>4;
        case 3: //C000~FFFF
            return (PPI_A8 & 0xc0)>>6;
    }
    return 0;
}

// Memory read -- read the value at memory location 'address'
uint8_t RdZ80(register uint16_t address)
{
    uint8_t data = 0xff;
    uint8_t page = (address >> 14) & 0x03;
    uint8_t slot = GetSlot(page);
    switch (slot)
    {
        case 0: //ROM
            // if (page<2) data = ROM[address]; //pages 0 1
            if (page<2) data = PIO_RdZ80(address);
            // data = RdMem(address);
            break; 
        case 1: //RAM
            data = RAM[address]; //all pages
            break;
        case 2: //CARTRIDGE SLOT
            // data = RdMem(address);
            //if (page==1) data = pacman[address & 0x3fff];
            break; 
        case 3: //EXPANSION BUS
            // data = RdMem(address);
            break;
    }
    return data;
}

// Opcode read -- read the opcode at memory location 'address'
uint8_t OpZ80(register uint16_t address)
{
    uint8_t data = 0xff;
    uint8_t page = (address >> 14) & 0x03;
    uint8_t slot = GetSlot(page);
    switch (slot)
    {
        case 0: //ROM
            // if (page<2) data = ROM[address]; //pages 0 1
            data = PIO_RdZ80(address);
            break; 
        case 1: //RAM
            data = RAM[address]; //all pages
            break;
        case 2: //CARTRIDGE SLOT
            // data = RdMem(address);
            //if (page==1) data = pacman[address & 0x3fff];
            break; 
        case 3: //EXPANSION BUS
            // data = RdMem(address);
            break;
    }
    return data;
}

// Memory write -- write the 'data' value to memory location 'address'
void WrZ80(register uint16_t address, register uint8_t data)
{
    uint8_t page = (address >> 14) & 0x03;
    uint8_t slot = GetSlot(page);
    if (slot == 1) 
        RAM[address] = data;
    else
        PIO_WrZ80(address, data);
}

void InitRAM()
{
    RAM = malloc(65536);
}

