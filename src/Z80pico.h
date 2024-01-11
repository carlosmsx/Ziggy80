#pragma once

void PIO_OutZ80(register uint16_t port, register uint8_t data);

uint8_t PIO_InZ80(register uint16_t port);

void PIO_WrZ80(register uint16_t port, register uint8_t data);

uint8_t PIO_RdZ80(register uint16_t address);

void SetupPIO();
