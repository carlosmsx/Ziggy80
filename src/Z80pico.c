#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "z80pico.pio.h"

uint sm_z80pico;
uint offset_z80pico;
uint sm_z80sequencer;
uint offset_z80sequencer;

//                        +------ OE
//                        |+----- RD
//                        ||+---- WR
//                        |||+--- MREQ
//                        ||||+-- IORQ
//                        |||||
const uint32_t outT1_ = 0b11111; // 
const uint32_t outT2  = 0b01010; // IORQ WR OE
const uint32_t outT2_ = 0b01010; // IORQ WR OE
const uint32_t outT3  = 0b01010; // IORQ WR OE
const uint32_t outT3_ = 0b11111; // 
const uint32_t outSeq = outT1_ | (outT2 << 5) | (outT2_ << 10) | (outT3 << 15) | (outT3_ << 20);

//                       +------ OE
//                       |+----- RD
//                       ||+---- WR
//                       |||+--- MREQ
//                       ||||+-- IORQ
//                       |||||
const uint32_t inT1_ = 0b11111; 
const uint32_t inT2  = 0b10110; // IORQ RD
const uint32_t inT2_ = 0b00110; // IORQ RD OE
const uint32_t inT3  = 0b00110; // IORQ RD OE
const uint32_t inT3_ = 0b00110; // IORQ RD OE
const uint32_t inSeq = inT1_ | (inT2 << 5) | (inT2_ << 10) | (inT3 << 15) | (inT3_ << 20);

//                        +------ OE
//                        |+----- RD
//                        ||+---- WR
//                        |||+--- MREQ
//                        ||||+-- IORQ
//                        |||||
const uint32_t wrT1_ = 0b11111; // 
const uint32_t wrT2  = 0b11001; // MREQ WR 
const uint32_t wrT2_ = 0b01001; // MREQ WR OE
const uint32_t wrT3  = 0b01001; // MREQ WR OE
const uint32_t wrT3_ = 0b01001; // MREQ WR OE 
const uint32_t wrSeq = wrT1_ | (wrT2 << 5) | (wrT2_ << 10) | (wrT3 << 15) | (wrT3_ << 20);

//                       +------ OE
//                       |+----- RD
//                       ||+---- WR
//                       |||+--- MREQ
//                       ||||+-- IORQ
//                       |||||
const uint32_t rdT1_ = 0b10111; // RD
const uint32_t rdT2  = 0b10101; // MREQ RD
const uint32_t rdT2_ = 0b00101; // MREQ RD 
const uint32_t rdT3  = 0b00101; // MREQ RD OE
const uint32_t rdT3_ = 0b00101; // MREQ RD OE
const uint32_t rdSeq = rdT1_ | (rdT2 << 5) | (rdT2_ << 10) | (rdT3 << 15) | (rdT3_ << 20);

void PIO_OutZ80(register uint16_t port, register uint8_t data)
{
    uint32_t z = (0xff << 24) | (data << 16) | port;
    pio_sm_put(pio0, sm_z80sequencer, outSeq);
    pio_sm_put(pio0, sm_z80pico, z);
    pio_sm_get_blocking(pio0, sm_z80pico); //descarto resultado
    sleep_us(1);
}

uint8_t PIO_InZ80(register uint16_t port)
{
    pio_sm_put(pio0, sm_z80sequencer, inSeq);
    pio_sm_put(pio0, sm_z80pico, port);
    return pio_sm_get_blocking(pio0, sm_z80pico); 
}

void PIO_WrZ80(register uint16_t port, register uint8_t data)
{
    uint32_t z = (0xff << 24) | (data << 16) | port;
    pio_sm_put(pio0, sm_z80sequencer, wrSeq);
    pio_sm_put(pio0, sm_z80pico, z);
    pio_sm_get_blocking(pio0, sm_z80pico); //descarto resultado
}

uint8_t PIO_RdZ80(register uint16_t address)
{
    pio_sm_put(pio0, sm_z80sequencer, rdSeq);
    pio_sm_put(pio0, sm_z80pico, address);
    uint8_t b = pio_sm_get_blocking(pio0, sm_z80pico); 
    return b;
}

void SetupPIO() {
    sm_z80pico = pio_claim_unused_sm(pio0, true);
    offset_z80pico = pio_add_program(pio0, &z80pico_program);
    z80pico_program_init(pio0, sm_z80pico, offset_z80pico, 2);

    sm_z80sequencer = pio_claim_unused_sm(pio0, true);
    offset_z80sequencer = pio_add_program(pio0, &z80sequencer_program);
    z80sequencer_program_init(pio0, sm_z80sequencer, offset_z80sequencer, 12);
}
