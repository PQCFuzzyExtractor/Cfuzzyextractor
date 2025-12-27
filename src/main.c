#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "fe_core.h"
#include "bch_wrapper.h" // 파라미터(SYS_T, FE_DATA_BYTES 등) 사용

// 테스트 반복 횟수 (횟수가 클수록 통계가 정확해짐)
#define NUM_TRIALS 1000

// 색상 매크로
#define COL_RED     "\033[1;31m"
#define COL_GREEN   "\033[1;32m"
#define COL_YELLOW  "\033[1;33m"
#define COL_CYAN    "\033[1;36m"
#define COL_RESET   "\033[0m"

/* =================================================================
 * [유틸리티] 랜덤 데이터 생성 및 노이즈 주입
 * ================================================================= */
void generate_random_data(uint8_t *data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] = rand() & 0xFF;
    }
}

// 정확히 error_cnt 개수만큼 비트를 랜덤하게 뒤집음
void inject_exact_noise(uint8_t *data, int total_bytes, int error_cnt) {
    if (error_cnt <= 0) return;

    int max_bits = total_bytes * 8;
    int *indices = (int *)malloc(sizeof(int) * max_bits);
    
    // 0 ~ max_bits-1 인덱스 생성
    for(int i=0; i<max_bits; i++) indices[i] = i;

    // Fisher-Yates Shuffle (인덱스 섞기)
    for (int i = max_bits - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // 앞쪽에서 error_cnt 개수만큼 골라서 비트 반전
    for (int i = 0; i < error_cnt; i++) {
        int bit_idx = indices[i];
        data[bit_idx / 8] ^= (1 << (bit_idx % 8));
    }

    free(indices);
}

/* =================================================================
 * [통계 테스트] 지정된 에러 개수에 대해 N번 반복 수행
 * ================================================================= */
void run_statistical_test(int error_cnt) {
    int success_cnt = 0;      // 복구 성공 & 키 일치
    int fail_cnt = 0;         // 복구 실패 (Rep가 -1 반환)
    int false_accept_cnt = 0; // 위험! 복구 성공했으나 키가 다름 (오인식)

    uint8_t w_org[FE_DATA_BYTES];   // 원본 생체정보
    uint8_t w_noisy[FE_DATA_BYTES]; // 노이즈 낀 생체정보
    uint8_t helper[FE_ECC_BYTES];   // 헬퍼 데이터
    FE_Key key_org, key_rec;        // 원본 키, 복구된 키

    for (int i = 0; i < NUM_TRIALS; i++) {
        // 1. 랜덤 생체정보 생성 및 등록(Gen)
        generate_random_data(w_org, FE_DATA_BYTES);
        FE_Gen(w_org, helper, &key_org);

        // 2. 노이즈 주입 (복사본에 수행)
        memcpy(w_noisy, w_org, FE_DATA_BYTES);
        inject_exact_noise(w_noisy, FE_DATA_BYTES, error_cnt);

        // 3. 복구 시도(Rep)
        int res = FE_Rep(w_noisy, helper, &key_rec);

        // 4. 결과 판별
        if (res < 0) {
            // BCH가 "이건 못 고칩니다" 하고 포기함 (안전한 실패)
            fail_cnt++;
        } else {
            // BCH가 고쳤다고 함. 진짜 맞게 고쳤는지 키 비교
            if (memcmp(key_org.key, key_rec.key, FE_KEY_LEN) == 0) {
                success_cnt++;
            } else {
                // BCH는 고쳤다고 했지만, 엉뚱한 값으로 고침 (위험한 상황)
                false_accept_cnt++;
            }
        }
    }

    // 결과 출력 (표 형식)
    // 안전 구간(<=64)에서는 Success가 100%여야 함
    // 위험 구간(>=65)에서는 Fail이 100%여야 함
    printf("| %3d bits |  %4d  | " COL_GREEN "%5.1f%%" COL_RESET " | " COL_RED "%5.1f%%" COL_RESET " | ", 
            error_cnt, NUM_TRIALS, 
            (double)success_cnt / NUM_TRIALS * 100.0, 
            (double)false_accept_cnt / NUM_TRIALS * 100.0);

    // 상태 메시지 출력
    if (error_cnt <= SYS_T) {
        if (success_cnt == NUM_TRIALS) printf(COL_CYAN "[PERFECT]" COL_RESET "\n");
        else printf(COL_RED "[WARNING]" COL_RESET "\n");
    } else {
        if (false_accept_cnt > 0) printf(COL_RED "[DANGER: FAR]" COL_RESET "\n");
        else printf(COL_YELLOW "[SAFE REJECT]" COL_RESET "\n");
    }
}

int main() {
    srand((unsigned int)time(NULL));

    if (FE_Init() < 0) {
        printf("FE_Init Error!\n");
        return -1;
    }

    printf("\n========================================================================\n");
    printf("   Fuzzy Extractor Statistical Analysis (N=%d Trials)\n", NUM_TRIALS);
    printf("   - Input Size : %d bits (%d Bytes)\n", PK_NCOLS, FE_DATA_BYTES);
    printf("   - Capability : Max %d bit errors correctable\n", SYS_T);
    printf("========================================================================\n");
    printf("|  Error   | Trials | Success | False%% |  Status  \n");
    printf("|----------|--------|---------|--------|----------\n");

    // 1. 안전 구간 테스트 (0 ~ 64비트) -> 모두 성공해야 함
    int safe_tests[] = {0, 10, 30, 50, 60, 62, 63, 64};
    for (int i = 0; i < 8; i++) {
        run_statistical_test(safe_tests[i]);
    }

    printf("|----------|--------|---------|--------|----------\n");

    // 2. 실패 구간 테스트 (65비트 이상) -> 모두 실패해야 함
    // False% (오인식률)가 0%여야 안전한 시스템임
    int fail_tests[] = {65, 66, 70, 80, 100};
    for (int i = 0; i < 5; i++) {
        run_statistical_test(fail_tests[i]);
    }

    printf("========================================================================\n");
    printf(" * Success : Correctly recovered original key.\n");
    printf(" * False%%  : Wrong key recovered (False Accept Rate). Should be 0%%.\n");
    
    FE_Free();
    return 0;
}