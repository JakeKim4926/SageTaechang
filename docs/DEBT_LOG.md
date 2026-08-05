# 기술부채 로그 (DEBT_LOG)

이번 작업 범위 밖이라 남겨둔 위험 요소를 기록한다. 즉시 해결이 아니라 추적이 목적이다.
해결한 항목은 `## 해결됨` 섹션으로 옮긴다.

## 열린 항목

### [2026-08-06] 구조불일치 — 폼 라벨 정렬이 화면마다 다르고 목업과도 어긋난다
- 위치: `app/ui/panels/SagePriceCalcPanel.cpp:50,53,55,64,66,68,70,75` (`SS_RIGHT`) ↔ `app/ui/panels/SagePriceManagePanel.cpp:84,87,91,93` (`SS_LEFT`)
- 설명: 단가 계산 패널 라벨은 전부 우측 정렬인데 단가 관리 상세 폼은 좌측 정렬이다. 목업은 「페이지」 하나만 `text-align:right`이고 나머지 라벨은 전부 좌측 정렬이므로 **양쪽 다 목업과 완전히 맞지는 않는다.** D3b는 "라벨 폭 통일"이 범위라 정렬은 손대지 않았다. 폭이 46→64로 커지면서 `SS_RIGHT` 쪽은 라벨 텍스트가 필드에 더 가까이 붙었다(간격 4px).
- 위험도: 낮음
- 후속: **D7**에서 화면별로 목업과 대조할 때 정렬까지 함께 정한다. 정렬을 바꾸면 라벨 텍스트 위치가 눈에 띄게 이동하므로 폭 변경과 같은 커밋에 섞지 않는다

### [2026-08-06] 기존부채 — 단가 계산 입력 라벨에 6px 수동 보정이 남아 있다
- 위치: `TaechangDefine.h` `TAECHANG_CALC_INPUT_LABEL_SHIFT`(=6), `app/ui/panels/SagePriceCalcPanel.cpp:231,236`
- 설명: 「부수」·「페이지」 라벨만 좌표를 6px 왼쪽으로 당긴다. `SS_RIGHT`와 겹쳐서 1행 「법인명」과 텍스트 끝선이 6px 어긋난다. 라벨 폭이 46이던 시절에 눈으로 맞춘 보정값으로 보이며, D3b에서 폭을 64로 올렸어도 그대로 두었다(기존 코드 · 화면 확인 결과 문제없음). `SagePriceManagePanel.cpp:198`의 `TAECHANG_PRICE_COMPANY_LABEL_SHIFT`(=4)도 같은 성격이다.
- 위험도: 낮음
- 후속: 위 정렬 항목과 한 뿌리다. **D7**에서 정렬을 확정할 때 두 SHIFT 상수가 여전히 필요한지 다시 본다. 불필요하면 상수와 참조를 함께 제거한다

### [2026-08-05] 머지위험 — refactor/result-table-panel에 디자인 적용이 누락된다
- 위치: `app/ui/panels/SageResultTablePanel.cpp` (브랜치 `refactor/result-table-panel`, 커밋 `533ab0d`)
- 설명: `fix/design-tokens`는 develop 기준이라 그 패널 파일이 없다. 패널이 머지되면 **디자인 적용이 안 된 표가 하나 생긴다** — `SetHighlightColumns` 인자(3열 → 미수금 1열), `LVS_EX_GRIDLINES` 제거 + `SetRowSeparator`, 그 패널이 만드는 버튼 변형이 대상이다.
- 위험도: **중간** — 같은 화면에 규격이 다른 표 두 개가 생긴다
- 후속: **패널 연결(3-B-4a)을 먼저 머지하고 그 위에 디자인을 얹는다.** 순서가 뒤바뀌면 위 3개를 수동으로 다시 넣어야 한다

