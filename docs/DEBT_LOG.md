# 기술부채 로그 (DEBT_LOG)

이번 작업 범위 밖이라 남겨둔 위험 요소를 기록한다. 즉시 해결이 아니라 추적이 목적이다.
해결한 항목은 `## 해결됨` 섹션으로 옮긴다.

## 열린 항목

### [2026-07-31] 기존부채 — TaechangCoverPriceDlg 호출부 없음
- 위치: app/ui/dialogs/TaechangPriceSimpleDlg.h:44 및 대응 .cpp
- 설명: 클래스가 정의되어 있으나 프로젝트 어디서도 DoModal 호출이 없다. 3-A-2에서 버튼 교체 대상을 세다가 발견했다.
- 위험도: 낮음
- 후속: 실제 미사용이면 제거. 표지 단가 입력이 계획된 기능이면 진입 경로를 연결

### [2026-07-31] 구조불일치 — TaechangPriceSimpleDlg 파일명과 내용 불일치
- 위치: app/ui/dialogs/TaechangPriceSimpleDlg.h / .cpp
- 설명: 파일명은 PriceSimpleDlg인데 실제로는 TaechangCompanyRenameDlg와 TaechangCoverPriceDlg 두 클래스가 들어 있다. coding-design의 "파일 하나에 과도하게 많은 클래스를 넣지 않는다"와 어긋난다.
- 위험도: 낮음
- 후속: 클래스당 파일로 분리하고 파일명을 클래스명에 맞춤

### [2026-07-31] 구조불일치 — UI 계층이 infra를 직접 호출
- 위치: app/ui/view/SageTaechangView.cpp, app/ui/dialogs/{TaechangLoginDlg, TaechangPasswordChangeDlg, TaechangCalcEstimateDlg}.cpp
- 설명: coding-design의 의존 방향(ui → core ← infra)을 어기고 SageDBMgr/Repository를 직접 참조한다.
- 위험도: 중
- 후속: Step 4 워크플로 핸들러 도입 시 core 경유로 전환

### [2026-07-31] 구조불일치 — core Service 헤더가 infra Repository 헤더에 컴파일 의존
- 위치: app/core/auth/TaechangUserService.h:5, app/core/price/TaechangPriceService.h:5, app/core/receivable/TaechangReceivableCompanyOrderService.h:5
- 설명: Service 헤더가 Repository 헤더를 include해 core가 infra에 컴파일 타임으로 묶여 있다. core를 독립 라이브러리로 분리하거나 테스트를 붙일 때 걸림돌이 된다.
- 위험도: 중
- 후속: Step 4에서 core에 인터페이스를 두고 infra가 구현하도록 의존 역전

### [2026-07-31] 구조불일치 — infra/office가 infra/db를 직접 참조
- 위치: app/infra/office/TaechangReceivablesExcelService.cpp
- 설명: 문서 생성 모듈이 DB 매니저를 직접 호출한다. 같은 infra 계층 안이지만 관심사가 섞여 있다.
- 위험도: 낮음
- 후속: Step 4에서 core Service 경유로 전환

## 해결됨

### [2026-07-31] 해결 — 앱 헤더가 DB 계층을 전역 노출
- 위치: SageTaechang.h:11
- 조치: CWinApp 헤더의 SageDBMgr.h include를 제거하고 실제 사용처인 SageTaechang.cpp에 직접 추가했다. 이 과정에서 SageTaechangView.h가 SageTaechang.h → SageDBMgr.h → TaechangPriceRepository.h 사슬로 TaechangPriceDto를 간접 획득하고 있었음이 드러나, 전방 선언과 직접 include로 정리했다.
- 커밋: 19d95eb, 21d5679

### [2026-07-31] 해결 — SqlInitializer 파일명과 클래스명 불일치
- 위치: app/infra/db/SqlInitializer.h
- 조치: 클래스명 SQLInitializer를 SqlInitializer로 변경 (13곳). coding-rules의 약어 표기 규칙에 맞춤.
- 커밋: 6dccd52

### [2026-07-31] 철회 — SageDBMgr Getter 7개가 포인터 반환
- 위치: app/infra/db/SageDBMgr.h:36~45
- 사유: 부채가 아니라 **규칙 쪽을 바꿨다.** coding-rules의 "함수 반환 타입에 포인터 사용 금지"를 제거하고, raw pointer 반환을 허용하되 계약을 규약으로 고정하는 방식으로 전환했다 (기본 비소유 / `Get*`은 항상 유효·`Find*`는 NULL 가능 / 소유권 이전은 Create·Detach·Release·Post 네이밍 / 필수 내부 객체는 참조 우선하되 기존 MFC 패턴은 포인터 유지 허용).
- 결과: 현재 코드는 `Get*` + 항상 유효 + 호출부 미검사로 새 규약과 이미 정합하다. 코드 변경 없음.
