#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "fe_core.h"

// 터미널 색상 출력
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED   "\033[0;31m"
#define COLOR_BLUE  "\033[0;34m"
#define COLOR_RESET "\033[0m"

// 랜덤 데이터 생성
void generate_random_data(uint8_t *data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] = rand() & 0xFF;
    }
}

// 노이즈 주입 함수 (정확히 error_bits 개수만큼 비트 반전)
void inject_noise(uint8_t *data, int total_bytes, int error_bits) {
    int max_bits = total_bytes * 8;
    int *indices = (int *)malloc(sizeof(int) * max_bits);
    
    // 인덱스 배열 초기화
    for(int i=0; i<max_bits; i++) indices[i] = i;

    // Fisher-Yates Shuffle (비트 위치 섞기)
    for (int i = max_bits - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // 앞쪽 error_bits 개수만큼 선택해서 비트 반전 (XOR)
    for (int i = 0; i < error_bits; i++) {
        int bit_idx = indices[i];
        int byte_idx = bit_idx / 8;
        int bit_pos = bit_idx % 8;
        data[byte_idx] ^= (1 << bit_pos);
    }

    free(indices);
}

void run_test_case(int test_id, int error_count) {
    // 크기는 bch_wrapper.h의 define에 따라 자동 결정됨 (3488 bits / 436 bytes)
    uint8_t bio_original[FE_DATA_BYTES];
    uint8_t bio_noisy[FE_DATA_BYTES];
    uint8_t helper[FE_ECC_BYTES];
    FE_Key key_gen, key_rep;

    // 1. 랜덤 생체 데이터 생성
    generate_random_data(bio_original, FE_DATA_BYTES);

    // 2. 등록 (Gen)
    FE_Gen(bio_original, helper, &key_gen);

    // 3. 노이즈 주입
    memcpy(bio_noisy, bio_original, FE_DATA_BYTES);
    inject_noise(bio_noisy, FE_DATA_BYTES, error_count);

    // 4. 복구 시도 (Rep)
    int result = FE_Rep(bio_noisy, helper, &key_rep);

    // 5. 검증
    int is_key_match = (memcmp(key_gen.key, key_rep.key, FE_KEY_LEN) == 0);
    // SYS_T(64)개 이하면 성공해야 정상
    int expected_success = (error_count <= SYS_T); 

    printf("Test #%02d | Errors: %3d | ", test_id, error_count);

    if (expected_success) {
        // [성공해야 하는 구간] (0 ~ 64 에러)
        if (result >= 0 && is_key_match) {
            printf(COLOR_GREEN "[PASS]" COLOR_RESET " Corrected %d bits (Keys Match)\n", result);
        } else {
            printf(COLOR_RED   "[FAIL]" COLOR_RESET " Expected Success but Failed (Ret: %d)\n", result);
        }
    } else {
        // [실패해야 하는 구간] (65개 이상 에러)
        if (result < 0 || !is_key_match) {
            printf(COLOR_BLUE  "[PASS]" COLOR_RESET " Expected Failure and it Failed safely.\n");
        } else {
            // 실패해야 하는데 성공했다면(키가 다르거나 복구됨) 위험!
            printf(COLOR_RED   "[FAIL]" COLOR_RESET " Expected Failure but SUCCEEDED? (Dangerous!)\n");
        }
    }
}

int main() {
    srand(time(NULL)); // 매번 다른 랜덤값 사용

    if (FE_Init() < 0) {
        printf("Init Failed\n");
        return -1;
    }

    printf("============================================================\n");
    // PK_NCOLS = 3488, SYS_T = 64 가 출력되어야 함
    printf("   Fuzzy Extractor Robustness Test\n");
    printf("   - Input Data: %d bits (%d Bytes)\n", PK_NCOLS, FE_DATA_BYTES);
    printf("   - Capability: Max %d bit errors\n", SYS_T);
    printf("============================================================\n");

    // [그룹 1] 안전 구간 (Safe Zone) - 100% 성공해야 함
    printf("\n--- Group 1: Safe Zone (0 ~ 50 errors) ---\n");
    run_test_case(1, 0);   // 에러 없음
    run_test_case(2, 1);   // 1비트 에러
    run_test_case(3, 5);
    run_test_case(4, 10);
    run_test_case(5, 20);
    run_test_case(6, 30);
    run_test_case(7, 40);
    run_test_case(8, 45);
    run_test_case(9, 50);

    // [그룹 2] 경계 구간 (Boundary Zone) - 64개까지 성공해야 함
    printf("\n--- Group 2: Boundary (60 ~ 64 errors) ---\n");
    run_test_case(10, 55);
    run_test_case(11, 60);
    run_test_case(12, 61);
    run_test_case(13, 62);
    run_test_case(14, 63);
    run_test_case(15, 64); // ★ 여기가 한계점 (PASS 필수)

    // [그룹 3] 실패 구간 (Fail Zone) - 65개부터는 실패해야 함
    printf("\n--- Group 3: Over Limit (65+ errors) ---\n");
    run_test_case(16, 65); // ★ 여기서부터 파란색 PASS(실패 확인) 떠야 함
    run_test_case(17, 66);
    run_test_case(18, 70);
    run_test_case(19, 100);
    run_test_case(20, 200);

    printf("\n============================================================\n");
    printf("Test Complete.\n");

    FE_Free();
    return 0;
}