### [2026-08-05] 미검증 — 금액 tabular 정렬과 선택 행 Bold를 적용하지 못했다
- 위치: `app/ui/drawing/SageListCtrl.cpp`, 개선안 3장 3-1 · 3-3
- 설명: 목업은 금액을 `font-variant-numeric:tabular-nums` + Bold로, 선택 행을 Bold로 표시한다. GDI `DrawText`는 OpenType `tnum`을 못 켜고, Bold는 볼드 폰트 리소스가 필요하다. 지금 Gmarket 볼드로 맞추면 Pretendard 전환 때 다시 재야 한다. 또 목업 자체가 불일치한다 — 3-3 선택 행은 전 셀 weight 700인데 3-1 선택 행은 합계·입금 셀에 굵기가 없다.
- 위험도: 낮음
- 후속: `DESIGN_PLAN` **D4b**에서 서체 전환(D1b) 직후에 처리한다. tabular는 문서 지시대로 우측 정렬 + Bold로 대체한다

### [2026-08-05] 기존부채 — 참조 없는 상수 2개
- 위치: `TaechangDefine.h` `TAECHANG_LABEL_WIDTH`(=90) · `TAECHANG_RECEIVABLES_COL_IDX_DEPOSIT_AMOUNT`(=6)
- 설명: 둘 다 정의만 있고 참조가 0곳이다. `git grep HEAD`로 **내 변경 이전부터** 참조가 없었음을 확인했다(내 변경이 고아로 만든 `..._COL_IDX_TOTAL_AMOUNT`는 규칙대로 제거함). `TAECHANG_LABEL_WIDTH`는 계획서에 90→80으로 적혀 있었으나 죽은 상수라 바꿔도 효과가 없다. D3b에서 원인을 확인했다 — 이 상수를 쓸 「입력 파일」·「저장 위치」 라벨(`m_wndInputLabel`·`m_wndOutputLabel`)이 생성 후 `ShowWindow(SW_HIDE)`만 되고 `MoveWindow`가 한 번도 불리지 않는다(`SageTaechangView.cpp:270,490`). 계획서 C4에서 이 항목을 「변경 없음」으로 정정했다.
- 위험도: 낮음
- 후속: `DEPOSIT_AMOUNT`는 **D4b**가 금액 3열 우측 정렬에서 다시 쓸 가능성이 있어 그때 판단한다. `TAECHANG_LABEL_WIDTH`는 Step 5 접두사 전환 때 함께 정리한다

### [2026-08-05] 기존부채 — INPUT_PANEL_HEIGHT가 파생값이 아니라 매직넘버다
- 위치: `TaechangDefine.h` `TAECHANG_INPUT_PANEL_HEIGHT`
- 설명: 입력 영역 높이가 `2×SECTION_TITLE_HEIGHT + 3×ROW_GAP + BUTTON_HEIGHT + EDIT_HEIGHT`(=138) + 여유 6인데 값으로 박혀 있다. D3a에서 컨트롤 높이를 32로 올릴 때 136→144를 **손으로 계산해 넣었다.** 다음에 높이를 만지면 또 손계산이 필요하다. 나머지 레이아웃은 상수에서 누적 계산하므로 자동 전파된다.
- 위험도: 낮음
- 후속: `constexpr` 파생식으로 바꾼다. 여유분을 별 상수로 뽑아야 하므로 **D8(DPI 대응)에서 좌표를 손볼 때** 함께 처리한다

### [2026-08-05] 기존부채 — app/ui/drawing/*.cpp에 UTF-8 BOM이 없다
- 위치: `app/ui/drawing/` 하위 `.cpp` 4개 이상 (`SageButton` · `SageListCtrl` · `SageHeaderCtrl` · `SageSidebarTree`)
- 설명: 프로젝트 관례는 UTF-8 BOM + CRLF인데 이 파일들만 BOM이 없다. `git show HEAD`로 **내 편집이 지운 것이 아님**을 확인했다(첫 3바이트 `23 69 6e` = `#in`). 현재 이 파일들은 한글 리터럴이 없어 문제가 없지만, 한글이 들어가는 순간 컴파일러 코드페이지에 따라 깨질 수 있다.
- 위험도: 낮음
- 후속: 이 파일에 한글 문자열을 넣기 전에 BOM을 추가한다. 지금 일괄 추가하면 diff가 전 파일에 퍼져 리뷰가 어려워진다

