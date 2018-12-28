#ifndef GPIO_REG_H
#define GPIO_REG_H

// Restricted to C 2011 syntax to allow use by embedded C developers

#include <stdint.h>

// Bit masks
// - NONE

// Convenience
typedef struct {    // rA FUNC
  uint64_t pindirn; // x0 0=out, 1=in (tri)
  uint64_t pinintr; // x1 1=enabled to interrupt
  uint64_t pinpull; // x2 1=pull
  uint64_t pulldir; // x3 0=down/1=up
  uint64_t datainp; // x4 =
  uint64_t dataout; // x5 =
  uint64_t dataset; // x6 |=
  uint64_t dataclr; // x7 &= ~
  uint64_t datainv; // x8 ^=
  uint64_t datachg; // x9 input change detected / write 1 to clear
  uint64_t dataena; // xA write 1's to clear changed bits
  uint64_t pinrise; // xB 1's indicate rise detection
  uint64_t pinfall; // xC 1's indicate fall detection
  uint64_t reservD; // xD
  uint64_t reservE; // xE
  uint64_t reservF; // xF
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
