# 기술부채 로그 (DEBT_LOG)

이번 작업 범위 밖이라 남겨둔 위험 요소를 기록한다. 즉시 해결이 아니라 추적이 목적이다.
해결한 항목은 `## 해결됨` 섹션으로 옮긴다.

## 열린 항목

### [2026-08-04] 중복로직 — 결과 표 판정 규칙이 핸들러와 View 양쪽에 있다
- 위치: app/core/workflow/handlers/ 내 `HasReceivablesResultTable` / `HasDeliveryInputTable` / `HasEstimateInputTable` ↔ app/ui/view/SageTaechangView.cpp `IsReceivablesResultTable` / `IsDeliveryInputTable` / `IsEstimateInputTable`
- 설명: Step 4-4로 "이 태스크에 전용 표가 있는가" 판정이 핸들러에 생겼지만, View의 술어 3개도 같은 규칙(워크플로 + 태스크 조합)을 그대로 들고 있다. View 쪽은 행 삽입·필터·버튼 표시에서 아직 20곳 넘게 쓰여 지울 수 없었다. 한쪽 규칙만 바뀌면 컬럼과 행 내용이 어긋난다.
- 위험도: 중
- 후속: Step 4-7(응답 표시 축)에서 행 삽입이 핸들러로 넘어갈 때 View 술어를 제거한다

### [2026-08-04] 중복로직 — View의 핸들러 조회 + NULL 검사가 축마다 반복된다
- 위치: app/ui/view/SageTaechangView.cpp `UpdateWorkflowLabels`, `ApplyWorkflowTabs`, `GetTaskTabVisualIndex`, `GetTaskTabSemanticIndex`
- 설명: `SageWorkflowRegistry::FindHandler(GetSelectedWorkflow())` + NULL 검사가 4곳이며 Step 4-4~4-7에서 축이 늘 때마다 증가한다. 지금 헬퍼를 만들면 선행 일반화라 미뤘다.
- 위험도: 낮음
- 후속: Step 4-4 착수 시 호출부 개수를 다시 세고 View 전용 헬퍼 도입 여부를 결정한다

### [2026-08-02] 기존부채 — m_wndPriceCompanyLabel만 가격 패널에서 배경색이 다름
- 위치: app/ui/view/SageTaechangView.cpp `OnCtlColor` (1847~1958, 해당 분기 없음)
- 설명: 가격 데이터 관리 패널의 라벨은 전부 배경 PANEL 분기를 타는데 `m_wndPriceCompanyLabel`만 분기가 없어 맨 끝 `CTLCOLOR_STATIC` 폴백으로 APP 배경이 된다. 3-A-8 4단계 역할 매핑을 뽑다 발견했고 버그로 확인받았다. 4단계에서 같이 고치면 5단계 검증 때 나타난 화면 변화가 의도한 수정인지 이관 실수인지 구분할 수 없어 현행(APP)대로 옮긴다.
- 위험도: 낮음
- 후속: 3-A-8 완료 후 별도 커밋으로 `SetBackgroundRole(SAGE_BG_PANEL)` 한 줄. 화면이 실제로 바뀌는 수정이라 눈으로 확인이 필요하다

