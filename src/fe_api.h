#ifndef FE_API_H
#define FE_API_H

#include <stdint.h>
#include <stddef.h>

/* =================================================================
 * [상태 코드 정의]
 * ================================================================= */
#define FE_SUCCESS       0
#define FE_FAIL_DECODE  -1  // 복구 실패 (에러 과다)
#define FE_FAIL_PARAM   -2  // 입력 파라미터 오류 (길이 불일치 등)

/* =================================================================
 * [API 함수 선언]
 * ================================================================= */

/**
 * @brief (1) Enrollment API
 * 입력 생체 데이터로부터 Helper Data와 Secret Key를 생성합니다.
 */
int fe_enroll(
    const uint8_t *input,
    size_t input_len,
    uint8_t *helper_data,
    size_t *helper_len,
    uint8_t *secret_key,
    size_t *key_len
);

/**
 * @brief (2) Reproduction API
 * 노이즈가 섞인 입력과 Helper Data를 이용해 Secret Key를 복원합니다.
 * SCA 분석 시, 이 함수의 실행 시간과 전력 소모를 측정합니다.
 */
int fe_reproduce(
    const uint8_t *input,
    size_t input_len,
    const uint8_t *helper_data,
    size_t helper_len,
    uint8_t *recovered_key,
    size_t *key_len
);

#endif // FE_API_H