### [2026-08-05] 중복로직 — DrawEditBorder가 View와 패널에 각각 있다
- 위치: app/ui/view/SageTaechangView.cpp `DrawEditBorder` / app/ui/panels/SagePriceCalcPanel.cpp `DrawEditBorder`
- 설명: 컨트롤 바깥 1px에 테두리를 그리는 같은 8줄이 두 곳에 있다. Step 3-B-1b에서 계산 패널이 자기 영역을 그리게 되면서 생겼다. 패널을 하나 만들 때마다 한 벌씩 늘어난다(3-B-2·3-B-4에서 3벌·4벌).
- 위험도: 낮음
- 후속: 기존 부채 *입력 컨트롤 테두리 방식이 View와 다이얼로그에서 다름*과 같은 뿌리다. `CSageEdit` 승격(`WS_BORDER` + `SetWindowTheme` 전환) 때 한 번에 없앤다

### [2026-08-05] 구조불일치 — core 서비스 인스턴스 획득 방식이 두 가지다
- 위치: app/ui/view/SageTaechangView.cpp `UpdateCalcPreview` / `UpdateCalcTotal`
- 설명: 기존 서비스 6개는 `SageDBMgr`이 보유하고 `Get*`으로 노출하는데, `SagePriceCalcService`만 호출 지점에서 스택 생성한다(2곳). 무상태라 동작 문제는 없지만 다음 core 서비스를 만들 때 어느 쪽을 따를지 근거가 없다. Step 3-B-1a에서 `SageDBMgr`을 고치지 않는 쪽을 택한 결과다.
- 위험도: 낮음
- 후속: Step 4-B 의존 역전에서 서비스 획득 경로를 정할 때 통일한다

### [2026-08-05] 구조불일치 — 견적 입력 표가 행 구조의 범용 멤버를 빌려 쓴다
- 위치: app/core/workflow/handlers/SageEstimateWorkflowHandler.cpp `g_inputColumns` ↔ app/core/workflow/TaechangWorkflowResultPresenter.cpp `AddEstimateInputRows`
- 설명: 견적 입력 표의 단가·표지·운임 컬럼이 각각 `m_strTotalCopies` · `m_strValue` · `m_strReason`에서 값을 가져온다. Presenter가 견적 값을 범용 멤버에 실어 보내기 때문이다. Step 4-7 A에서 컬럼 배열에 데이터 출처를 명시하면서 드러났다. 동작은 정상이고 이제 눈에는 보이지만, 이름과 내용이 어긋나 다음 사람이 오해한다.
- 위험도: 낮음
- 후속: Presenter에 견적 전용 필드(`m_strUnitPrice` · `m_strCoverPrice` · `m_strFreight`)를 추가하고 컬럼 배열을 그쪽으로 돌린다. 화면 변화는 없다

### [2026-08-02] 기존부채 — m_wndPriceCompanyLabel만 가격 패널에서 배경색이 다름
- 위치: app/ui/view/SageTaechangView.cpp `OnCtlColor` (1847~1958, 해당 분기 없음)
- 설명: 가격 데이터 관리 패널의 라벨은 전부 배경 PANEL 분기를 타는데 `m_wndPriceCompanyLabel`만 분기가 없어 맨 끝 `CTLCOLOR_STATIC` 폴백으로 APP 배경이 된다. 3-A-8 4단계 역할 매핑을 뽑다 발견했고 버그로 확인받았다. 4단계에서 같이 고치면 5단계 검증 때 나타난 화면 변화가 의도한 수정인지 이관 실수인지 구분할 수 없어 현행(APP)대로 옮긴다.
- 위험도: 낮음
- 후속: **3-B-2(`SagePriceManagePanel`) 소관.** `LayoutPriceManagePanel`(:2270)이 배치하므로 패널과 함께 이동한다. 이관은 현행(APP)대로 재현하고, `SetBackgroundRole(SAGE_BG_PANEL)` 한 줄은 그 뒤 별도 커밋으로 분리한다. 화면이 실제로 바뀌는 수정이라 눈으로 확인이 필요하다

