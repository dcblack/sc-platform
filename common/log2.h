#ifndef LOG2_H
#define LOG2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
int log2_32( uint32_t value );
int log2_64( uint64_t value );
#ifdef __cplusplus
}
#endif

#endif /*LOG2_H*/
