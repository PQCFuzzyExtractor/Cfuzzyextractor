#include "bch_wrapper.h"
#include "../lib/bch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static struct bch_control *bch = NULL;

int fe_bch_init(void) {
    // 정의된 상수를 사용하여 초기화
    bch = init_bch(GFBITS, SYS_T, 0);
    if (!bch) return -1;
    return 0;
}

void fe_bch_free(void) {
    if (bch) {
        free_bch(bch);
        bch = NULL;
    }
}

void fe_encode(const uint8_t *input, uint8_t *ecc) {
    if (!bch) return;
    
    // 라이브러리에 순수 데이터(FE_DATA_BYTES)만 넘기면 
    // 내부적으로 Shortening(Zero-Padding)을 처리하여 ECC 생성
    memset(ecc, 0, bch->ecc_bytes);
    encode_bch(bch, input, FE_DATA_BYTES, ecc);
}

int fe_decode(uint8_t *noisy_input, const uint8_t *ecc) {
    if (!bch) return -1;

    unsigned int errloc[SYS_T]; 
    // 디코딩 수행
    int count = decode_bch(bch, noisy_input, FE_DATA_BYTES, ecc, NULL, NULL, errloc);

    if (count >= 0) {
        // [비트 플리핑] 에러 위치 정정 수행
        for (int i = 0; i < count; i++) {
            unsigned int idx = errloc[i];
            if (idx < FE_DATA_BYTES * 8) {
                noisy_input[idx / 8] ^= (1 << (idx % 8));
            }
        }
    }
    return count;
}