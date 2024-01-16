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
#include "marat/Z80.h"
#include "pico/cyw43_arch.h"

//Memoria ram
uint8_t *RAM;

//Selector de slot
uint8_t PPI_A8 = 0;

uint8_t InZ80(register uint16_t port)
{
    return PIO_InZ80(port);
}

void OutZ80(register uint16_t port, register uint8_t data)
{
    if (port==0)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, data);
    }
    else
    {
        if ((port & 0xff) == 0xa8) 
            PPI_A8 = data;
        PIO_OutZ80(port, data);
    }
}

// Memory read -- read the value at memory location 'address'
uint8_t RdZ80(register uint16_t address)
{
    uint8_t data = 0xff;
    uint8_t page = (address >> 14) & 0x03;
    uint8_t slot;
    switch (page)
    {
        case 0: //0000~3FFF
            slot = (PPI_A8 & 0x03);
            break;
        case 1: //4000~7FFF
            slot = (PPI_A8 & 0x0c)>>2; 
            break;
        case 2: //8000~BFFF
            slot = (PPI_A8 & 0x30)>>4;
            break;
        case 3: //C000~FFFF
            slot = (PPI_A8 & 0xc0)>>6;
            break;
    }
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
    uint8_t page = (address >> 14);
    uint8_t slot;
    switch (page)
    {
        case 0: //0000~3FFF
            slot = (PPI_A8 & 0x03);
            break;
        case 1: //4000~7FFF
            slot = (PPI_A8 & 0x0c)>>2; 
            break;
        case 2: //8000~BFFF
            slot = (PPI_A8 & 0x30)>>4;
            break;
        case 3: //C000~FFFF
            slot = (PPI_A8 & 0xc0)>>6;
            break;
    }

    if (slot == 1) 
        RAM[address] = data;
    else
        PIO_WrZ80(address, data);
}

// Advanced -- called when an emulator-specific opcode of
// ED FE is encountered. Generally can be left empty.
void PatchZ80(register Z80 *R) {}

void InitRAM()
{
    RAM = malloc(65536);
}

// General

static uint8_t _vdp_reg[8] = {0,0,0,0,0,0,0,0};

void VDP_set(uint8_t reg, uint8_t value)
{
    reg = reg & 0x7; //solo registros de 0 a 7, TMS9918A
    OutZ80(0x99, value); // paso el valor 
    OutZ80(0x99, 0x80 | reg); // a continuacion el registro destino
    _vdp_reg[reg] = value; //guardo el valor para el get
}

uint8_t VDP_get(uint8_t reg)
{
    reg = reg & 0x7;
    return _vdp_reg[reg];
}

void VRAM_write(uint16_t addr, uint8_t value)
{
    // static uint16_t nextAddr = 0xffff;
    addr = addr & 0x3fff; //enmascaro para solo 16K de vram

    // if (addr != nextAddr) // si no es la sig posicion de memoria la seteo
    // {
    // InZ80(0x99);
    OutZ80(0x99, addr & 0xff); //parte baja
    OutZ80(0x99, 0x40 | (addr>>8) ); //parte alta
    // }
    OutZ80(0x98, value);
    sleep_us(5);

    // nextAddr = addr + 1;
}

uint8_t VRAM_read(uint16_t addr)
{
    addr = addr & 0x3fff; //enmascaro para solo 16K de vram

    //InZ80(0x99);
    OutZ80(0x99, addr & 0xff); //parte baja
    OutZ80(0x99, addr>>8 ); //parte alta
    uint8_t z = InZ80(0x98);
    sleep_us(10);
    return z;
}

void InitPPI()
{
  OutZ80(0xab, 0x82);
  OutZ80(0xaa, 0x50);
  OutZ80(0xa8, 0xa0); //selecciono ROM por las dudas
}

uint8_t ch[]={
    0b00111100,
    0b01000010,
    0b10000001,
    0b10100101,
    0b10000001,
    0b10111101,
    0b01000010,
    0b00111100,
};

void CharSet1(uint16_t base, uint8_t c)
{
    for (uint16_t i=0; i<8; i++)
    {
        VRAM_write(base + i + c*8, ch[i]);
    }
}

void CharSet(uint16_t base)
{
    for (uint16_t i=0; i<2048; i++)
    {
        uint8_t c = i<1024? RdZ80(CHARSET_ROM+i) : 0;
        VRAM_write(base + i, c);
    }
}

void screen0() //SCREEN 0
{
    // InZ80(0x99);
    VDP_set(0, 0); //
    VDP_set(1, 0xf0); // text 1 mode
    VDP_set(2, 0);
    VDP_set(3, 0);
    VDP_set(4, 0x01); //tabla de patrones en &H800
    VDP_set(5, 0);
    VDP_set(6, 0);
    VDP_set(7, 0xf4); //COLOR 15,4
    //VDP_set(8, 128); 

    CharSet(0x800);
    for (uint16_t i=0; i<40*24; i++)
    {
        VRAM_write(i, i);
    }
}

void screen1() //SCREEN 1
{
    // InZ80(0x99);
    VDP_set(0, 0x00); //
    VDP_set(1, 0xe0);
    VDP_set(2, 0x06);
    VDP_set(3, 0x80);
    VDP_set(4, 0x00);
    VDP_set(5, 0x36);
    VDP_set(6, 0x07);
    VDP_set(7, 0x17); 

    CharSet(0);
}

void Ti99Splash()
{
    screen1();
    const uint8_t color[] = {6, 3, 1, 11, 12, 13, 15, 4, 2, 13, 8, 14, 5, 9, 10, 6};
    for (uint16_t i=0; i<(32*24); i++)
        VRAM_write(6144+i, 32);
    for (uint16_t i=0; i<16; i++)
        VRAM_write(8192+i, 0x17); //color 1,7
    for (uint16_t i=0; i<16; i++)
        // VRAM_write(8192+16+i, (uint8_t)(color[i]+color[i]*16));
        VRAM_write(8192+16+i, (uint8_t)(color[i]+16));
    for (uint16_t j=0; j<=2; j++)
        for (uint16_t i=0; i<16; i++)
        {
            uint16_t a = 6144 + i*2 + j*32;
            VRAM_write(a, 128 + i*8);
            VRAM_write(a+1, 128 + i*8);
            a = 6144 + i*2 + j*32 + 21*32;
            VRAM_write(a, 128 + i*8);
            VRAM_write(a+1, 128 + i*8);
        }    
}

void Test_PSG_1()
{
  OutZ80(0xa0, 7);
  OutZ80(0xa1, 0xB8); //b1011 1000 ; apaga ruido y deja bit 7 en 1 que es el default para MSX
  for (uint16_t xx=0; xx<0x480; xx++)
  {
    OutZ80(0xa0, 0);
    OutZ80(0xa1, xx & 0xff);
    OutZ80(0xa0, 1);
    OutZ80(0xa1, xx >> 8);
    OutZ80(0xa0, 8);
    OutZ80(0xa1, 8);
    sleep_ms(1);
  }
  OutZ80(0xa0, 8);
  OutZ80(0xa1, 0);
  
}
