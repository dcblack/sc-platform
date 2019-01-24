#ifndef GPIO_REG_H
#define GPIO_REG_H

// Restricted to C 2011 syntax to allow use by embedded C developers

#include <stdint.h>

// Bit masks
// - NONE

// Convenience
typedef struct {    // Bits Description
  bool pindirn; //   1  0=out, 1=in (tri)
  bool pinintr; //   1  1=enabled to interrupt
  bool pinpull; //   1  1=pull
  bool pulldir; //   1  0=down/1=up
  bool datainp; //   1  =
  bool dataout; //   1  =
  bool dataset; //   1  |=
  bool dataclr; //   1  &= ~
  bool datainv; //   1  ^=
  bool datachg; //   1  input change detected / write 1 to clear
  bool dataena; //   1  write 1's to clear changed bits
  bool pinrise; //   1  1's indicate rise detection
  bool pinfall; //   1  1's indicate fall detection
  bool reservD; //   1 
  bool reservE; //   1 
  bool reservF; //   1 
} Gpio_regs_t;

// Address offsets
enum GpioAddr : uint64_t
{ GPIO_PINDIRN_REG    =  ( 0x0 << 3 )
, GPIO_PININTR_REG    =  ( 0x1 << 3 )
, GPIO_PINPULL_REG    =  ( 0x2 << 3 )
, GPIO_PULLDIR_REG    =  ( 0x3 << 3 )
, GPIO_DATAINP_REG    =  ( 0x4 << 3 )
, GPIO_DATAOUT_REG    =  ( 0x5 << 3 )
, GPIO_DATASET_REG    =  ( 0x6 << 3 )
, GPIO_DATACLR_REG    =  ( 0x7 << 3 )
, GPIO_DATAINV_REG    =  ( 0x8 << 3 )
, GPIO_DATACHG_REG    =  ( 0x9 << 3 )
, GPIO_DATAENA_REG    =  ( 0xA << 3 )
, GPIO_PINRISE_REG    =  ( 0xB << 3 )
, GPIO_PINFALL_REG    =  ( 0xC << 3 )
, GPIO_RESERVD_REG    =  ( 0xD << 3 )
, GPIO_RESERVE_REG    =  ( 0xE << 3 )
, GPIO_RESERVF_REG    =  ( 0xF << 3 )
, GPIO_REGS_SIZE   = sizeof(Gpio_regs_t)
};

#endif /*GPIO_REG_H*/
