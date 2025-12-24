# 🔐 C언어 기반 Shortened BCH 퍼지 추출기 (Fuzzy Extractor)

![Language](https://img.shields.io/badge/language-C99-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## 1. 프로젝트 개요 (Overview)
본 프로젝트는 **리눅스 커널(Linux Kernel)** 기반의 고성능 BCH 라이브러리를 포팅하여 구현한 **생체인증용 퍼지 추출기(Fuzzy Extractor)** 시스템입니다.

생체 정보(Biometric Data)와 같이 노이즈가 발생하는 데이터에서 안정적인 **비밀키(Secret Key)**를 생성하고 복구하는 **Secure Sketch** 알고리즘을 C언어로 구현하였습니다.

### 🛠 기술 스펙 (Specification)
| 항목 | 값 | 설명 |
| :--- | :--- | :--- |
| **알고리즘** | BCH Code | Bose-Chaudhuri-Hocquenghem |
| **Galois Field** | $m = 13$ | $GF(2^{13})$, 최대 8191비트 블록 |
| **오류 정정** | $t = 64$ | 최대 **64비트** 오류 정정 가능 |
| **순수 입력 데이터** | **3488 bits** | (436 Bytes) 생체 특징 벡터 |
| **ECC 크기** | **832 bits** | (104 Bytes) Parity Data |
| **전체 블록 크기** | **4320 bits** | (540 Bytes) Data + ECC |

---

## 2. 파일 구조 (File Structure)

```text
Project_Root/
├── CMakeLists.txt        # [빌드] CMake 빌드 설정 파일
├── README.md             # [문서] 프로젝트 설명서
│
├── lib/                  # [엔진] Linux Kernel 기반 BCH 라이브러리
│   ├── bch.c             # BCH 알고리즘 핵심 연산
│   ├── bch.h             # 헤더 파일
│   └── win_compat.h      # 윈도우 호환성 패치
│
└── src/                  # [소스] 퍼지 추출기 구현체
    ├── bch_wrapper.c     # Shortening(단축) 및 Padding 구현
    ├── bch_wrapper.h     # 파라미터(m, t, 길이) 설정 및 매크로
    ├── fe_core.c         # Fuzzy Extractor (Gen/Rep) 로직
    ├── fe_core.h         # API 인터페이스
    └── main.c            # 테스트 시나리오 (20개 케이스)