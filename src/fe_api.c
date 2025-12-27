#include "fe_api.h"
#include "fe_core.h"
#include "bch_wrapper.h"
#include <string.h>

/* =================================================================
 * (1) Enrollment 구현
 * ================================================================= */
int fe_enroll(
    const uint8_t *input,
    size_t input_len,
    uint8_t *helper_data,
    size_t *helper_len,
    uint8_t *secret_key,
    size_t *key_len
) {
    // 1. 파라미터 유효성 검사
    if (!input || !helper_data || !helper_len || !secret_key || !key_len) {
        return FE_FAIL_PARAM;
    }

    // 입력 길이가 우리 시스템 규격(FE_DATA_BYTES)과 맞는지 확인
    if (input_len != FE_DATA_BYTES) {
        return FE_FAIL_PARAM;
    }

    // 초기화 (최초 1회 필요, 중복 호출 안전)
    if (FE_Init() < 0) return FE_FAIL_PARAM;

    FE_Key key_struct;

    // 2. Core 엔진 호출 (Gen)
    // 내부적으로 Helper Data와 Key를 생성함
    FE_Gen(input, helper_data, &key_struct);

    // 3. 결과 전달
    // 생성된 키를 사용자가 제공한 버퍼로 복사
    memcpy(secret_key, key_struct.key, FE_KEY_LEN);

    // 실제 출력된 길이 정보 갱신
    *helper_len = FE_ECC_BYTES;
    *key_len = FE_KEY_LEN;

    return FE_SUCCESS;
}

/* =================================================================
 * (2) Reproduction 구현 (SCA 측정 대상)
 * ================================================================= */
int fe_reproduce(
    const uint8_t *input,
    size_t input_len,
    const uint8_t *helper_data,
    size_t helper_len,
    uint8_t *recovered_key,
    size_t *key_len
) {
    // 1. 파라미터 유효성 검사
    if (!input || !helper_data || !recovered_key || !key_len) {
        return FE_FAIL_PARAM;
    }

    // 규격 검사
    if (input_len != FE_DATA_BYTES || helper_len != FE_ECC_BYTES) {
        return FE_FAIL_PARAM;
    }

    // 초기화
    if (FE_Init() < 0) return FE_FAIL_PARAM;

    FE_Key key_struct;

    // 2. Core 엔진 호출 (Rep)
    // 이곳이 실행 시간 측정의 핵심 포인트
    int ret = FE_Rep(input, (uint8_t *)helper_data, &key_struct);

    if (ret < 0) {
        // 복구 실패 (에러가 너무 많음)
        return FE_FAIL_DECODE; 
    }

    // 3. 결과 전달 (성공 시에만)
    memcpy(recovered_key, key_struct.key, FE_KEY_LEN);
    *key_len = FE_KEY_LEN;

    return FE_SUCCESS;
}