### [2026-08-01] 기존부채 — 헤더 상태 표시 기능이 통째로 동작하지 않음
- 위치: app/ui/view/SageTaechangView.cpp:372 `m_wndHeaderStatus.Create`, :841 `MoveWindow(0,0,0,0)`
- 설명: `m_wndHeaderStatus`가 `WS_CHILD | SS_RIGHT`로만 생성되어 **`WS_VISIBLE`이 없고**, `ShowWindow` 호출도 없으며 레이아웃에서 크기를 `0,0,0,0`으로 준다. 즉 화면에 표시된 적이 없다. 그런데 이 컨트롤을 위해 멤버 2개(`m_colorHeaderStatus`/`m_nHeaderStatusBgRole`), 함수 2개(`ResolveStatusColor`/`ResolveStatusBgRole`), `OnCtlColor` 분기 5줄, 상수 3개(`TAECHANG_COLOR_STATUS_BG_SUCCESS`/`WARNING`/`ERROR` — 다른 사용처 없음)가 유지되고 있다. `SetStatusText`가 같이 호출하는 `pFrame->SetMessageText`(창 하단 상태바)는 정상 동작하므로 상태 표시 자체가 안 되는 것은 아니다. PR_LOG의 상태별 색 기능(PR #15)에서 도입됐고 컨트롤을 보이게 하는 단계가 빠진 것으로 보인다. 3-A-8 2단계에서 브러시를 이관하다 발견했다.
- 위험도: 낮음
- 후속: **숨긴 것이 의도인지 미완성인지 확인이 필요하다.** 미완성이면 `WS_VISIBLE` 추가와 `MoveWindow` 좌표 부여, 불필요하면 관련 멤버·함수·분기·상수를 함께 제거

### [2026-08-01] 기존부채 — m_brushListHeader가 생성만 되고 쓰이지 않음
- 위치: app/ui/view/SageTaechangView.cpp:277 (생성자)
- 설명: `CreateSolidBrush(TAECHANG_COLOR_LIST_HEADER)`로 만들지만 반환·사용하는 곳이 없다. 리스트 헤더 배경은 `CSageHeaderCtrl::OnPaint`가 `FillSolidRect`로 직접 칠하므로 남은 흔적이다. 3-A-8 2단계에서 나머지 브러시 4개를 저장소로 옮기면서 드러났다. 이번 변경이 만든 것이 아니라 그대로 뒀다.
- 위험도: 낮음
- 후속: 멤버 선언과 생성 2줄 제거

### [2026-08-01] 구조불일치 — 컨트롤의 폰트 선택 가드가 6곳 중 2곳만 다름
- 위치: app/ui/drawing/SageButton.cpp:57, app/ui/drawing/SageSectionLabel.cpp:17
- 설명: 그리기 중 폰트를 거는 6곳 가운데 `CSageTabCtrl`·`CSageHeaderCtrl`·`CSageFilterComboBox`(2곳)는 `pFont ? dc.SelectObject(pFont) : NULL`로 가드하는데, 이 2곳은 `pDC->SelectObject(GetFont())`로 가드가 없다. `GetFont()`가 NULL이면 디버그 빌드에서 ASSERT가 뜬다. 실제로는 두 컨트롤 모두 `ApplyControlFonts`에서 항상 `SetFont`을 받으므로 재현되지 않는다. 3-A-7 중복 조사 중 발견했고, 고치면 3-A-7 범위(콤보 화살표 추출)가 흐려져 남겼다.
- 위험도: 낮음
- 후속: 폰트 저장소 작업(`ApplyControlFonts` 해체) 때 6곳을 한 번에 통일한다. 그때 컨트롤이 자기 폰트를 갖게 되므로 가드 방식도 함께 정해진다

### [2026-08-01] 기존부채 — Excel COM 최적화 설정이 없음
- 위치: app/infra/office/TaechangReceivablesExcelService.cpp (및 office 계층 전반)
- 설명: `ScreenUpdating` / `Calculation` / `EnableEvents` / `DisplayAlerts` 설정이 하나도 없다. 3-A-5에서 미수금 생성 지연을 조사하다 발견했다. UI 쪽 O(N^2) 리페인트는 `SetRedraw`로 해결했으나 워커 쪽 개선 여지는 남아 있다. 셀 접근 패턴(범위 일괄 읽기 vs 셀 단위 접근)도 아직 확인하지 않았다.
- 위험도: 낮음
- 후속: 성능이 다시 문제되면 조사한다. COM 작업이라 검증이 별도로 필요하고 3-A 범위를 벗어난다

### [2026-07-31] 기존부채 — 계산 내역 리스트에 커스텀드로우가 적용되지 않음
- 위치: app/ui/view/SageTaechangView.cpp 메시지맵 / `OnListCustomDraw`
- 설명: `OnListCustomDraw`에 `ID_CALC_HISTORY_LIST` 분기가 있으나 메시지맵에 `ON_NOTIFY(NM_CUSTOMDRAW, ID_CALC_HISTORY_LIST, ...)` 등록이 없어 실행된 적이 없다. 그래서 계산 내역 리스트만 짝수/홀수 배경색과 첫 컬럼 가운데 정렬이 적용되지 않는다. 3-A-5에서 `CSageListCtrl` 승격 중 발견했고, 승격 시 자동으로 적용되면 화면 변경이므로 OFF로 현재 동작을 재현했다.
- 위험도: 낮음
- 후속: 계산 내역 리스트에 다른 리스트와 같은 스타일을 적용할지 UI 결정 필요. 적용하기로 하면 `SetAlternateRowColor(TRUE)` / `SetCenterFirstColumn(TRUE)` 두 줄이면 된다

### [2026-07-31] 구조불일치 — 입력 컨트롤 테두리 방식이 View와 다이얼로그에서 다름
- 위치: app/ui/view/SageTaechangView.cpp `DrawEditBorder` / app/ui/dialogs/*.cpp
- 설명: View는 `WS_BORDER` 없이 부모 `OnDraw`에서 `DrawEditBorder`로 **컨트롤 바깥 1px**에 그린다(대상 16개, 콤보박스 3개 포함). 다이얼로그는 `WS_BORDER`로 시스템이 그린다. 3-A-4에서 컨트롤 승격을 검토했으나, 테두리가 컨트롤 영역 밖이라 리플렉션으로 옮기면 픽셀 위치가 1px 이동한다. 3-A의 완료 기준이 화면 무변화이므로 **현행 유지로 결정**했다.
- 위험도: 낮음
- 후속: 현행 유지. 향후 테마·디자인 변경 시 `WS_BORDER` + `SetWindowTheme` 전환을 별도로 검토한다. 검토 시 화면 변화 검증이 필요하다

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

### [2026-08-04] 해결 — 탭 semantic 상수 3쌍이 같은 값이라 분기가 무의미
- 등록: 2026-08-04 (Step 4-3) / 해결: 2026-08-04 (`b2e8169`)
- 내용: PREVIEW(1)==DOCUMENT_RESULT(1), RESULT(2)==DOCUMENT_HISTORY(2), DETAIL(3)==DATA_MANAGE(3)으로 값이 겹쳐 `IsResultTab`·`IsDetailTab`의 워크플로 분기가 같은 숫자를 비교하고 있었다.
- 해결: 겹치던 세 상수가 전부 검수 워크플로 전용이라 PDF·HWP 제거와 함께 사라졌다. 남은 탭 인덱스는 문서용 0~3뿐이고 중복 값이 없다. 술어의 분기도 함께 접혔다.

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
