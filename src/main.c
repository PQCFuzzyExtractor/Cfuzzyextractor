#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fe_api.h"       // 우리가 만든 API 헤더
#include "fe_core.h"      // ★ 핵심! 여기에 FE_KEY_LEN 정의가 들어있습니다.
#include "bch_wrapper.h"  // 사이즈 상수(FE_DATA_BYTES 등) 사용

// 색상 매크로
#define COL_GREEN   "\033[1;32m"
#define COL_RED     "\033[1;31m"
#define COL_RESET   "\033[0m"

void print_result(const char* title, int ret) {
    printf("[%s] Result: ", title);
    if (ret == FE_SUCCESS) {
        printf(COL_GREEN "SUCCESS (0)" COL_RESET "\n");
    } else {
        printf(COL_RED "FAIL (%d)" COL_RESET "\n", ret);
    }
}

int main() {
    printf("============================================\n");
    printf("      Fuzzy Extractor API Test\n");
    printf("============================================\n");

    // 1. 변수 준비
    uint8_t input[FE_DATA_BYTES] = {0}; // 테스트용 000... 데이터
    uint8_t helper[FE_ECC_BYTES];
    uint8_t key_org[FE_KEY_LEN];
    uint8_t key_rec[FE_KEY_LEN];

    size_t h_len = FE_ECC_BYTES;
    size_t k_len = FE_KEY_LEN;
    int ret;

    // 더미 데이터 채우기 (패턴: 0xAA)
    memset(input, 0xAA, FE_DATA_BYTES);

    // ---------------------------------------------------------
    // TEST 1: 등록 (Enrollment)
    // ---------------------------------------------------------
    printf("\n[1] Testing fe_enroll()...\n");
    ret = fe_enroll(input, FE_DATA_BYTES, helper, &h_len, key_org, &k_len);
    print_result("Enroll", ret);

    if (ret != FE_SUCCESS) return -1; // 등록 실패하면 뒤에는 테스트 의미 없음

    // ---------------------------------------------------------
    // TEST 2: 복구 (Reproduction) - 정상 케이스
    // ---------------------------------------------------------
    printf("\n[2] Testing fe_reproduce() - Clean Data...\n");
    // 입력 데이터가 똑같으니 무조건 성공해야 함
    ret = fe_reproduce(input, FE_DATA_BYTES, helper, h_len, key_rec, &k_len);
    print_result("Repro(Clean)", ret);

    // 키가 진짜 똑같은지 확인
    if (memcmp(key_org, key_rec, FE_KEY_LEN) == 0) {
        printf(" -> Key Check: " COL_GREEN "MATCH" COL_RESET "\n");
    } else {
        printf(" -> Key Check: " COL_RED "MISMATCH" COL_RESET "\n");
    }

    // ---------------------------------------------------------
    // TEST 3: 복구 (Reproduction) - 노이즈 케이스 (에러 주입)
    // ---------------------------------------------------------
    printf("\n[3] Testing fe_reproduce() - Noisy Data (10 bits error)...\n");
    
    // 데이터 복사 후 에러 10개 주입
    uint8_t noisy_input[FE_DATA_BYTES];
    memcpy(noisy_input, input, FE_DATA_BYTES);
    
    // 앞에서부터 10비트 반전 시킴
    for(int i=0; i<10; i++) {
        noisy_input[i/8] ^= (1 << (i%8));
    }

    ret = fe_reproduce(noisy_input, FE_DATA_BYTES, helper, h_len, key_rec, &k_len);
    print_result("Repro(Noisy)", ret);

    if (memcmp(key_org, key_rec, FE_KEY_LEN) == 0) {
        printf(" -> Key Check: " COL_GREEN "MATCH" COL_RESET "\n");
    } else {
        printf(" -> Key Check: " COL_RED "MISMATCH" COL_RESET "\n");
    }

    printf("\n============================================\n");
    return 0;
}