### [2026-08-01] 기존부채 — 헤더 상태 표시 기능이 통째로 동작하지 않음
- 위치: app/ui/view/SageTaechangView.cpp:372 `m_wndHeaderStatus.Create`, :841 `MoveWindow(0,0,0,0)`
- 설명: `m_wndHeaderStatus`가 `WS_CHILD | SS_RIGHT`로만 생성되어 **`WS_VISIBLE`이 없고**, `ShowWindow` 호출도 없으며 레이아웃에서 크기를 `0,0,0,0`으로 준다. 즉 화면에 표시된 적이 없다. 그런데 이 컨트롤을 위해 멤버 2개(`m_colorHeaderStatus`/`m_nHeaderStatusBgRole`), 함수 2개(`ResolveStatusColor`/`ResolveStatusBgRole`), `OnCtlColor` 분기 5줄, 상수 3개(`TAECHANG_COLOR_STATUS_BG_SUCCESS`/`WARNING`/`ERROR` — 다른 사용처 없음)가 유지되고 있다. `SetStatusText`가 같이 호출하는 `pFrame->SetMessageText`(창 하단 상태바)는 정상 동작하므로 상태 표시 자체가 안 되는 것은 아니다. PR_LOG의 상태별 색 기능(PR #15)에서 도입됐고 컨트롤을 보이게 하는 단계가 빠진 것으로 보인다. 3-A-8 2단계에서 브러시를 이관하다 발견했다.
- 위험도: 낮음
- 후속: **3-B-5b(`SageHeaderPanel`) 착수 전에 결정한다 (계획의 R9).** 숨긴 것이 의도인지 미완성인지 확인이 필요하다. 미완성이면 `WS_VISIBLE` 추가와 `MoveWindow` 좌표 부여, 불필요하면 관련 멤버·함수·분기·상수를 함께 제거. 확인 없이 패널로 옮기면 죽은 코드를 새 패널로 복제한다

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
- 후속: **3-B-1(`SagePriceCalcPanel`)에서 반드시 마주친다.** `m_wndCalcHistoryList`는 `LayoutPriceCalcPanel`(:2510)이 배치하므로 단가 계산 패널로 함께 이동한다. 옮기면서 메시지맵에 등록하면 화면이 바뀌므로 **미등록 상태를 그대로 재현한다.** 적용 여부는 UI 결정 사항이고, 적용하기로 하면 `SetAlternateRowColor(TRUE)` / `SetCenterFirstColumn(TRUE)` 두 줄이면 된다

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
- 위치: app/ui/view/SageTaechangView.cpp, app/ui/panels/SagePriceCalcPanel.cpp, app/ui/dialogs/{TaechangLoginDlg, TaechangPasswordChangeDlg, TaechangCalcEstimateDlg}.cpp
- 설명: coding-design의 의존 방향(ui → core ← infra)을 어기고 SageDBMgr/Repository를 직접 참조한다.
- 위험도: 중
- 후속: View 경로는 **3-B-6b**(`ISageWorkflowRunner` 도입, infra include 6줄 제거), 다이얼로그 3개는 **Step 4-B**로 갈렸다

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

### [2026-08-04] 해결 — 결과 표 판정 규칙이 핸들러와 View 양쪽에 있었다
- 등록: 2026-08-04 (Step 4-4) / 해결: 2026-08-05 (`b2c4dc9`~`842eccf`, Step 4-7)
- 내용: "이 태스크에 전용 표가 있는가" 판정이 핸들러(`Has*Table`)와 View(`Is*Table` 술어 3개)에 이중으로 있었다. 한쪽만 바뀌면 컬럼과 행 내용이 어긋난다.
- 해결: 판정을 핸들러의 `UsesCustomResultTable` 하나로 합쳤다(4-6b). View 술어 3개는 4-7에서 제거 — 행 삽입은 컬럼 배열의 `nField`가 답하고, 입력 표·한 페이지 여부는 `IsInputTableVisible` · `IsOnePageOptionVisible`이 핸들러에 묻는다. 헤더와 데이터가 같은 배열에서 나오므로 어긋남이 구조적으로 불가능해졌다.

### [2026-08-04] 해결 — View의 핸들러 조회 + NULL 검사가 축마다 반복된다
- 등록: 2026-08-04 (Step 4-4) / 해결: 2026-08-04 (`e8a0c56`)
- 내용: `SageWorkflowRegistry::FindHandler(GetSelectedWorkflow())`가 4-4 시점 4곳에서 4-5 시점 9곳으로 늘었다.
- 해결: View 사설 헬퍼 `FindCurrentHandler()`로 모았다. NULL 검사는 반환값이 함수마다 달라 호출부에 남긴다. View.h는 전방 선언만 두고 핸들러 헤더를 물지 않는다.

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
