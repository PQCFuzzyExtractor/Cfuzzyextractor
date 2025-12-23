#include "fe_core.h"
#include <string.h>
#include <stdio.h>

// 간단한 해시 함수 (테스트용)
static void simple_hash(const uint8_t *input, int len, uint8_t *out_key) {
    memset(out_key, 0, FE_KEY_LEN);
    for (int i = 0; i < len; i++) {
        out_key[i % FE_KEY_LEN] ^= input[i];
        out_key[i % FE_KEY_LEN] = (out_key[i % FE_KEY_LEN] << 1) | (out_key[i % FE_KEY_LEN] >> 7);
    }
}

int FE_Init(void) {
    return fe_bch_init();
}

void FE_Free(void) {
    fe_bch_free();
}

int FE_Gen(const uint8_t *input_data, uint8_t *helper_out, FE_Key *key_out) {
    if (!input_data || !helper_out || !key_out) return -1;
    fe_encode(input_data, helper_out);
    simple_hash(input_data, FE_DATA_BYTES, key_out->key);
    return 0; 
}

int FE_Rep(uint8_t *noisy_input, const uint8_t *helper_in, FE_Key *key_out) {
    if (!noisy_input || !helper_in || !key_out) return -1;
    int err_cnt = fe_decode(noisy_input, helper_in);
    if (err_cnt < 0) return -1;
    simple_hash(noisy_input, FE_DATA_BYTES, key_out->key);
    return err_cnt;
}