cmake_minimum_required(VERSION 3.10)
project(FuzzyExtractor C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# 헤더 파일 경로 지정
include_directories(src lib)

# 컴파일할 소스 파일 목록 (이 파일들이 다 있는지 꼭 확인하세요!)
set(SOURCES
    src/main.c
    src/fe_core.c       # <--- 이 줄이 빠져있을 확률이 높습니다!
    src/bch_wrapper.c
    lib/bch.c
)

# 실행 파일 생성
add_executable(fe_system 
    src/main.c 
    src/fe_core.c 
    src/bch_wrapper.c
    src/fe_api.c        
    lib/bch.c
)