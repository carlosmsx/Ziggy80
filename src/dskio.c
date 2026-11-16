/*********************************************
 * Proyecto: ZiggyBox                        *
 * Autor: Carlos Escobar                     *
 * Jul-2024                                  *
 *********************************************/
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "dskio.h"
#include "embedded_dsk.h"

uint16_t _word=0;
uint8_t _stat=0;
uint8_t _cmd;
uint8_t _cmd_st;
uint8_t _checksum=0;
uint32_t _total=0;
uint8_t _drive_number; //, _last_drv=0;
uint8_t _n_sectors;
uint8_t _media;
uint8_t _sec_H;
uint8_t _sec_L;
uint8_t _addr_H;
uint8_t _addr_L;
uint32_t _sector;
uint16_t _address;
uint8_t _cur_drive[2] = {0,1};
uint8_t _dsk_changed = 0;
uint32_t _dsk_idx;

void InitHardware()
{
  _stat=0;
  _cmd=0;
  _cmd_st=0;
  _checksum=0;
  _total=0;
  _drive_number=0;
  _n_sectors=0;
  _media=0;
  _sec_H=0;
  _sec_L=0;
  _addr_H=0;
  _addr_L=0;
  _sector=0;
  _address=0;
}

void processCommand(register uint8_t command)
{
  _cmd = command;

  switch (_cmd)
  {
    case CMD_SENDSTR:
      break;
    case CMD_FSAVE:
      _cur_drive[_drive_number] = (_cur_drive[_drive_number]+1) % 3; //0 a 3
      _dsk_changed = 1;
      break;
    case CMD_WRITE:
      _cmd_st = CMD_PARAM__DRIVE_NUMBER; // proximo sub-estado escritura
      break;
    case CMD_READ:
      _cmd_st = CMD_PARAM__DRIVE_NUMBER; // proximo sub-estado lectura
      break;
    case CMD_INIHRD:
      InitHardware();
      break;
    case CMD_INIENV:
      break;
    case CMD_DRIVES:
      break;
    case CMD_DSKCHG:
      _cmd_st = CMD_DSKCHG__DRIVE_NUMBER;
      break;
    case CMD_CHOICE:
      break;
    case CMD_DSKFMT:
      break;
    case CMD_OEMSTAT:
      break;
    case CMD_MTOFF:
      break;
    case CMD_GETDPB:
      break;
  }
}

void processData(register uint8_t data)
{
  switch (_cmd)
  {
    case CMD_SENDSTR:
      break;
    case CMD_FSAVE:
      break;
    case CMD_DSKCHG:
      switch (_cmd_st)
      {
        case CMD_DSKCHG__DRIVE_NUMBER:
          _drive_number = data; 
          _cmd_st = CMD_DSKCHG__B; 
          break;
        case CMD_DSKCHG__B:
          _cmd_st = CMD_DSKCHG__MEDIA_DESC;
          break;
        case CMD_DSKCHG__MEDIA_DESC:
          _cmd_st = CMD_DSKCHG__ADDR_H;   
          break;
        case CMD_DSKCHG__ADDR_H:
          _cmd_st = CMD_DSKCHG__ADDR_L;
          break;
        case CMD_DSKCHG__ADDR_L:
          _cmd_st = CMD_DSKCHG__CHANGE_STATUS;
          break;
      }
      break;
    case CMD_WRITE:
    case CMD_READ:
      switch (_cmd_st)
      {
        case CMD_PARAM__DRIVE_NUMBER: 
          _drive_number = data; 
          _cmd_st = CMD_PARAM__N_SECTORS; 
          break;
        case CMD_PARAM__N_SECTORS: 
          _n_sectors = data; 
          _cmd_st = CMD_PARAM__MEDIA; 
          break;
        case CMD_PARAM__MEDIA: 
          _media = data; 
          _cmd_st = CMD_PARAM__ADDR_H;
          break;
        case CMD_PARAM__ADDR_H: 
          _addr_H = data;    
          _cmd_st = CMD_PARAM__ADDR_L;
          break;
        case CMD_PARAM__ADDR_L: 
          _addr_L = data;    
          _address = (uint16_t)_addr_H<<8 | _addr_L;
          _cmd_st = CMD_PARAM__SECTOR_H;
          break;
        case CMD_PARAM__SECTOR_H: 
          _sec_H = data;    
          _cmd_st = CMD_PARAM__SECTOR_L;
          break;
        case CMD_PARAM__SECTOR_L: 
          _sec_L = data;
          _sector = (uint16_t)_sec_H<<8 | _sec_L /*particion 0*/;
          _dsk_idx = _sector*512;
          _total = (uint32_t)_n_sectors << 9; //_n_sectors * 512; 

          if ( _cmd == CMD_READ )
          {
            _cmd_st = CMD_ST__READING_SEC;
          }
          else
          {
            _cmd_st = CMD_ST__WRITING_SEC;
          }
          break;
        case CMD_ST__WRITING_SEC:
          //write byte to buffer
///////////          dsk[_dsk_idx++]=data;
          _total--;
          
          if (_total == 0)
          {
            _cmd = 0;
            _cmd_st = 0;
          }
          break;
      }
      break;
  }
}

uint8_t dataToSend()
{
  switch(_cmd)
  {
    case CMD_DSKCHG:
      switch ( _cmd_st )
      {
        case CMD_DSKCHG__CHANGE_STATUS:
          _cmd_st = CMD_DSKCHG__ERROR_CODE;
          if (_dsk_changed) 
          {
            _dsk_changed = 0;
            return -1;
          }
          else 
            return 1;
        case CMD_DSKCHG__ERROR_CODE:
          _cmd = 0;
          _cmd_st = 0;
          return 0;
      }
    case CMD_READ:
      if ( _cmd_st == CMD_ST__READING_SEC )
      {
        //read byte from buffer
        uint8_t b= dsk[_dsk_idx++];
        _checksum = _checksum ^ b;
        _total--;
        
        if (_total == 0)
        {
          _cmd = 0;
          _cmd_st = 0;
        }
        
        return b;
      }
  }
//   DEBUG1("NULL");
  return 0;
}

