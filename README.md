# SageTaechang

SageTaechang은 인쇄/문서 업무에서 반복되는 엑셀 데이터 확인, 문서 생성, 단가 관리, 파일 검수 작업을 한 화면에서 처리하기 위한 Windows MFC 기반 업무 자동화 프로그램입니다.

엑셀 입력 파일을 불러와 표 형태로 검토하고, 필요한 행을 선택해 문서를 생성하며, 단가 데이터와 정렬 기준을 SQLite DB로 관리합니다. PDF/HWP 표지 검수와 CSV 내보내기 기능도 함께 제공합니다.

## 주요 기능

### 문서 생성

- 미수금 내역서 생성
- 납품서 생성
- 견적서 생성
- 입력 엑셀 파일 로드 및 결과 표 표시
- 행 선택 기반 문서 생성
- 결과 검색, 초기화, 탭 전환 상태 유지
- 견적서 한 페이지 작성 옵션
- 견적서 최근 생성 내역 날짜 컬럼 표시

### 단가 관리

- 법인별 단가 데이터 등록, 수정, 삭제
- 부수 범위별 부수 단가와 표지 단가 관리
- 단일 부수 모드 지원 (단일 부수 체크박스)
- 최대부수 없음 범위 처리
- 법인명 선택 미니 다이얼로그
- 부수와 페이지 기준 금액 계산
- 계산 탭에서 직접 견적서 생성
- 최근 계산 내역 표시 (품목명·날짜 포함)

### 미수금 정렬 기준 관리

- 데이터 관리 탭에서 법인 정렬 순서 CRUD
- 미수금 내역서의 법인 정렬 순서 DB 관리
- 기존 엑셀 기준 정렬 대신 사용자 설정 순서 적용
- 법인명은 유지하면서 기타 정렬 기준 처리
- 법인명 검색 및 중복 저장 방지

### 파일 검수

- PDF 표지 검수
- HWP 표지 검수
- 다중 파일 선택
- 검수 결과 표시
- CSV 내보내기

### 사용자 관리

- 관리자 로그인
- 비밀번호 변경
- 관리자 전용 단가 관리 메뉴 접근 제어

## 기술 스택

- C++20
- MFC
- Win32
- SQLite
- PowerShell 자동화 스크립트
- Visual Studio C++ toolset

## 프로젝트 구조

```text
SageTaechang/
  app/
    application/services/       업무별 서비스 계층
    common/                     공통 유틸리티
    infrastructure/bridge/      응답 생성 및 브릿지 보조 코드
    presentation/               결과 표시 변환 계층
  external/sqlite/              SQLite 소스
  templates/                    문서 생성용 템플릿 파일 로컬 배치 위치
  tools/                        엑셀 로드 및 문서 생성 PowerShell 스크립트
  SageTaechangView.*            메인 업무 화면
  SageDBMgr.*                   DB 서비스 구성
  SQLContext.*                  SQLite 연결 관리
  SQLInitializer.*              DB 테이블 초기화
  Taechang*Repository.*         DB 접근 계층
  Taechang*Service.*            도메인 서비스 계층
```

## 주요 파일

- `SageTaechang/SageTaechangView.cpp`
  메인 화면, 메뉴 전환, 문서 생성 흐름, 단가 관리 UI, 결과 테이블 표시를 담당합니다.

- `SageTaechang/app/application/services/`
  미수금, 납품서, 견적서, PDF/HWP 검수, CSV 내보내기 등 업무 단위 서비스를 포함합니다.

- `SageTaechang/SageDBMgr.cpp`
  SQLite 컨텍스트, Repository, Service 객체를 구성하고 앱 전역에서 사용할 수 있게 관리합니다.

- `SageTaechang/SQLInitializer.cpp`
  `TaechangPrice`, `TaechangUser`, `TaechangReceivableCompanyOrder` 테이블을 초기화합니다.

- `SageTaechang/tools/`
  문서 생성과 엑셀 데이터 로드를 수행하는 PowerShell 스크립트가 들어 있습니다.

- `SageTaechang/templates/`
  미수금 내역서, 납품서, 견적서 생성에 사용하는 템플릿 파일을 로컬에 배치하는 위치입니다.
  실제 템플릿 파일은 회사 보안 자료이므로 저장소에 포함하지 않고 별도로 관리합니다.

## 빌드 환경

Windows와 Visual Studio C++ 개발 환경이 필요합니다.

필요 구성:

- Visual Studio
- Desktop development with C++
- MFC
- Windows 10 SDK
- PowerShell

지원 구성:

- `Debug|Win32`
- `Release|Win32`
- `Debug|x64`
- `Release|x64`

## 빌드 방법

Visual Studio에서 `SageTaechang.slnx`를 열고 원하는 구성을 선택해 빌드합니다.

명령줄에서는 MSBuild로 빌드할 수 있습니다.

```powershell
MSBuild.exe SageTaechang\SageTaechang.vcxproj /p:Configuration=Debug /p:Platform=Win32 /m
```

빌드 결과는 구성에 따라 아래 폴더에 생성됩니다.

- `Debug_x32/`
- `Release_x32/`
- `Debug_x64/`
- `Release_x64/`

빌드 후 `tools/` 폴더는 실행 폴더로 복사됩니다.
`templates/` 폴더는 로컬에 존재하는 경우에만 실행 폴더로 복사됩니다.

## 실행 전 확인 사항

- 문서 생성 기능은 로컬 `SageTaechang/templates/`의 템플릿 파일을 사용합니다.
- 템플릿 파일은 회사 보안 자료이므로 저장소에 포함하지 않습니다. 문서 생성 기능을 실행하려면 로컬 환경에 필요한 템플릿 파일을 별도로 배치해야 합니다.
- 엑셀 로드와 문서 생성은 `SageTaechang/tools/`의 PowerShell 스크립트를 통해 처리됩니다.
- 단가, 사용자, 미수금 정렬 기준 데이터는 SQLite DB에 저장됩니다.
- 관리자 기능을 사용하려면 로그인해야 합니다.

## 개발 규칙

- `main`에는 직접 커밋하지 않습니다.
- 작업은 기능 단위 브랜치에서 진행하고 PR을 통해 반영합니다.
- PR 생성 및 머지 이력은 `docs/decisions/PR_LOG.md`에 기록합니다.
- C++ 코드 수정 시 로컬 코딩 규칙을 따릅니다.

## 참고

이 저장소에는 실제 업무 자동화를 위한 스크립트가 포함되어 있습니다. 외부 공유 전에는 샘플 데이터, 경로, 회사명, 거래처명, 금액 정보 등 민감한 업무 정보가 포함되어 있지 않은지 반드시 확인해야 합니다.
