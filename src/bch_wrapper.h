#ifndef BCH_WRAPPER_H
#define BCH_WRAPPER_H

#include <stdint.h>

/* =================================================================
 * [Configuration] 순수 입력 데이터 3488비트 확보 설정
 * ================================================================= */

// 1. 갈로아 필드 차수 (m)
// 3488비트 데이터를 담기 위해 m=13 (최대 8191비트 용량) 사용
#define GFBITS          13            

// 2. 오류 정정 개수 (t)
#define SYS_T           64            

// 3. 전체 코드 길이 (Total Bits)
// 목표: 순수 데이터(PK_NCOLS)가 3488비트가 되도록 역산
// 계산: ECC 비트 = 64 * 13 = 832 비트
//       전체 비트 = 3488(데이터) + 832(ECC) = 4320 비트
#define SYS_N_BITS      4320          


/* =================================================================
 * [Calculation] 내부 상수 자동 계산 (하드코딩 제거)
 * ================================================================= */

// ECC 비트 수 (t * m) = 832 bits
#define PK_NROWS        (SYS_T * GFBITS)

// 순수 데이터 비트 수 = 전체 - ECC
// 4320 - 832 = 3488 bits (★요구사항 만족★)
#define PK_NCOLS        (SYS_N_BITS - PK_NROWS)

/* Byte 단위 변환 (자동 올림 처리) */
#define FE_ECC_BYTES    ((PK_NROWS + 7) / 8)    // 104 Bytes
#define FE_DATA_BYTES   ((PK_NCOLS + 7) / 8)    // 436 Bytes (3488 bits)
#define FE_TOTAL_BYTES  ((SYS_N_BITS + 7) / 8)  // 540 Bytes

/* 라이브러리 내부 버퍼 크기 (m=13일 때 넉넉하게 잡음) */
#define BCH_TOTAL_BYTES 2048


/* =================================================================
 * [API Declarations]
 * ================================================================= */

int fe_bch_init(void);
void fe_bch_free(void);
void fe_encode(const uint8_t *input, uint8_t *ecc);
int fe_decode(uint8_t *noisy_input, const uint8_t *ecc);

#endif // BCH_WRAPPER_H