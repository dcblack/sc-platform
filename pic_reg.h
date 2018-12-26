#ifndef PIC_REG_H
#define PIC_REG_H

#include "macros.h"
#include <stdint.h>

// Bit masks
enum PicField : uint32_t
{ PIC_PRI_LSB         =  0
  , PIC_PRI_BITS      =  8
  , PIC_PRI_MASK      = MASK(PIC_PRI)
, PIC_ENABLED_LSB     =  8
  , PIC_ENABLED_MASK  = BIT(PIC_ENABLED_LSB)
, PIC_ACTIVE_LSB      =  9
  , PIC_ACTIVE_MASK   = BIT(PIC_ACTIVE_LSB)
, PIC_PENDING_LSB     =  10
  , PIC_PENDING_MASK  = BIT(PIC_PENDING_LSB)
, PIC_CLEARALL_LSB    =  9
  , PIC_CLEARALL_MASK = BIT(PIC_CLEARALL_LSB)
, PIC_ENABLEIF_LSB    =  8
  , PIC_ENABLEIF_MASK = BIT(PIC_ENABLEIF_LSB)
, PIC_SOURCES_LSB     =  0
  , PIC_SOURCES_BITS  =  16
  , PIC_SOURCES_MASK  = MASK(PIC_SOURCES)
, PIC_TARGETS_LSB     =  16
  , PIC_TARGETS_BITS  =  16
  , PIC_TARGETS_MASK  = MASK(PIC_TARGETS)
};

static constexpr uint32_t PIC_INVALID_IRQ = uint32_t(~0ul - 1);

// Equivalent structures
typedef struct
{
  uint32_t reserved : 21 ;
  uint32_t pending  : 1  ;
  uint32_t active   : 1  ; // read-only
  uint32_t enabled  : 1  ;
  uint32_t priority : 8  ; // higher is bigger
} Pic_Source_t;

typedef struct
{
  uint32_t targetid : 16 ; //< Self-reference -- read-only
  uint32_t reserved :  6 ;
  uint32_t clearall :  1 ; //< Clear all sources
  uint32_t enabled  :  1 ; //< Interface is active
  uint32_t mask     :  8 ; //< Prevent lower-priorities
} Pic_Target_t;

typedef struct
{
  uint32_t target_count : 16 ;
  uint32_t source_count : 16 ;
} Pic_Config_t;

typedef struct 
{
  uint32_t         next; //< Read for next interrupt
  uint32_t         done; //< Write when done servicing
  uint32_t         target; //< aka Pic_target_t
  // 8 reserved1;
  // 6 reserved2;
  // 1 if_enabled; //< Interface is active
  // 1 clearall;   //< Clear this interface
  // 8 mask;       //< Prevent lower-priorities
  uint32_t         ident;  //< Read-only
  uint32_t         config; //< Read-only
  uint32_t         select; //< Choose source to interrogate or setup
  uint32_t         source; //< aka Pic_Source_t
  // 21 reserved;
  // 1  pending;
  // 1  active; //< Read-only
  // 1  enabled;
  // 8  priority;
  uint32_t         targets;
} Pic_regs_t;

// Address offsets
enum PicAddr : uint64_t
{ PIC_NEXT_REG    =  0
, PIC_DONE_REG    =  4
, PIC_TARGET_REG  =  8
, PIC_IDENT_REG   = 12 //< Read-only
, PIC_CONFIG_REG  = 16 //< Read-only
, PIC_SELECT_REG  = 20
, PIC_SOURCE_REG  = 24
, PIC_TARGETS_REG = 28
, PIC_REGS_SIZE   = sizeof(Pic_regs_t)
};

#endif /*PIC_REG_H*/
