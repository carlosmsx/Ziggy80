/*********************************************
 * Proyecto: ZiggyBox                        *
 * Autor: Carlos Escobar                     *
 * Jul-2024                                  *
 *********************************************/
#ifndef _DEFS_H
#define _DEFS_H
//#include <Arduino.h>

// #define DEBUG_ON

// Algunas definiciones para comunicar con el driver en MSX
#define CMD_DEBUG     0xD0
#define CMD_SENDSTR   0xE0
#define CMD_FSAVE     0xE1
#define CMD_WRITE     0xF0
#define CMD_READ      0xF1
#define CMD_INIHRD    0xF2
#define CMD_INIENV    0xF3
#define CMD_DRIVES    0xF4
#define CMD_DSKCHG    0xF5
#define CMD_CHOICE    0xF6
#define CMD_DSKFMT    0xF7
#define CMD_OEMSTAT   0xF8
#define CMD_MTOFF     0xF9
#define CMD_GETDPB    0xFA

//definiciones de estados especiales
#define CMD_PARAM__DRIVE_NUMBER   0
#define CMD_PARAM__N_SECTORS      1
#define CMD_PARAM__MEDIA          2
#define CMD_PARAM__SECTOR_H       3
#define CMD_PARAM__SECTOR_L       4
#define CMD_PARAM__ADDR_H         5
#define CMD_PARAM__ADDR_L         6
#define CMD_ST__READING_SEC       10
//#define CMD_ST__READ_CRC          11
#define CMD_ST__WRITING_SEC       20

#define CMD_DSKCHG__DRIVE_NUMBER  30
#define CMD_DSKCHG__B             31
#define CMD_DSKCHG__MEDIA_DESC    32
#define CMD_DSKCHG__ADDR_H        33
#define CMD_DSKCHG__ADDR_L        34
#define CMD_DSKCHG__CHANGE_STATUS 35
#define CMD_DSKCHG__ERROR_CODE    36

void processCommand(register uint8_t command);
uint8_t dataToSend();
void processData(register uint8_t data);

#endif