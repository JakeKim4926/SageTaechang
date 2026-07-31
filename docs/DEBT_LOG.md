# 기술부채 로그 (DEBT_LOG)

이번 작업 범위 밖이라 남겨둔 위험 요소를 기록한다. 즉시 해결이 아니라 추적이 목적이다.
해결한 항목은 `## 해결됨` 섹션으로 옮긴다.

## 열린 항목

### [2026-07-31] 구조불일치 — UI 계층이 infra를 직접 호출
- 위치: app/ui/view/SageTaechangView.cpp, app/ui/dialogs/{TaechangLoginDlg, TaechangPasswordChangeDlg, TaechangCalcEstimateDlg}.cpp
- 설명: coding-design의 의존 방향(ui → core ← infra)을 어기고 SageDBMgr/Repository를 직접 참조한다.
- 위험도: 중
- 후속: Step 5 워크플로 핸들러 도입 시 core 경유로 전환

### [2026-07-31] 구조불일치 — core Service 헤더가 infra Repository 헤더에 컴파일 의존
- 위치: app/core/auth/TaechangUserService.h:5, app/core/price/TaechangPriceService.h:5, app/core/receivable/TaechangReceivableCompanyOrderService.h:5
- 설명: Service 헤더가 Repository 헤더를 include해 core가 infra에 컴파일 타임으로 묶여 있다. core를 독립 라이브러리로 분리하거나 테스트를 붙일 때 걸림돌이 된다.
- 위험도: 중
- 후속: Step 5에서 core에 인터페이스를 두고 infra가 구현하도록 의존 역전

### [2026-07-31] 구조불일치 — 앱 헤더가 DB 계층을 전역 노출
- 위치: SageTaechang.h:11
- 설명: CWinApp 헤더가 app/infra/db/SageDBMgr.h를 include해, 이 헤더를 포함하는 모든 곳에 DB 계층이 노출된다.
- 위험도: 중
- 후속: Step 3에서 의존 제거

### [2026-07-31] 기존부채 — SageDBMgr Getter 7개가 포인터 반환
- 위치: app/infra/db/SageDBMgr.h:36~45
- 설명: coding-rules의 포인터 반환 금지 위반. 호출부 15곳 이상이 null 검사 없이 역참조하고 있어 타입이 표현하는 계약과 실제 사용법이 어긋난다. 실제로는 Initialize 실패 시 앱이 시작되지 않으므로 null이 될 수 없다.
- 위험도: 중
- 후속: Step 2에서 참조 반환으로 전환

### [2026-07-31] 구조불일치 — infra/office가 infra/db를 직접 참조
- 위치: app/infra/office/TaechangReceivablesExcelService.cpp
- 설명: 문서 생성 모듈이 DB 매니저를 직접 호출한다. 같은 infra 계층 안이지만 관심사가 섞여 있다.
- 위험도: 낮음
- 후속: Step 5에서 core Service 경유로 전환

### [2026-07-31] 구조불일치 — SqlInitializer 파일명과 클래스명 불일치
- 위치: app/infra/db/SqlInitializer.h
- 설명: 파일명은 coding-rules의 약어 표기(Sql)로 맞췄으나 클래스명은 SQLInitializer로 남아 있다. Step 1을 "파일 이동만"으로 유지하기 위해 의도적으로 미뤘다.
- 위험도: 낮음
- 후속: Step 2에서 클래스명 변경

## 해결됨
