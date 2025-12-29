cmake_minimum_required(VERSION 3.10)
project(FE_System C)

# Release 빌드 + O2 최적화 + 디버그 비활성화
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-O2 -DNDEBUG)

# 헤더 파일 경로
include_directories(lib src)

# 실행 파일 생성
add_executable(fe_system 
    src/main.c 
    src/fe_core.c 
    src/bch_wrapper.c
    src/fe_api.c
    lib/bch.c
)