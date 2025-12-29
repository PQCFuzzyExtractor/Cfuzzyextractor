#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <windows.h> // 고정밀 타이머용 (Windows)

#include "fe_api.h"
#include "bch_wrapper.h"

// ---------------------------------------------------------
// [설정] 팀원 요청 사항 반영
// ---------------------------------------------------------
#define MAX_ERRORS  63    // 0 ~ 63 비트 에러까지 측정
#define NUM_TRIALS  100   // 반복 횟수 100회

// ---------------------------------------------------------
// [유틸리티] 시간 측정 및 통계
// ---------------------------------------------------------
// 윈도우 고정밀 타이머 주파수
double pc_freq = 0.0;
int64_t timer_start = 0;

void timer_init() {
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
        printf("QueryPerformanceFrequency failed!\n");
    pc_freq = (double)li.QuadPart / 1000000.0; // 마이크로초(us) 단위
}

void timer_tic() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    timer_start = li.QuadPart;
}

double timer_toc() {
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (double)(li.QuadPart - timer_start) / pc_freq;
}

// 정렬을 위한 비교 함수 (qsort용)
int compare_doubles(const void *a, const void *b) {
    double arg1 = *(const double *)a;
    double arg2 = *(const double *)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// ---------------------------------------------------------
// [유틸리티] 노이즈 주입
// ---------------------------------------------------------
void inject_random_noise(uint8_t *data, int len, int error_cnt) {
    if (error_cnt <= 0) return;
    
    // 단순화를 위해 랜덤 위치 비트 반전 (중복 위치 허용 안함 로직 추가 가능하지만 성능상 단순화)
    // 여기서는 정확한 개수를 위해 Fisher-Yates 셔플 방식 사용
    int total_bits = len * 8;
    int *indices = (int*)malloc(total_bits * sizeof(int));
    for(int i=0; i<total_bits; i++) indices[i] = i;

    // 섞기
    for(int i=total_bits-1; i>0; i--) {
        int j = rand() % (i+1);
        int t = indices[i]; indices[i] = indices[j]; indices[j] = t;
    }

    // 앞에서부터 error_cnt 개수만큼 반전
    for(int i=0; i<error_cnt; i++) {
        int bit_idx = indices[i];
        data[bit_idx / 8] ^= (1 << (bit_idx % 8));
    }
    free(indices);
}

// ---------------------------------------------------------
// MAIN
// ---------------------------------------------------------
int main() {
    // 1. 초기화
    srand(12345); // 재현 가능성을 위해 시드 고정
    timer_init();
    
    // 변수 준비
    uint8_t input[FE_DATA_BYTES];
    uint8_t noisy_input[FE_DATA_BYTES];
    uint8_t helper[FE_ECC_BYTES];
    uint8_t key_org[FE_KEY_LEN];
    uint8_t key_rec[FE_KEY_LEN];
    size_t h_len = FE_ECC_BYTES;
    size_t k_len = FE_KEY_LEN;
    
    // 2. CSV 헤더 출력 (팀원 요청 포맷)
    printf("errors,attempts,success_rate,mean_us,median_us,p05_us,p95_us,stddev_us\n");

    // 3. 측정 루프 (Error: 0 ~ 63)
    for (int err = 0; err <= MAX_ERRORS; err++) {
        
        double times[NUM_TRIALS];
        int success_count = 0;

        for (int t = 0; t < NUM_TRIALS; t++) {
            // A. 데이터 생성 및 등록 (Enroll)
            // 매번 새로운 데이터로 실험 (캐시 효과 방지 및 일반화)
            for(int i=0; i<FE_DATA_BYTES; i++) input[i] = rand() & 0xFF;
            fe_enroll(input, FE_DATA_BYTES, helper, &h_len, key_org, &k_len);

            // B. 노이즈 주입
            memcpy(noisy_input, input, FE_DATA_BYTES);
            inject_random_noise(noisy_input, FE_DATA_BYTES, err);

            // C. 측정 시작 (Reproduce만 측정)
            timer_tic();
            int ret = fe_reproduce(noisy_input, FE_DATA_BYTES, helper, h_len, key_rec, &k_len);
            double elapsed_us = timer_toc();

            // D. 결과 저장
            times[t] = elapsed_us;
            if (ret == FE_SUCCESS) success_count++;
        }

        // 4. 통계 계산
        // (1) Success Rate
        double success_rate = (double)success_count / NUM_TRIALS;

        // (2) 정렬 (Median, Percentile 계산용)
        qsort(times, NUM_TRIALS, sizeof(double), compare_doubles);

        // (3) Mean
        double sum = 0.0;
        for(int i=0; i<NUM_TRIALS; i++) sum += times[i];
        double mean = sum / NUM_TRIALS;

        // (4) Median (50th), P05 (5th), P95 (95th)
        double median = times[NUM_TRIALS / 2];
        double p05 = times[(int)(NUM_TRIALS * 0.05)];
        double p95 = times[(int)(NUM_TRIALS * 0.95)];

        // (5) StdDev
        double variance_sum = 0.0;
        for(int i=0; i<NUM_TRIALS; i++) {
            variance_sum += pow(times[i] - mean, 2);
        }
        double stddev = sqrt(variance_sum / NUM_TRIALS);

        // 5. CSV 한 줄 출력
        // errors,attempts,success_rate,mean_us,median_us,p05_us,p95_us,stddev_us
        printf("%d,%d,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
            err, NUM_TRIALS, success_rate,
            mean, median, p05, p95, stddev);
    }

    return 0;
}