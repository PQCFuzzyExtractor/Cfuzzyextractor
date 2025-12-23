#ifndef FE_CORE_H
#define FE_CORE_H

#include <stdint.h>
#include "bch_wrapper.h"

#define FE_KEY_LEN 32 

typedef struct {
    uint8_t key[FE_KEY_LEN];
} FE_Key;

int FE_Init(void);
void FE_Free(void);
int FE_Gen(const uint8_t *input_data, uint8_t *helper_out, FE_Key *key_out);
int FE_Rep(uint8_t *noisy_input, const uint8_t *helper_in, FE_Key *key_out);

#endif // FE_CORE_H