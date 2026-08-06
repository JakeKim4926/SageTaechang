## [2026-08-07] fix/large-table-render
- **목적**: 690행 견적서 파일에서 로드·워크플로 전환·필터 초기화가 매번 약 1초 멈추는 것을 줄인다 (`DEBT_LOG` 후속 ①)
- **변경 내용**: `SageResultTablePanel::RefreshRows`의 삽입 루프를 `LPSTR_TEXTCALLBACK`으로 전환해 **행당 메시지를 10회에서 1회로** 줄였다(`InsertItem` 1 + `SetItemText` 8 + `SetItemData` 1 → `LVIF_TEXT|LVIF_PARAM` 삽입 1회). 셀 문자열은 패널이 `LVN_GETDISPINFO`에 답하며 **그리는 행에 한해** 만든다 — 답할 데이터(`m_arrVisibleRows` · `m_arrColumns`)를 가진 쪽이 패널이므로 콜백도 패널이 받는다. **`CSageListCtrl`에는 아무것도 넘기지 않았다**(컨트롤은 도메인 개념을 알면 안 된다). **착수 전 전수 조사로 가부를 확정했다** — 이 리스트의 텍스트를 외부에서 읽는 코드 0곳(`GetItemText`는 `SageListCtrl.cpp` 내부 5곳뿐이고 전부 그리는 행 한정), `SortItems` 0곳, 결과 필터는 `m_arrRows`를 직접 훑는다. `SagePriceManagePanel`의 `GetItemText` 6곳은 **다른 리스트**였다. **`push_back`을 `InsertItem`보다 앞에 뒀다** — 콜백 구조에서는 컨트롤이 삽입 도중에도 텍스트를 되물을 수 있다. **재측정은 하지 않았다** — 빌드와 690행 화면(속도 체감 · 표 내용 동일)을 사용자가 확인했고, `DEBT_LOG`의 「약 270ms」는 계산된 예상값이지 잰 값이 아니다. `InsertItem` 221ms(가상 리스트 전환분)와 리스트 인덱스 전제는 `DEBT_LOG`에 남겼다
- **PR 링크**: 없음
- **결과**: pending — `develop` 미머지. 빌드 + 화면 확인 완료

## [2026-08-06] feature/estimate-one-page-inline
- **목적**: 「한 페이지 작성」을 목업대로 테두리 박스에 담고 6행 제한을 상시 안내해 모달 경고창을 없앤다 (D7-8 2단계, D7-7·D7-8 완결)
- **변경 내용**: `CSageOptionCheck` 신설 — 테두리 박스 + 체크박스 + 라벨 + 힌트를 **스스로 그리고 폭을 실측**한다(고정 폭 상수 `TAECHANG_ESTIMATE_ONE_PAGE_WIDTH`는 참조 0곳이 되어 제거). `SageUiStyle::DrawCheckBox`를 `CSageListCtrl`에서 올려 표 체크박스와 공유했다 — 스킬이 *「두 컨트롤이 실제로 같은 코드를 가질 때만 모은다」*고 한 조건에 처음 해당했고 `SageUiStyle`의 두 번째 조각이다. **`AfxMessageBox` 3곳 제거**(패널 내 참조 0곳). 초과분은 지금도 자동 해제되고 힌트가 상시 보이므로 사전 차단된다. **착수 전 목업 재대조에서 2건을 잡았다** — 박스 좌우 여백은 목업 10이지만 「간격은 4의 배수」 규칙에 따라 8, 그리고 목업 3-8에는 「선택 해제」가 없지만 입력 표가 납품·견적 공용 인스턴스라 양쪽에 보인다(`DEBT_LOG`). **작업 중 잡은 것**: 「선택 해제」가 잘렸다 — 선택 바 폭이 `Layout()` 시점 건수(「0건 중 0건」)로 정해지는데 690행이 들어오면 「690건 중 6건」으로 텍스트가 길어져 버튼이 바 밖으로 밀렸다. `LayoutSelectionRow()`를 분리해 건수가 바뀔 때도 재배치한다. **생성 버튼 문구는 고정으로 되돌렸다**(사용자 결정) — `BuildActionButtonLabel` 확장점과 포맷 상수 2개를 걷어냈고 0건 비활성은 유지한다. **성능**: 690행에서 로드·전환·초기화가 각각 약 1초 멈추는 것을 판별 실험 3개로 「690행에 도착하는 비용」으로 좁힌 뒤, 일회용 계측 브랜치(`perf/measure-refresh-rows`, 폐기)로 구간을 갈라 **`InsertItem` 221ms · `SetItemText` 305ms**로 확정했다 — 문자열 생성 5ms, 행 복사 1ms, 이번에 추가한 선택바 동기화 1ms는 무관했다. `SetItemCount` 예약 가설은 측정으로 폐기(526 → 523ms). 수치와 후속(`LPSTR_TEXTCALLBACK` → 가상 리스트)은 `DEBT_LOG`
- **PR 링크**: 없음
- **결과**: pending — `develop` 미머지. 빌드 + 화면 확인 완료

## [2026-08-06] feature/table-selection-bar
- **목적**: 입력 표에 선택 바를 올려 D7-7·D7-8의 공통 부분을 끝내고(1단계), 작업 중 드러난 체크박스 회귀를 함께 차단
- **변경 내용**: `CSageSelectionBar` 신설 — 「전체 선택」 체크박스와 「선택 해제」를 **소유하고 클릭은 커맨드 ID로 부모에 올린다**(`CSageEmptyState` 선례). 「N건 중 **M건** 선택됨」에서 건수만 `PRIMARY` SemiBold로 그리며, 이를 위해 `SAGE_FONT_CONTENT_SEMIBOLD`(105 SemiBold가 없었다)를 신설했다. 폭은 상수로 박지 않고 **바가 자기 텍스트를 GDI로 실측**해(`GetContentWidth`) 패널이 그 뒤에 「한 페이지 작성」을 붙인다. 생성 버튼 문구는 `ISageWorkflowHandler::BuildActionButtonLabel(int)`로 **핸들러가 답한다**(미수금은 기존 문구). 체크 변경은 `WM_TAECHANG_RESULT_SELECTION_CHANGED`로 흐르며 **행 집합 변경과 분리**했다 — 체크 토글마다 요약 바·합계 밴드를 재계산할 이유가 없다. 대량 `SetCheck` 구간은 알림을 억제하고 끝난 뒤 한 번만 보낸다. **입력 표는 납품·견적 공용 인스턴스**라 선택 바가 두 화면에 함께 붙었다. **겹침 수정**: 밴드 행 컨트롤은 32px인데 그 행에 26px만 할당되어 있어 표와 **4px 겹쳐 있었다**(미수금은 요약 바가 밀어내서, 납품·견적은 겹치는 폭이 버튼 하나뿐이라 가려져 있었다) — 선택 바가 보일 때 리스트 시작점을 밴드 아래로 내렸다. **함께 고친 D4 회귀**: `f4690b8`(행 높이 34) 이후 입력 표의 체크박스가 보이지 않았다. `CSageListCtrl`이 상태 이미지 2장을 직접 그려 `LVSIL_STATE`에 지정한다(목업대로 카멜색). **확인 빌드를 4회 썼고 원인이 셋이었다** — ① 스타일을 켜기 전에 이미지를 물려 자동 생성분에 덮어써짐 ② 상태 이미지 인덱스가 **1-based**라 0번을 빈 이미지로 두어 「해제는 안 보이고 선택하면 빈 박스」가 됨 ③ `SetColumns`가 확장 스타일을 통째로 덮어써 체크박스 스타일이 꺼질 때 **comctl32가 상태 이미지리스트를 파괴**하는데 멤버로 재사용해 두 번째 워크플로부터 죽은 핸들을 물었다 — 매번 새로 만들어 `Detach`로 소유권을 넘기는 것으로 해결. **기록 정정**: 첫 수정 커밋(`defb468`)의 메시지는 원인을 「1×34 더미 이미지리스트 때문」이라고 단정했으나 **그 인과는 끝내 증명되지 않았다.** 현재 구현은 자동 생성분을 쓰지 않아 원인과 무관하게 동작한다. 커밋 순서가 `fix → feat → fix ×3`으로 섞여 있어 이력 재작성 위험이 정정 이득보다 커 메시지는 그대로 두고 여기에 남긴다
- **PR 링크**: 없음
- **결과**: pending — `develop` 미머지. 빌드 + 화면 확인 완료(납품 → 견적 → 납품 전환 포함)

## [2026-08-06] feature/receivables-total-row
- **목적**: 미수금 결과 표 하단에 합계 밴드를 올려 D7-1을 마무리 (2단계)
- **변경 내용**: `CSageTableTotalBar` 신설(`app/ui/drawing/`) — 셀 좌표·텍스트·표현 역할만 받고 워크플로와 컬럼을 모른다. 합계 데이터는 `ISageWorkflowHandler::BuildResultTotals`로 **핸들러가 답한다**(납품·견적은 FALSE) — 1단계 `BuildResultSummary`와 같은 확장점 형태다. **요약 바와 밴드가 같은 수치를 두 번 세지 않도록** 핸들러의 `SumVisibleRows` 한 곳에서 합산하고 둘이 그 결과를 쓴다. 셀 좌표는 **폭이 정해지는 유일한 지점인 `UpdateColumnWidths()` 안에서만** 다시 계산해 밀어넣는다 — 밴드가 리스트를 참조하면 그리기 위젯이 리스트를 알게 되고, 폭을 캐시하면 창 크기 변경 시 어긋난다. **작업 중 잡은 것**: 리스트가 `WS_BORDER`로 생성되어 **컬럼 좌표(클라이언트 기준)가 리스트 창 rect보다 1px 안쪽**이라 밴드를 `x=0`에 두면 셀이 1px 밀린다 — `ClientToScreen`→`ScreenToClient`로 실제 오프셋을 구해 더했다(상수 1을 박지 않았다). **색은 목업대로 muted 2색을 한 줄에 썼고**(「합계」 `#6E655B` · 「N건」 `#7A7064`) 이를 위해 `sagetaechang-ui`의 「두 색을 섞어 쓰지 않는다」를 역할별 열거로 교체했다. 이때 초안이 「라벨은 `#6E655B`」여서 **1단계 요약 바(라벨 `#7A7064`)를 규칙 위반으로 만들 뻔한 것을 재점검에서 잡았다** — 규칙을 고칠 때는 그 규칙이 설명해야 할 기존 사례를 전부 대조한다. 폰트는 목업 12px가 아니라 **표 셀 13px 계열**을 썼고(타입 스케일 우선), 그 과정에서 스킬에 남아 있던 낡은 폰트 값(표 셀 9pt · 캡션 8.3pt)이 코드(98 · 90)와 어긋난 것을 발견해 참조로 정리했다
- **PR 링크**: 없음
- **결과**: pending — `develop` 미머지. 빌드 + 화면 확인 완료

## [2026-08-06] feature/receivables-summary-bar
- **목적**: 미수금 결과 표 상단에 요약 바를 올리고(D7-1 1단계), 작업 중 발견한 Excel 프로세스 누수를 함께 차단
- **변경 내용**: `CSageSummaryBar` 신설(`app/ui/drawing/`) — 라벨·수치·단위 문자열만 받고 워크플로 타입을 모른다. 요약 데이터는 `ISageWorkflowHandler::BuildResultSummary`로 **핸들러가 답한다**(미수금만 채우고 납품·견적은 FALSE) — View에 워크플로 분기를 만들지 않기 위한 확장점이다. **계획서 전제가 틀렸다**: `RECEIVABLES_PREVIEW_TOTAL`·`MISSING_COMPANIES` 두 상수가 참조 0곳이었고 미수금 합계를 계산하는 코드가 없었다. 행 금액은 `FormatAmountCellText`로 포맷된 **문자열만** 있어 더할 수 없었으므로 `TaechangResultRow`에 `__int64` 3종을 함께 담고 `FormatAmountNumber`를 추가했다. `missingCompanies`는 PowerShell이 이미 내보내던 것을 처음 읽었다. 건수·합계는 **보이는 행 기준**으로 갱신하고(기존 `WM_TAECHANG_RESULT_TABLE_CHANGED` 흐름에 얹음) **구분 표기(`-`) 행은 건수에서 제외**한다 — 데이터 구분선이지 미수금 건이 아니다. 폰트는 `sagetaechang-ui` 타입 스케일대로 `SAGE_FONT_SUMMARY`(128 = 17px SemiBold)를 신설했다(C7의 16px 기술과 어긋나며 스킬 표를 따랐다). **성능 조사**: 사용자가 내역서 생성이 느려졌다고 해 추적한 결과 원인은 요약 바가 아니라 **워크플로 스크립트 7개가 띄운 Excel 인스턴스가 남는 것**이었다 — 좀비가 쌓이며 생성이 4초에서 12초까지 늘었다. COM 참조 해제 + GC 방식은 측정해보니 누수를 막지 못하고 실행당 수 초를 더 써서 폐기하고, 프로세스 목록 차이로 **자기가 띄운 PID만 특정해 종료**하는 방식(`tools/excel-process.ps1`, 7개가 dot-source)으로 바꿨다. 리빌드 후 생성 3회 연속 4초대 유지 확인. **진단 교훈**: `Handles=0`·`WS 44KB` EXCEL 항목은 이미 죽은 껍데기이므로 프로세스 개수로 누수를 판정하면 안 된다
- **PR 링크**: 없음
- **결과**: merged — `develop`에 fast-forward 머지(`7926a1c`, 2026-08-06). 커밋 3개가 `feat`·`fix`·`docs` 세 목적이라 squash하지 않고 보존했다. 작업 브랜치 삭제. D7-1 2단계(합계 밴드)는 `feature/receivables-total-row`

## [2026-08-02] refactor/sage-label-roles
- **목적**: 라벨이 역할로 자기 색과 폰트를 내게 해 View의 색상 분기를 걷어냄 (3-A-8 4~6단계, Step 3-A 완결)
- **변경 내용**: View의 `CStatic` 39개 중 **37개를 `CSageLabel`로 교체**하고 `ApplyLabelRoles`에서 `(텍스트/배경/폰트)` 역할을 지정. 착수 전 39개의 매핑을 표로 확정해 확인받고, 코드 작성 후 스크립트로 `OnCtlColor`·`ApplyControlFonts`의 기존 값과 대조(불일치 0건). 이후 `OnCtlColor`에서 라벨 분기를 제거하고 `IsKindOf(RUNTIME_CLASS(CSageLabel))` 조기 반환 2줄로 대체(**112 → 31줄**), `ApplyControlFonts`에서 라벨 34줄 제거(**114 → 76줄**). 리플렉션 동작은 추측하지 않고 MFC 원본에서 확인 — `CWnd::OnCtlColor`(`wincore.cpp:4308`)가 `SendChildNotifyLastMsg`로 자식에게 먼저 넘기고 자식이 반환한 브러시를 그대로 돌려주므로, 부모는 그 반환값을 조기 반환하면 된다. **동적 색 2개(`m_wndHeaderStatus`·`m_wndActionStatus`)는 `CStatic`으로 잔류** — 역할이 고정값인 `CSageLabel`로는 런타임 색 변경을 담을 수 없다. 체크박스 2개와 기본값 2개도 잔류. 구분선 3개는 원래대로 폰트 미지정. **줄 수는 6줄만 줄었다**(분기 119줄 제거, 역할 지정 113줄 추가) — 얻은 것은 길이가 아니라 View가 라벨 색을 판단하지 않게 된 것이고, 3-B에서 라벨이 역할을 들고 함께 이동한다는 점이 실제 이득이다. 작업 중 확인된 `m_wndPriceCompanyLabel` 배경색 버그는 5단계 검증을 흐리지 않도록 현행 유지하고 DEBT_LOG에 남김
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-02] refactor/sage-ui-resources
- **목적**: 색상과 폰트를 한 저장소로 묶기 위한 기반 마련 (3-A-8 1~3단계)
- **변경 내용**: `namespace SageUiResources` 신설 — 폰트 4개 + 배경 브러시 6개를 소유하고 역할 enum 3종(`SageFontRole` 4 / `SageBackgroundRole` 6 / `SageTextRole` 7)으로 꺼내 쓴다. **폰트가 `ApplyControlFonts` 안에서 생성되던 구조**(두 번 호출하면 안 됨)를 `InitInstance`/`ExitInstance` 수명으로 옮겨 해소했고, `LoadPrivateFonts()` 이후여야 한다. View의 폰트 멤버 4개와 브러시 멤버 4개 제거, 헤더 상태 브러시의 `DeleteObject`/`CreateSolidBrush` 재생성도 소멸. 컨트롤 99개의 폰트 역할 배정이 이전과 1:1 동일함을 스크립트로 대조(불일치 0건). `CSageLabel`을 신설했으나 **아직 아무도 쓰지 않는다** — 기본값 `SAGE_TEXT_DEFAULT`+`SAGE_BG_APP`가 View의 기존 폴백과 같아 다음 단계에서 역할 지정을 빠뜨려도 색이 유지되는 안전망이다. `GetBrush`는 `HBRUSH`를 반환해야 한다(`CBrush*`는 C2440 — `CBrush::operator HBRUSH()`가 포인터에는 적용되지 않는다). 작업 중 죽은 코드 2건(헤더 상태 기능 전체, `m_brushListHeader`)과 배경색 버그 1건(`m_wndPriceCompanyLabel`) 발견해 DEBT_LOG 기록. **3-A-8은 6단계 중 3단계까지이며 4~6단계는 별도 브랜치로 이어간다**
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-01] refactor/sage-ui-style
- **목적**: 컨트롤 간 실제 중복을 조사하고 공유 그리기 조각을 추출 (3-A-7)
- **변경 내용**: 컨트롤 7개를 전수 조사한 결과 **진짜 중복은 콤보 화살표 삼각형 14줄 하나**였고, SageComboBox와 SageFilterComboBox가 들여쓰기만 다른 동일 코드를 갖고 있었다. `namespace SageUiStyle`에 `DrawComboArrow` 하나만 두고 두 곳을 한 줄 호출로 교체(28줄 제거). **추출하지 않은 후보 3건과 이유를 함께 기록** — 배경 채우기(컨트롤마다 색이 다르고 FillSolidRect 한 줄), 테두리(CSageButton Secondary 한 곳뿐), 가운데 텍스트(6곳이나 색·rect·포맷이 매번 달라 관용구지 로직이 아님). SageUiStyle을 색상 파사드로 만드는 안은 색이 이미 TaechangDefine.h에 모여 있어 기각. 조사 중 폰트 선택 가드 불일치 2곳 발견해 DEBT_LOG 기록
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-01] refactor/sage-sidebar-tree
- **목적**: 사이드바 트리 커스텀드로우를 컨트롤로 승격 (3-A-6)
- **변경 내용**: CSageSidebarTree 신설. ON_NOTIFY_REFLECT로 트리가 자기 NM_CUSTOMDRAW를 처리하고 View의 OnSidebarTreeCustomDraw·메시지맵 등록·헤더 선언을 제거. **설정 메서드가 필요 없는 단일 스타일 컨트롤**로, CSageTabCtrl·CSageSectionLabel과 같은 형태다. 그룹 헤더 판정이 GetParentItem(hItem) == NULL로 트리 내부 정보만 쓰기 때문 — 워크플로 타입은 SetItemData에 실려 있으나 그리기 로직이 참조하지 않아 "컨트롤은 도메인 개념을 알면 안 된다" 기준을 원래부터 지키고 있었다. TVN_SELCHANGED는 워크플로 전환 로직이라 View에 잔류(리플렉션은 NM_CUSTOMDRAW만 가져감). 이동한 그리기 로직은 함수 시그니처와 m_wndSidebarTree. 접두사를 빼면 원본과 완전히 동일함을 diff로 확인. View.cpp 4,124→4,088줄
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-01] refactor/sage-listctrl
- **목적**: 리스트 커스텀드로우를 컨트롤로 승격하고, 작업 중 발견한 리페인트 병목을 해소 (3-A-5)
- **변경 내용**: CSageListCtrl 신설. OnListCustomDraw의 리스트 ID 분기를 렌더링 속성 3종(SetAlternateRowColor / SetCenterFirstColumn / SetHighlightColumns)으로 분해하고 ON_NOTIFY_REFLECT로 컨트롤이 자기 커스텀드로우를 처리하게 함. 컨트롤은 워크플로 타입을 모르고, IsReceivablesResultTable() 판단은 View에 남긴 뒤 ApplyResultColumns에서 금액 컬럼 범위만 전달. 계산 내역 리스트는 메시지맵 미등록으로 원래 커스텀드로우가 걸리지 않았어서 OFF/OFF로 현행 재현(DEBT_LOG 기록). View의 OnListCustomDraw·메시지맵 3개·헤더 선언 제거. **추가로 O(N^2) 리페인트 병목을 발견**: 행 삽입마다 리페인트가 걸려 그때까지의 전체 행에 커스텀드로우가 다시 도는 구조였고(미수금 10컬럼 500행이면 약 125만 회), 리스트 4곳에 SetRedraw 억제를 넣어 O(N)으로 정리. 조기 return이 모두 억제 시작 이전에 있음을 확인
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-07-31] refactor/sage-section-label
- **목적**: 섹션 라벨을 컨트롤로 승격해 View의 OnDrawItem을 완전히 제거 (3-A-3)
- **변경 내용**: CSageSectionLabel 신설(카멜 액센트 바 3px + 텍스트, variant 없는 단일 스타일). View의 SS_OWNERDRAW 라벨 7개를 CStatic→CSageSectionLabel로 교체. 이후 OnDrawItem 함수·DrawSectionLabel·메시지맵 ON_WM_DRAWITEM()·헤더 선언 2개를 제거해 **View의 OnDrawItem이 128줄에서 0줄이 됨**(함수 자체 소멸). 제거 전에 View의 오너드로우 컨트롤 34개(버튼 26 + 섹션 라벨 7 + 필터 콤보 1)가 전부 CSage*로 승격됐음을 확인. 7개 라벨 모두 SetFont와 그리기 폰트가 일치해 3-A-1의 R2 같은 함정은 없었음. View.cpp 4,204→4,172줄
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-07-31] refactor/sage-button-dialogs
- **목적**: 다이얼로그의 버튼 그리기 복제를 제거하고 CSageButton으로 통일 (3-A-2)
- **변경 내용**: 다이얼로그 파일 7개(클래스 8개 — TaechangPriceSimpleDlg에 CompanyRenameDlg/CoverPriceDlg 2개)의 m_wndOkBtn/m_wndCancelBtn을 CSageButton으로 교체하고 확인 버튼에 SetVariant(PRIMARY) 호출. OnDrawItem 함수 8개, 메시지맵 ON_WM_DRAWITEM() 8개, 헤더 선언 8개, static DrawSimpleButton 1개 제거(총 325줄 삭제). 버튼 외 오너드로우 컨트롤이 없어 핸들러를 통째로 제거해도 안전했고, 체크박스는 BS_AUTOCHECKBOX라 영향 없음. 커밋 2개로 나눠 교체 후 화면 무변화, 제거 후 컨트롤 그리기를 각각 확인. 작업 중 발견한 CoverPriceDlg 미호출·파일명 불일치는 DEBT_LOG에 기록
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-07-31] refactor/sage-button
- **목적**: 버튼 스타일을 부모 OnDrawItem의 ID 조건식으로 가르던 구조를 컨트롤 자기 그리기로 전환 (3-A-1)
- **변경 내용**: CSageButton 신설(Primary/Secondary variant, 검색·계산·리셋 아이콘 3종). View의 오너드로우 버튼 26개를 CButton→CSageButton으로 교체하고 Primary 12개에 SetVariant, 아이콘 4곳에 SetIcon 호출. OnDrawItem의 ODT_BUTTON 블록 117줄을 제거하고 미처리 owner-draw를 CView::OnDrawItem으로 위임해 MFC 리플렉션이 동작하게 함. bPrimary ID 12개 OR 조건식과 아이콘 ID 분기 소멸. 아이콘 색이 전부 그 시점 텍스트 색과 같은 값이어서 GetTextColor()로 통합. m_wndResultResetBtn의 SetFont가 실제 그리던 폰트(m_fontHeader)와 달랐던 것도 함께 해소. OnDrawItem 128→11줄, View.cpp 4,305→4,204줄. 커밋 3개로 나눠 각 단계마다 화면 동일성 확인
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-07-31] refactor/view-control-extract
- **목적**: View에 정의돼 있던 MFC 컨트롤 서브클래스를 app/ui/drawing으로 분리하고, 신규 코드 접두사를 Sage로 전환
- **변경 내용**: CTaechangHeaderCtrl / CTaechangTabCtrl / CTaechangComboBox / CTaechangFilterComboBox 4개를 클래스당 파일로 app/ui/drawing에 분리(SageTaechangView.h의 클래스 정의와 .cpp 구현 193줄 제거). 이후 파일명·클래스명을 CSage*로 전환. vcxproj 78→86 항목, filters에 app\ui\drawing 필터 생성. View.cpp 4,499→4,305줄, View.h 428→397줄. 부수적으로 CSageTabCtrl은 메시지맵과 OnPaint 구현이 100줄 떨어져 있던 것이 한 파일로 모임. 리팩토링 계획을 docs/decisions/REFACTORING_PLAN.md로 이관하고 작성 절차는 sagetaechang-plan skill로 분리
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-07-31] refactor/layer-cleanup
- **목적**: Step 1 재배치 이후 남은 계층 정리 — 앱 헤더의 DB 계층 전역 노출 제거, SqlInitializer 클래스명 정합
- **변경 내용**: SageTaechang.h(CWinApp 헤더)에서 SageDBMgr.h include를 제거하고 실제 사용처인 SageTaechang.cpp에 직접 추가. 이 과정에서 SageTaechangView.h가 SageTaechang.h → SageDBMgr.h → TaechangPriceRepository.h 사슬로 TaechangPriceDto를 간접 획득하던 것이 빌드 오류로 드러나, 전방 선언 + 직접 include로 정리(별도 fix 커밋). SQLInitializer 클래스명을 SqlInitializer로 변경(13곳). DEBT_LOG 2건 해소. 별건으로 coding-rules의 "함수 반환 타입에 포인터 사용 금지"를 제거하고 계약 규약(기본 비소유 / Get*·Find* 네이밍 / 소유권 이전 동사)으로 전환해, SageDBMgr Getter 부채 1건은 코드 변경 없이 철회
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-07-31] refactor/layer-relocation
- **목적**: 소스 구조를 core/infra/ui 3계층으로 재배치해 신규 파일 배치 기준을 확립. 기존에는 루트 평면 배치와 app/ 4계층이 공존해 새 파일을 어디 둘지 판단이 서지 않았고, 그 결과 기능이 SageTaechangView(4,497줄)로 몰렸다
- **변경 내용**: 루트 파일 20여 개와 기존 app/application·infrastructure·presentation을 app/core(price·receivable·auth·workflow) / app/infra(db·office·file) / app/ui(frame·view·dialogs)로 이동. include를 프로젝트 루트 기준 전체 경로로 통일. vcxproj 78항목 경로 갱신, filters를 "소스 파일\app\..." 37개 필터로 전면 재작성. SQLContext/SQLInitializer 파일명을 Sql 표기로 정합(클래스명은 유지). 실체가 없던 bridge 폴더 제거하고 TaechangBridgeResponse를 core/workflow/TaechangWorkflowResponse로 이동. **로직 변경 없음** — 이동 커밋이 0 insertions/0 deletions. 남은 의존 위반 6건은 docs/DEBT_LOG.md에 기록
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-06-23] fix/enable-clipboard-shortcuts
- **목적**: 입력창에서 막혀 있던 Ctrl+C/V/X/Z 클립보드 단축키 복구
- **변경 내용**: IDR_MAINFRAME 액셀러레이터 테이블에서 Ctrl+C/V/X/Z(ID_EDIT_COPY/PASTE/CUT/UNDO) 4줄 제거 → Edit 컨트롤 기본 처리로 전달. 원인은 마법사 기본 테이블이 키를 가로채 핸들러 없는 명령으로 버린 것
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-06-23] feature/result-filter-criteria-select
- **목적**: 미수금/납품서/견적서 결과 검색에 기준 셀렉박스 추가
- **변경 내용**: 검색 기준 콤보(미수금: 법인명/담당자/품목명, 납품서·견적서: 품목명/법인명) 추가, 기준별 필드 매칭, 워크플로우별 상태 저장, 검색 영역 흰 띠 배경, 전용 오너드로우 콤보(CTaechangFilterComboBox)로 글자 상하좌우 가운데 정렬
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-06-23] fix/receivables-form-etc-grouping
- **목적**: 미수금 엑셀 폼 생성기에 기타 법인 그룹핑 적용 + 기타 섹션 내부 구분선 제거
- **변경 내용**: generate-receivables-form.ps1에 #91과 동일한 그룹핑 수정(정렬키 companySortName 복원, 미매칭 sortCompanyName 보존, 기타 priority 계산) 적용 + Build-OutputRows에서 기타-기타 사이 '-' 구분선 생략(매칭/경계는 유지)
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-06-23] fix/receivables-etc-company-grouping
- **목적**: 미수금 내역서에서 같은 법인(미등록 법인)이 흩어져 정렬되는 문제 수정
- **변경 내용**: 미매칭 법인의 정렬용 법인명 실제값 보존, 정렬키에 companySortName을 담당자보다 앞에 복원, 미매칭 priority를 기타 위치로 계산(미등록 시 맨 뒤)
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/91
- **결과**: pending

## [2026-05-26] fix/co-crud-buttons-center
- **목적**: 데이터 관리 탭 법인목록 UI 수정 4건 (버튼 정렬, 검색 행 겹침, 헤더 빈 열 시각 처리, phantom 3열 제거)
- **변경 내용**: CRUD 버튼 가운데 정렬 / 검색 행 겹침 수정 / 헤더 OnPaint 역순 채색으로 blank 영역 흰색 처리 / Win32 직접 API + ShowWindow 후 재정리로 phantom 열 완전 제거
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/89
- **결과**: pending

## [2026-05-18] fix/receivables-login-gate-and-initial-screen
- **목적**: 미수금 내역서 로그인 게이트 추가 및 앱 최초 화면을 납품서 생성으로 변경
- **변경 내용**: 미수금 내역서 클릭 시 로그인 필요, 앱 시작 시 납품서 생성 화면으로 초기화
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/84
- **결과**: merged

## [2026-05-16] feature/receivables-company-order-data-tab
- **목적**: 미수금 내역서에서 법인 출력 순서 데이터를 직접 관리할 수 있는 데이터 관리 탭 추가
- **변경 내용**: 데이터 관리 탭 UI(CRUD 카드 + 법인 목록 2층 구조), 법인 순서 추가/수정/삭제, 법인명 검색, 선택 시 필드 자동 로드, 수정 모드 전환
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/81
- **결과**: merged

## [2026-05-16] feature/calc-icon-buttons
- **목적**: 견적서 생성 버튼 아이콘화 및 초기화 버튼 추가로 단가 계산 탭 UI 개선
- **변경 내용**: 견적서 생성 버튼을 문서 아이콘(30×38)으로 교체, 부수·페이지·운임 초기화 버튼(원형 화살표 아이콘) 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/80
- **결과**: merged

## [2026-05-15] feature/calc-tab-estimate-generate
- **목적**: 단가 계산 탭에서 계산 결과를 바탕으로 견적서를 바로 생성
- **변경 내용**: 견적서 생성 미니 다이얼로그 추가, 단일 견적서 생성 PowerShell 스크립트 추가, 계산 이력 품목명 열 추가, 계산 버튼 → 견적서 생성 버튼 변경
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/79
- **결과**: merged

## [2026-05-17] fix/receivables-order-title-layout
- **목적**: 미수금 내역서 데이터 관리 탭의 법인 출력 순서 관리 영역 가독성 개선
- **변경 내용**: 법인 출력 순서 관리 제목을 카드 밖으로 이동, 제목 배경색을 탭 배경과 일치, 카드 높이 축소, 법인 목록 검색 컨트롤 위치 조정
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/82
- **결과**: merged

## [2026-05-15] fix/price-calc-auto-refresh
- **목적**: 단가 계산 탭의 입력 초기화와 계산 결과 자동 갱신 개선
- **변경 내용**: 법인명 변경 시 입력/결과 초기화, 부수·페이지 입력 시 금액 자동 계산, 법인명 콤보 Tab 포커스 이동 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/78
- **결과**: merged

## [2026-05-15] fix/amount-comma-format
- **목적**: 주요 금액 표시와 금액 입력칸에 천 단위 콤마 적용
- **변경 내용**: 미수금 내역서 합계/입금/미수금, 견적서 보고서/표지/운임, 단가 데이터 관리 부수 단가/표지 단가, 단가 계산 운임 입력 포맷 보정
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/77
- **결과**: merged

## [2026-05-15] feature/result-filter-enter-search
- **목적**: 문서 결과 검색과 주요 입력 폼의 키보드 이동 흐름 개선
- **변경 내용**: 미수금/납품서/견적서 검색 edit Enter 처리, 로그인/비밀번호 변경/단가 관리/단가 계산 입력칸 Tab 포커스 이동 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/70
- **결과**: merged

## [2026-05-12] fix/auth-header-ui
- **목적**: 헤더 우측 로그인 사용자 표시(텍스트+로그아웃 버튼) UI 통일성 개선
- **변경 내용**: 사용자 라벨 색상 SECONDARY_TEXT 적용, Y축 버튼 텍스트 오프셋 맞춤, SS_RIGHT→SS_NOPREFIX 교체, 구분선 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/64
- **결과**: merged

## [2026-05-11] feature/price-detail-inline-form
- **목적**: 단가 데이터 관리 우측 상세 패널 UI 전반 개선
- **변경 내용**: 인라인 폼 레이아웃 전환, 헤더/구분선 추가, 레이블 배경 정리, 체크박스 위치·폭·배경 수정, 초기 자동 선택 제거, 선택 전·후 패널 헤더 일관화
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/61
- **결과**: pending

## [2026-05-11] codex/fix-login-edit-align
- **목적**: 로그인 다이얼로그 ID/PW 입력 박스의 텍스트가 위로 치우쳐 보이는 문제 수정
- **변경 내용**: 로그인 edit 컨트롤에 내부 텍스트 영역 보정 적용, Enter 키 로그인 동작 유지
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/62
- **결과**: merged

## [2026-05-11] codex/estimate-one-page-mode
- **목적**: 견적서 생성에서 선택한 여러 행을 하나의 견적서 파일에 작성하는 한 페이지 작성 모드 추가
- **변경 내용**: 견적서 입력 화면에 한 페이지 작성 체크박스 추가, 선택 행 6개 제한, 생성 payload 플래그 전달, PowerShell 견적서 스크립트 한 파일 작성 분기 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/58
- **결과**: merged

## [2026-05-14] fix/keep-price-dialog-on-validation
- **목적**: 단가 추가 검증 실패 시 다이얼로그 유지 및 부수 계산 라벨 문구 보정
- **변경 내용**: 기존 부수 범위 중복 검증을 단가 추가 다이얼로그 내부로 이동, 검증 실패 시 경고 후 입력 창 유지, 부수 계산/최근 계산 내역 라벨을 내용 금액·표지 금액으로 변경
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/66
- **결과**: merged

## [2026-05-14] fix/preserve-generated-input-table
- **목적**: 납품서/견적서 생성 후 탭 전환 시 입력 표 유지 및 단가 데이터 관리 화면의 문서 필터 컨트롤 겹침 제거
- **변경 내용**: 생성 완료 응답이 로드 결과 JSON을 덮어쓰지 않도록 보정, 가격 관리 계열 화면 진입 시 결과 필터 에디트/검색/초기화 버튼 숨김 처리
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/65
- **결과**: merged

## [2026-05-07] fix/price-login-guard
- **목적**: 단가 데이터 관리, 단가 계산 사이드바 메뉴를 로그인 미인증 시 접근 차단
- **변경 내용**: `OnSidebarSelChanged`에 `TAECHANG_WORKFLOW_PRICE_MANAGE` / `TAECHANG_WORKFLOW_PRICE_CALC` 로그인 체크 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/52
- **결과**: merged

## [2026-05-07] develop → main
- **목적**: 비밀번호 변경 기능, 단가 관리 UI 개선, 문서 생성 및 미수금 결과 UI 개선을 main에 반영
- **변경 내용**: 비밀번호 변경 다이얼로그/메뉴 추가, 비밀번호 정책 제한, 단가 관리 신규 다이얼로그(TaechangPriceRangeDlg/TaechangPriceSimpleDlg), 법인 추가 다이얼로그, 부수 계산 패널 이력 UI, 문서 생성 스크립트 개선
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/51
- **결과**: merged

## [2026-05-06] feature/price-manage-summary-card
- **목적**: 단가 관리 UI의 탭 기반 구조를 제거하고 우측 패널 편집 폼 구조로 전환하여 직관성 개선
- **변경 내용**: 조회/추가/수정/삭제 탭 제거, 우측 패널 상태 전환(요약↔편집폼), [+ 단가추가] 버튼 법인명 행 배치, 하단 좌측 버튼 완전 제거
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/45
- **결과**: merged

## [2026-05-06] fix/workflow-file-drop
- **목적**: 워크플로우 입력 화면에서 파일 드래그앤드롭 시 입력 경로가 반영되지 않는 문제 수정
- **변경 내용**: 드롭 파일 경로 수집 로직 정리, 입력 필드/파일 선택 버튼/결과 영역/프레임에 파일 드롭 허용, Debug 실행 환경의 드롭 메시지 필터 허용
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/43
- **결과**: merged

## [2026-05-05] feature/price-input-validation — feat: 가격 관리 입력값 범위 검증 추가
- **목적**: 가격 데이터 관리·부수 계산 화면의 입력값에 범위/길이 제한을 추가하여 잘못된 데이터 입력 방지
- **변경 내용**: 법인명 한글 20자/영문 40자, 부수 1~9,999,999, 인쇄·표지 가격 0~10,000,000, 운임 0~10,000,000 클램핑
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/40
- **결과**: merged

## [2026-05-05] feature/price-manage-ui — feat: 감사보고서 가격 관리 및 부수 계산 UI 초안
- **목적**: 법인별 감사보고서 가격 데이터 관리 및 부수 입력 시 가격 계산 기능 UI 초안 구현
- **변경 내용**: Repository에 DeleteByPriceId/SelectAllCompanyNames/UpdateByPriceId 추가, Service에 RemovePrice/LoadAllCompanyNames/ModifyPriceById 추가, 사이드바 "가격 관리" 카테고리(가격 데이터 관리/부수 계산) 추가, 두 전용 패널 생성·레이아웃·이벤트 구현
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/39
- **결과**: merged

## [2026-05-05] feature/user-auth-login — feat: 사용자 인증 및 로그인 시스템 구현
- **목적**: 관리자/사용자 2-Role 로그인 시스템 구현 — 이후 관리자 전용 기능 권한 분리 기반
- **변경 내용**: TaechangUser DB 테이블 + 기본 admin 계정 시딩, DTO/Repository/Service/AuthSession 계층 구현(BCrypt SHA-256), 메인 화면 우측 상단 로그인 버튼 및 로그인 다이얼로그 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/38
- **결과**: merged

## [2026-05-05] feat/delivery-middle-time-mapping — feat: 납품서 배송시간 '오전중'/'오후중' 처리 추가
- **목적**: 배송시간 입력값이 '오전 중'/'오후 중'일 때 결과 셀에 '중'이 기입되지 않던 문제 수정
- **변경 내용**: `generate-delivery-forms.ps1`에 `$MiddleSuffix` 변수 추가, '오전중' → H6에 '중', '오후중' → M6에 '중' 처리 `elseif` 블록 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/37
- **결과**: merged

## [2026-05-05] feature/sidebar-title-and-design — feat: 사이드바 브랜드 타이틀 및 디자인 개선
- **목적**: 사이드바 상단 영문 타이틀을 한글로 변경하고 전문 ERP 사이드바 디자인 완성도 향상
- **변경 내용**: 타이틀 "SageTaechang" → "태창기획" (중앙 정렬), 우측 상단 대기 중 상태 표시 제거, 브랜드 영역 구분선 추가, 그룹 헤더 뮤트 색상 처리, 선택 항목 좌측 3px 카멜 액센트 바 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/36
- **결과**: merged

## [2026-05-05] develop — feat: 납품서/견적서 생성 화면 UI 개선
- **목적**: 전반적인 UI 완성도 향상 — 학교 과제물 느낌 탈피, 컬러 통일성 확보
- **변경 내용**: 타이틀 브라운 accent 컬러, 상태 배지 배경색, 파일/폴더 선택 버튼 왼쪽 이동, 섹션 레이블 accent bar, 전체 선택 버튼 테이블 헤더 라인으로 이동, 테이블 헤더 다크 브라운 + 흰색 텍스트(CTaechangHeaderCtrl 서브클래스)
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/35
- **결과**: merged

## [2026-05-05] feature/select-all-button — feat: 납품서/견적서 생성 화면에 전체 선택 버튼 추가
- **목적**: 납품서/견적서 생성 시 행을 일일이 수동으로 체크해야 했던 불편함 해소
- **변경 내용**: 데이터 로드 후 입력 탭에 "전체 선택" 버튼 추가 (토글 방식 — 전체 선택/전체 해제), 실행 중 비활성화, 다른 워크플로우 전환 시 자동 숨김
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/34
- **결과**: merged

## [2026-05-05] feature/list-grid-and-amount-highlight — feat: 결과 리스트 그리드 라인 및 미수금 금액 열 배경 강조 추가
- **목적**: 미수금 결과·납품서·견적서 행 선택 화면의 리스트 가독성 개선
- **변경 내용**: 3개 테이블에 LVS_EX_GRIDLINES 적용, 미수금 합계·입금·미수금 열 NM_CUSTOMDRAW subitem 배경색 강조
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/32
- **결과**: merged

## [2026-05-05] fix/edit-border-style — fix: 에디트 박스 스타일 개선 및 UI 폰트/텍스트 정리
- **목적**: 에디트 박스 검은 테두리·텍스트 상단 쏠림 개선, 본문 폰트 상향, 결과 헤더 한글화
- **변경 내용**: 베이지 테두리(OnDraw 직접 그리기), 높이 28px, EM_SETRECT 7px 패딩, 사이드바 10pt·본문 11pt 분리, 상태→상태/사유 한글화
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/31
- **결과**: merged

## [2026-05-05] feat/step2-action-status — feat: 버튼 옆 완료/실패 상태 표시 (Step 2)
- **목적**: 작업 결과를 버튼 옆에 즉시 표시, 워크플로우 전환 시 입력 경로·저장 위치 초기화
- **변경 내용**: 진행바 자리 재활용 — 실행 중 진행바, 완료 후 초록 "완료" / 실패 후 빨간 "실패" 텍스트 전환
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/30
- **결과**: merged

## [2026-05-05] fix/step1-result-tab-button — fix: 결과/실행기록 탭 생성 버튼 제거 (Step 1)
- **목적**: 결과·실행기록 탭은 보는 탭이므로 생성 버튼 제거
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/29
- **결과**: merged

## [2026-05-05] fix/tab-custom-draw — fix: 탭/사이드바/버튼 UI 개선
- **목적**: 탭 색감 불일치, 사이드바 파란 선택색, 버튼 텍스트 위치 등 시각적 문제 수정
- **변경 내용**: CTaechangTabCtrl OnPaint 직접 드로잉, 사이드바 NM_CUSTOMDRAW 선택색, 버튼 텍스트 2px 하향, 상태 초기화 버그 수정
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/28
- **결과**: merged

## [2026-05-05] fix/ui-polish — fix: UI 개선 4종
- **목적**: Input/Output 한글화, 빈 화면 힌트, 진행바 조건 표시, 사이드바 스크롤 제거
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/27
- **결과**: merged

## [2026-05-04] feature/estimate-input-ui — feat: 견적서 생성 UI 개선
- **목적**: 견적서도 납품서처럼 파일 로드 후 입력 탭에 행 선택 테이블 표시, 체크박스 다중 선택 후 생성
- **변경 내용**: 입력 탭 테이블(행/법인명/날짜/품목명/부수/페이지/보고서/표지/운임 9열), 파일 선택·드롭 자동 로드, 운임 없을 때 F11·G11 빈칸, 파일명 법인명_견적서_yyyyMMdd_HHmmss 형식 변경, 결과 탭 체크박스 제거
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/26
- **결과**: merged

## [2026-05-04] feature/delivery-input-ui — feat: 납품서 생성 UI 개선
- **목적**: 납품서 입력 데이터를 입력 탭에서 바로 확인하고, 체크박스로 다중 행 선택 후 한 번에 생성
- **변경 내용**: 로드 완료 시 입력 탭에 테이블 표시, 생성 완료 시 결과 탭 전환, LVS_EX_CHECKBOXES 다중 선택, 행 번호 접두사 제거, 파일명 끝 행 번호 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/25
- **결과**: merged

## [2026-05-04] develop — refactor: 중복 코드 제거 및 코드 품질 개선
- **목적**: 전체 코드 리뷰 결과 발견된 Blocker/Major/Minor 항목 수정
- **변경 내용**: TaechangFileUtils/TaechangDialogHelper 신규 분리, FileExists 등 5개 파일 중복 제거, ExtractJsonArray 3개 파일 중복 제거, PostMessage 누수 Blocker 수정, 레이아웃 매직 넘버·스트링 상수화, 주석 제거 (17개 파일, 순 463줄 감소)
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/24
- **결과**: merged

## [2026-05-04] develop — feat: 파일 드래그 앤 드롭으로 입력 파일 지정 지원
- **목적**: 파일 선택 버튼 없이 탐색기에서 바로 파일을 끌어다 놓을 수 있게 개선
- **변경 내용**: WM_DROPFILES 핸들러 추가, 문서 생성 단일 파일/검수 다중 파일 드롭 지원, 드롭 시 Input 탭 자동 전환
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/23
- **결과**: merged

## [2026-05-04] develop — feat: 검수 기능 준비 중 비활성화 처리
- **목적**: PDF/HWP 검수 기능 완성 전까지 사이드바에서 비활성화 표시
- **변경 내용**: 검수 항목에 "(준비 중)" 텍스트 추가, 클릭 시 안내 메시지 후 이전 선택 복귀
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/22
- **결과**: merged

## [2026-05-04] develop — refactor: pdftotext.exe 자동 감지로 설정 메뉴 제거
- **목적**: 사용자가 직접 pdftotext.exe 경로를 설정해야 하는 불편함 제거
- **변경 내용**: 앱 실행 파일 옆 pdftotext.exe 자동 감지, 설정 사이드바 메뉴 및 OnSettings 제거, 에러 메시지 한국어화
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/21
- **결과**: merged

## [2026-05-04] develop — design: UI 시각적 완성도 개선
- **목적**: 기본 Windows 컨트롤 외관에서 벗어나 ERP다운 시각 구조 확보
- **변경 내용**: 사이드바 다크 컬러 적용, 버튼 Owner-draw(주요/보조 구분), 리스트뷰 교대 행 색상, 사이드바-콘텐츠 구분선 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/20
- **결과**: merged

## [2026-05-04] fix/receivables-preview
- **목적**: 미수금 내역서 생성 후 결과 확인 흐름 정리
- **변경 내용**: 미리보기 버튼/탭 제거, 생성 결과 탭을 저장 엑셀 헤더 구조로 표시, 생성 결과 JSON에 저장 행 기준 rows 포함, 실행 기록을 업무 로그 형태로 누적 표시
- **검증**: 사용자 파일로 미수금 내역서 생성 성공, PowerShell 스크립트 파싱 통과, Debug x64 빌드 성공, 금지 패턴 확인, git diff --check 통과
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/19
- **결과**: merged

## [2026-05-03] fix/receivables-array-output
- **목적**: 미수금 내역서 출력 중복 및 담당자 숫자 정렬 문제 수정
- **변경 내용**: Excel COM 다중 행 배열 대입을 행 단위 범위 대입으로 변경, 담당자 자연 정렬 키 추가, 저장 파일명을 미수금내역서_yyyymmdd_hhmmss 형식으로 변경
- **검증**: 미수금 사용자 파일 로드/생성 성공, 삼덕회계법인 10층 정렬 확인, 중복 출력 해결 확인, PowerShell 스크립트 파싱 통과, Debug x64 빌드 성공
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/18
- **결과**: merged

## [2026-05-03] feature/gmarket-fonts
- **목적**: SageNexus에서 사용하던 Gmarket Sans TTF 폰트를 SageTaechang MFC UI에 적용
- **변경 내용**: Gmarket Sans TTF 3종 리소스 추가, private font 로딩/해제 연결, 주요 MFC 컨트롤 폰트 지정
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/11
- **결과**: merged

## [2026-05-03] feature/erp-workflow-ui
- **목적**: SageTaechang MFC UI를 ERP형 업무 도구 구조로 재구성
- **변경 내용**: 좌측 사이드바(워크플로우 메뉴) + 헤더 + 탭 + 본문(입력/출력/결과/상세) 레이아웃 도입, UI 색/폰트/레이아웃 상수와 탭/섹션 라벨 상수 추가, Claude 작업 가이드/UI 진행 기록 문서 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/12
- **결과**: merged

## [2026-05-03] fix/task-tab-visibility
- **목적**: 탭 라벨과 실제 화면 가시성이 어긋나 있던 문제 정리
- **변경 내용**: 탭별 가시성 helper 6개 도입, UpdateTaskTabVisibility / LayoutChildControls / LayoutActionSection / LayoutResultSection을 새 정책에 맞춰 재작성. 결과 탭에서 입력 섹션 재노출, 상세/실행 기록 탭에서 결과 테이블 재노출 동작 제거. 설정 버튼은 PDF 검수 입력 탭에서만 노출.
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/13
- **결과**: merged

## [2026-05-03] fix/compare-result-file-column
- **목적**: 가이드의 검수 결과 컬럼(파일명/항목/값/상태/사유)을 적용
- **변경 내용**: TaechangResultRow에 m_strFile 추가, ApplyResultColumns로 워크플로우별 동적 컬럼 구성(검수 5컬럼/문서 생성 4컬럼), AddCompareFileRows 매핑 재정의(fileName→m_strFile, leftValue→m_strValue), 사유 prefix를 "기준="으로 변경, 관련 상수 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/14
- **결과**: merged

## [2026-05-03] fix/header-status-update
- **목적**: 헤더 우측 상태 영역이 정적 텍스트로 고정돼 있던 문제 해결
- **변경 내용**: 상태별 색 상수(SUCCESS/WARNING/ERROR) 추가, 미사용 WORKSPACE_STATUS 상수 제거, m_colorHeaderStatus 멤버와 ResolveStatusColor helper 도입, SetStatusText에서 헤더 텍스트/색 갱신, OnCtlColor에 헤더 상태 분기 추가
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/15
- **결과**: merged

## [2026-05-03] fix/receivables-generation-flow
- **목적**: 미수금 내역서 생성 흐름의 정렬, 진행률 표시, Excel 처리 병목 개선
- **변경 내용**: 미수금 법인 고정 순번 정렬 및 미등록 법인 기타 그룹 처리, 생성 중 진행률 0~100% 표시와 완료 상태 보존, 미수금/견적/거래명세 Excel 스크립트의 범위 읽기·배열 쓰기·템플릿 재사용 최적화, 거래명세 UsedRange 행 계산 보정
- **검증**: 미수금 사용자 파일 로드/생성 성공, 임시 견적/거래명세 로드/생성 성공, PowerShell 스크립트 파싱 통과, Debug x64 빌드 성공
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/16
- **결과**: merged

## [2026-05-03] refactor/sidebar-grouped-tree
- **목적**: 가이드의 사이드바 3그룹(문서 생성/검수/관리) 구조 적용 + 설정을 사이드바로 이전
- **변경 내용**: m_wndWorkflowMenu(CListBox) → m_wndSidebarTree(CTreeCtrl) 교체, 트리 노드를 세 그룹으로 구성(항상 펼침), 본문 m_wndSettings 버튼 제거 후 "관리 > 설정"으로 이전, OnSidebarSelectionChanged에서 ItemData 기반 라우팅(워크플로우 변경/설정 호출/그룹 헤더 무시), m_nCurrentWorkflow 멤버 도입, 미사용 ID/UI 상수 정리
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/17
- **결과**: merged
## [2026-05-10] fix/preserve-document-tab-data
- **목적**: 문서 생성 업무 탭 전환 시 미수금/납품서/견적서 표 데이터 유지 및 납품서/견적서 입력 파일 초기화 지원
- **변경 내용**: 업무별 UI 상태 저장/복원, 입력 표 체크·필터 상태 보존, 납품서/견적서 입력 초기화 버튼 추가, 로드 완료 직후 초기화 버튼 레이아웃 갱신 보정
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/57
- **결과**: merged

## [2026-05-14] fix/receivables-etc-company-name
- **목적**: 미수금 내역서에서 미등록 법인이 기타 정렬 그룹으로 처리될 때 법인명까지 기타로 저장되는 문제 수정
- **변경 내용**: 미수금 미리보기/생성 스크립트에서 정렬용 법인명과 표시/저장 법인명을 분리하고, 정렬 전용 필드를 결과에서 제거
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/67
- **결과**: merged

## [2026-05-14] feature/receivables-company-sort-order-crud
- **목적**: 미수금 내역서 법인명 정렬 기준을 사용자가 직접 관리할 수 있도록 DB CRUD 기반 추가
- **변경 내용**: 법인명/정렬 순서 테이블 생성, DTO/Repository/Service CRUD 추가, SageDBMgr 접근자와 생성/삭제 흐름 연결
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/68
- **결과**: merged

## [2026-05-14] feature/use-db-receivables-company-order
- **목적**: 미수금 내역서 로드/생성 정렬 기준을 DB에 관리되는 법인 정렬 순서로 적용
- **변경 내용**: DB 정렬 기준을 임시 JSON으로 PowerShell에 전달하고, PriorityPath가 있으면 엑셀 번호 시트보다 DB 기준을 우선 사용하도록 변경
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/69
- **결과**: merged

## [2026-05-15] fix/receivables-payment-note-columns
- **목적**: 미수금 내역서 로드/생성 시 원본 H/I/K 열 값이 결과에 반영되지 않는 문제 수정
- **변경 내용**: 입금금액(H), 미수금(I), 비고(K)를 미리보기 JSON과 생성 출력 행의 동일 열 위치에 전달하도록 변경
- **검증**: PowerShell Parser 파싱 통과, git diff --check 통과
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/72
- **결과**: merged

## [2026-05-15] fix/receivables-result-tab-columns
- **목적**: 미수금 내역서 생성 결과 탭에서 숫자 JSON 값이 표시되지 않는 문제 수정
- **변경 내용**: 결과 행 파서가 합계금액, 입금금액(H), 미수금(I)을 문자열 또는 숫자 JSON 값 모두에서 텍스트로 추출하도록 보정
- **검증**: git diff --check 통과, Debug x64 빌드 성공
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/74
- **결과**: merged

## [2026-05-17] fix/receivables-order-duplicate-validation
- **목적**: 미수금 내역서 데이터 관리에서 법인명 또는 출력 순서가 중복 저장되는 문제 방지
- **변경 내용**: 법인 출력 순서 저장 전 법인명/출력 순서 중복 검사 추가, 중복 오류 메시지 상수 정리
- **검증**: git diff --check 통과, Debug x64 빌드 성공
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/83
- **결과**: merged

## [2026-08-04] refactor/workflow-handler
- **목적**: Step 4-1. 워크플로 타입 분기를 View 밖으로 옮기기 위한 확장점(핸들러 인터페이스와 등록부) 신설
- **변경 내용**: `ISageWorkflowHandler`, `SageWorkflowRegistry`(`FindHandler`는 NULL 가능), 핸들러 5개 골격, vcxproj/filters 등록
- **검증**: Debug x64 빌드 성공. View 미사용이라 화면 변화 없음
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/92
- **결과**: merged

## [2026-08-04] refactor/workflow-handler-labels
- **목적**: Step 4-2. `UpdateWorkflowLabels`의 워크플로 분기 5갈래 제거
- **변경 내용**: 핸들러가 헤더 제목/입력 섹션/실행 버튼/상세 섹션 라벨을 답하도록 이관. 착수 전 5x4 매핑 표로 상수까지 대조
- **검증**: Debug x64 빌드 성공, 워크플로 5개 + 단가 2개 순회로 표시 무변화 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/93
- **결과**: merged

## [2026-08-04] refactor/workflow-handler-tabs
- **목적**: Step 4-3. 워크플로 탭 구성을 핸들러로 이관하고 인덱스 변환 로직 단순화
- **변경 내용**: `SageWorkflowTab`(semantic 인덱스 + 라벨) 신설, `ApplyWorkflowTabs`와 변환 함수 2개를 배열 조회로 대체, `HasDocumentResultTab` 제거. 계획서 4-3 범위 정정과 DEBT_LOG 2건 등록 포함
- **검증**: Debug x64 빌드 성공, 워크플로 5개 탭 목록·전환·워크플로 전환 후 탭 유지 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `86cc75d`)
- **결과**: merged

## [2026-08-04] refactor/workflow-handler-columns
- **목적**: Step 4-4. 결과 표 컬럼 정의와 표시 속성을 핸들러로 이관
- **변경 내용**: `SageWorkflowResultTable`(컬럼 값 객체 + 표시 속성 + 공용 기본 표) 신설, `ApplyResultColumns`/`UpdateResultColumns`가 같은 표를 순회하도록 변경. View 순 -83줄
- **검증**: Debug x64 빌드 성공, 미수금·납품서·견적서 표 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `54e51bd`)
- **결과**: merged

## [2026-08-04] style/result-column-fill
- **목적**: 미수금·납품서·견적서 표가 창 폭을 채우지 못하고 오른쪽에 여백이 남는 문제 해소
- **변경 내용**: 가변 컬럼이 없는 표는 정의 폭 비율대로 확대하고 마지막 컬럼이 나머지를 흡수. 창이 정의 폭보다 좁으면 기존 동작 유지
- **검증**: Debug x64 빌드 성공, 창 크기 조절 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `5a456bc`)
- **결과**: merged

## [2026-08-04] refactor/remove-compare-workflows
- **목적**: 실제로 쓰지 않는 PDF·HWP 표지 검수 워크플로 제거. Step 4의 남은 단계에서 죽은 기능을 핸들러로 옮기는 낭비를 막기 위해 4-4 이후에 삽입
- **변경 내용**: View의 검수 경로(CSV 내보내기·다중 파일 선택·검수 실행)와 IsCompareWorkflow 15곳, 핸들러 2쌍, infra 서비스 3개, presenter의 비교 행 생성, 검수 전용 상수 31개 제거. 20파일 -1,295줄
- **검증**: Debug x64 빌드 성공. 사이드바에 진입점이 없던 기능이라 화면 변화 없음. 문서 3종·단가 2종 동작 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `b2e8169`)
- **결과**: merged

## [2026-08-04] refactor/workflow-handler-input
- **목적**: Step 4-5. 입력 축(파일 선택 제목, 선택·드롭 후 자동 불러오기, 입력 초기화)을 핸들러로 이관
- **변경 내용**: `GetInputDialogTitle`과 `UsesInputTable` 추가. 세 함수에서 워크플로 상수 분기 제거. 검수 제거 덕에 다중 파일 선택 분기는 이미 없어진 상태였다
- **검증**: Debug x64 빌드 성공. 미수금(자동 불러오기 없음)·납품·견적(즉시 불러오기)·드롭·입력 초기화·단가 2종 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `e6b616b`)
- **결과**: merged

## [2026-08-04] refactor/view-current-handler
- **목적**: View에 9곳으로 늘어난 핸들러 조회 중복 제거 (DEBT_LOG 후속 조건 만기)
- **변경 내용**: 사설 헬퍼 `FindCurrentHandler()` 도입, 9곳 치환. View.h는 전방 선언만 추가. NULL 검사는 호출부 유지
- **검증**: Debug x64 빌드 성공. 문서 3종 라벨·탭·결과 표, 입력 선택·초기화, 단가 2종(핸들러 없는 경로) 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `e8a0c56`)
- **결과**: merged

## [2026-08-04] refactor/workflow-handler-validation
- **목적**: Step 4-6a. 생성 전 선택 행 검증을 핸들러로 이관
- **변경 내용**: `ValidateSelectedRows(nSelectedCount, bHasSelectedRowNums, bOnePage, strError)` 추가. 진입 조건은 `UsesInputTable()` 재사용. 실패 메시지 2종과 견적 6행 제한이 핸들러로 이동
- **검증**: Debug x64 빌드 성공. 납품·견적 미선택 메시지, 견적 한 페이지 6행 초과/이하, 납품 무제한, 미수금 생성 확인
- **PR 링크**: 없음 (사용자 요청으로 develop 직접 머지, `07f043b`)
- **결과**: merged

## [2026-08-05] docs/plan-responsibility-criteria
- **목적**: 완료 기준이 줄 수 지표로 쓰여 있어 짐을 패널로 옮기기만 해도 통과할 수 있었다. 목표를 책임 과중 해소로 되돌린다
- **변경 내용**: 도달 기준을 책임 기준 A~E로 재작성, 줄 수·멤버 수·메시지맵 항목은 증상 지표로 강등. 3-B 공통 절차에 패널 생성 직후 기준 A 점검 추가. 3-B-4에 SageWorkflowPanel 분할 재판단 블록 추가. 스킬 2개(coding-design·sagetaechang-ui)도 같은 방향으로 수정(gitignore 대상)
- **검증**: 문서 변경. BOM + CRLF 유지 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/94
- **결과**: merged

## [2026-08-05] docs/plan-substep-split
- **목적**: 스킬(완료 기준 → 책임 기준) 수정 후 sagetaechang-plan 규칙에 따른 계획 전체 재점검
- **변경 내용**: 새 기준이 만든 모순 2건 해소 — 3-B-5를 5a(사이드바)/5b(헤더)로, 3-B-6을 6a(컨트롤러+워커 수신처)/6b(Runner+infra 역전)로 분할. 순서 결정 근거를 부피에서 R5 좌표계 검증으로 교체. 3-B-1·3-B-2에 마주칠 부채와 현행 재현 지침 추가, R9 등록. DEBT_LOG 4건의 후속 Step 번호를 코드 확인 후 갱신
- **검증**: 문서 변경. 불필요해진 Step·순서 뒤집힘·네이밍 불일치는 없음을 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/95
- **결과**: merged

## [2026-08-05] refactor/price-calc-service
- **목적**: Step 3-B-1a. 기준 A 집계에서 단가 계산 화면의 변경 이유 3개 중 계산 규칙이 화면 소관이 아님이 드러나, 패널 이관 전에 core로 분리
- **변경 내용**: SagePriceCalcService(core) 신설 — 부수·페이지 범위 검증, 단가 조회, 인쇄비·소계·합계, 운임 클램프. 실패 종류는 enum, 문자열·아이콘 매핑은 UI 유지. ValidateCopies를 따로 노출해 기존 검증 순서 보존. View 금액 멤버 3개 → SagePriceCalcResult 1개, UpdateCalcPreview 86 → 67줄
- **검증**: Debug x64 빌드 성공. 실패 메시지 8종(아이콘 포함), 정상 계산 3항목 + 합계, 운임 변경, 견적 저장 후 내역 추가 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/96
- **결과**: merged

## [2026-08-05] refactor/price-calc-panel
- **목적**: Step 3-B-1b. 패널 분리 1호로 단가 계산 화면을 옮겨 패턴과 R5(좌표계 전환)를 검증
- **변경 내용**: SagePriceCalcPanel 신설(808줄) — 컨트롤 25개·메시지맵 8항목·전용 함수 18개·레이아웃·카드 그리기·Tab 이동 소유. View.cpp 3,798 → 3,079줄, View.h 389 → 318줄, 컨트롤 멤버 104 → 79, 메시지맵 50 → 42. 위임 스텁 0개. 금액 포맷 유틸 4개를 app/common/SageNumberFormat으로 승격. ShowWindow 35줄 → 1줄, 그리기 4벌 → 1곳, 워크플로 상수 참조 1곳 감소
- **검증**: Debug x64 빌드 성공. 카드/테두리 위치, 법인 선택, 즉시 미리보기, 운임 콤마+합계, Tab 이동, 견적 저장 후 내역 추가, 업무 전환 후 복귀, 단가 관리 법인 추가 시 콤보 반영 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/97
- **결과**: merged

## [2026-08-05] refactor/price-manage-panel
- **목적**: Step 3-B-2. 단가 데이터 관리 화면을 패널로 이관. R6(notification 라우팅) 첫 실제 검증
- **변경 내용**: SagePriceManagePanel 신설(960줄) — 컨트롤 26개·메시지맵 14항목·전용 함수 25개·체크박스 OnCtlColor 분기 소유. 표시 전환은 WM_SHOWWINDOW로 받아 요약/편집 적용과 상태 초기화를 패널이 한다. 법인 변경 후 계산 콤보 즉시 갱신 호출 2곳 제거(패널 간 참조 없음). View.cpp 3,079 → 2,233줄, 컨트롤 멤버 79 → 53, 메시지맵 42 → 28
- **회귀 수정**: 카드 2개(검색 기준·법인 발주)가 OnDraw 전용이라 ShowPrice*Panel의 Invalidate(TRUE) 부수 효과에 의존하고 있었다. SetCardRect로 모아 이전 ∪ 새 영역만 무효화 (전체 무효화 시 레이아웃마다 깜빡임)
- **검증**: Debug x64 빌드 성공. 목록·카드·컬럼 폭, 콤보 자동 매칭, 행 선택 시 편집 폼, 체크박스 배경·경고, CRUD 4종, Tab 이동, 법인 변경 후 계산 콤보 반영, 창 크기 변경, 잔상 해소 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/98
- **결과**: merged

## [2026-08-05] refactor/workflow-handler-filter
- **목적**: Step 4-6b(3-B-3의 첫 단계). 문서 워크플로를 패널로 옮기기 전에 결과 필터의 워크플로 분기를 핸들러로 이관
- **변경 내용**: ISageWorkflowHandler에 UsesCustomResultTable · GetFilterCriteriaCount · GetFilterCriteria 추가. SageWorkflowFilterCriteria 순서 배열이 콤보 순서이고 첫 항목이 기본값(미수금 3개 / 납품·견적 2개). View 분기 4곳 제거, 핸들러 내부의 파일 지역 술어 Has*Table을 UsesCustomResultTable로 합침. 워크플로 상수 참조 34 → 30곳
- **회귀 수정**: DrawEditBorder가 컨트롤 바깥 1px에 그리는 테두리 링이 컨트롤을 숨겨도 남아 미수금 결과 화면에 입력 컨트롤 잔상이 생겼다. LayoutChildControls 호출부 8곳 중 배경을 지우던 곳이 하나뿐이었다. InvalidateContentArea로 사이드바 제외 작업 영역만 무효화
- **검증**: Debug x64 빌드 성공. 콤보 항목·기본값·기준별 필터·전환 후 복원·카드 표시 조건, 잔상 해소(결과 표시·생성 완료·입력 초기화·드롭·탭 왕복·창 크기 변경) 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/99
- **결과**: merged

## [2026-08-05] refactor/workflow-state-target
- **목적**: Step 4-6c. "문서 워크플로인가" 판정을 상수 나열에서 핸들러 등록 여부로 바꿔 워크플로 추가 시 고칠 목록을 줄인다
- **변경 내용**: IsDocumentWorkflowStateTarget 제거. SaveWorkflowUiState·RestoreWorkflowUiState가 FindHandler != NULL로 판정. RebuildCurrentWorkflowResultList의 세 번째 사용처는 IsDocumentResultFilterVisible이 이미 핸들러 존재를 보장하므로 검사 자체를 삭제. 워크플로 상수 참조 30 → 27곳
- **검증**: Debug x64 빌드 성공. 미수금 불러오기·체크 후 워크플로 왕복 시 탭·결과·체크 복원, 단가 화면 왕복 후 상태 유지, 견적 한 페이지 체크 유지 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/100
- **결과**: merged

## [2026-08-05] refactor/workflow-handler-response
- **목적**: Step 4-7(3-B-3 마지막). 응답 표시·결과 행 삽입의 워크플로 분기 제거와 결과 표 판정 이중화 부채 해소
- **변경 내용**: (A) SageWorkflowColumn에 데이터 출처 nField를 넣어 InsertResultRow의 3분기 43줄을 루프로. 헤더와 데이터가 같은 배열에서 나오므로 어긋남이 구조적으로 불가능. (B) DisplayResponse의 결과 표 유지·필터 경유·탭 전환·완료 메시지를 UsesInputTable·핸들러 존재·FindGenerateCompletedMessage로 대체. (C) 술어 2개를 IsInputTableVisible·IsOnePageOptionVisible로 교체하고 핸들러에 UsesOnePageOption 추가. 워크플로 상수 참조 34 → 16곳, View.cpp 2,199줄
- **포함 수정**: WS_CLIPCHILDREN 추가(부모 배경 지우기가 자식을 덮어 클릭할 때까지 안 보이던 문제), 결과 탭의 배치되지 않은 저장 위치 컨트롤 숨김(제 작업 이전부터 있던 결함)
- **검증**: Debug x64 빌드 성공. 표 컬럼 4종 값 위치, 납품·견적 불러오기/생성 후 탭·메시지·입력 표 유지, 미수금 결과 탭 전환, 전체선택·한 페이지 조건과 7행 경고, 워크플로 왕복 상태 복원, 잔상 해소 확인
- **PR 링크**: https://github.com/JakeKim4926/SageTaechang/pull/101
- **결과**: merged

## [2026-08-06] fix/design-tokens
- **목적**: 색 토큰 · 서체 · 버튼 위계 · 컨트롤 높이 · 표 렌더링을 개선안 기준으로 교체 (D1a · D2 · D3a · D4a · D1b, 커밋 14개)
- **변경 내용**: C1이 토큰 7개를 D1에서 한꺼번에 추가하도록 적었지만 참조 없는 dead 상수가 되므로 **쓰는 Step에서 추가**하는 쪽으로 바꿨다. **D4a(표)에서 세 번 헛짚었고 원인이 전부 그리기 코드가 아니라 적용 시점과 무효화 범위였다** — 행 높이는 `PreSubclassWindow`(WM_NCCREATE 시점)에서 `LVM_SETIMAGELIST`가 리스트뷰 내부 초기화 전이라 무시되어 `WM_CREATE` 기본 처리 뒤로 옮겼고, 헤더 글씨 뭉개짐은 `SetImageList`를 `CDDS_PREPAINT`에서 호출해 페인트 중 재레이아웃이 일어난 것이었고, 선택 액센트 바가 안 지워지는 것은 1열을 `CDRF_SKIPDEFAULT`로 직접 그리면 3px 구간을 아무도 덮지 않아서였고, 깜빡임은 전체 `Invalidate()`를 해당 행 rect로 축소해 잡았다. **다음 커스텀 드로잉은 그리기 코드보다 적용 시점·무효화 경로를 먼저 확정한다.** 강조 컬럼 색을 카멜로 바꾸자 이후 컬럼(입금 은행·비고)이 색을 물려받는 누수가 드러났다 — 본문색과 같을 때는 보이지 않던 결함이고, **강조가 켜지면 모든 서브아이템에서 색을 명시**해야 한다. `LVS_EX_GRIDLINES`는 가로·세로가 한 묶음이라 "가로만"을 만들 수 없어 커스텀 드로잉으로 하단 1px만 그었다. **D1b(서체)** — 기존 Gmarket 3종의 `AddFontMemResourceEx` 임베드 구조를 그대로 복제했고(폰트 6개가 되어 개별 핸들 멤버를 배열로 교체), 폰트 name 테이블을 직접 확인해 **GDI 패밀리 이름이 굵기마다 다르다**는 것을 확인했다 — GDI는 한 패밀리에 Regular/Bold/Italic/BoldItalic 4종만 담으므로 SemiBold가 자기 패밀리(`Pretendard SemiBold`)를 갖는다. **R1 재결정** — 목업 그대로(본문 13px) 적용하니 기존 앱(14.7px) 대비 체감 낙차가 커서 위계 비율만 유지하고 절대 크기를 한 단계 올렸다(제목 19 · 섹션 15 · 본문 14 · 표 셀 13px). **의도적으로 목업을 벗어난 유일한 지점이며 상수 5개라 되돌리기 쉽다.** `TAECHANG_BUTTON_TEXT_TOP_OFFSET`은 추측하지 않고 폰트 메트릭에서 계산했다 — Gmarket은 `winAsc/winDesc = 800/350`(비대칭)이라 계산값 +1.84로 기존 튜닝값 `2`와 일치해 계산 모델의 검증이 됐고, Pretendard는 `1949/494`로 거의 대칭이라 **0**이 됐다
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-06] docs/design-plan
- **목적**: 디자인 개선 계획(D0~D8) 수립과 D0~D4a·D1b 진행 기록 (커밋 4개, 문서 전용)
- **변경 내용**: Claude Design 개선안의 2·3·4장 전부를 대상으로 `docs/decisions/DESIGN_PLAN.md`를 세웠다. 목업 원문 HTML과 대조해 스킬에 적은 색 17개 중 16개가 실제로 존재함을 확인했고(예외는 앱 배경 `#F8F6F1`, R8 결정), **문서 본문과 목업이 어긋나는 지점을 조사로 확정해 별도 절로 남겼다.** 목업의 `_ds/.../colors_and_type.css`는 **EchoQuant라는 무관한 디자인 시스템**(다크 테마·블루 브랜드·Inter)이어서 가져오지 않았고 팔레트 출처는 목업 자신의 인라인 스타일임을 명시했다. **원본 HTML의 로컬 사본을 리포에 저장했다**(`docs/design/sagetaechang-design-proposal.dc.html`) — D7이 목업 CSS를 계속 실측해야 하고 Claude Design MCP 접근은 세션마다 재인증이 필요하며 원본이 수정될 수도 있어 **이 사본을 기준으로 삼는다**. 계획에 없던 **C9(크기의 동적 대응)** 절과 **D8(DPI 배율)** Step을 추가했다 — 리사이즈는 이미 대응되어 있으나 DPI 배율 대응이 전혀 없다는 것이 조사에서 드러났고, 성격이 달라 디자인 Step과 섞지 않고 분리했다(R11). 개선 전 앱 화면 캡처 19장은 용량·한글 파일명 때문에 git에 넣지 않고 리포 밖 경로를 문서에 적어 참조하기로 했다
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-06] design/label-widths
- **목적**: 폼 라벨 폭을 목업 기준으로 통일하고 금액 표현을 정리 (D3b · D4b)
- **변경 내용**: **D3b** — `TaechangDefine.h` 상수 5개만 바꿨고 **좌표 코드는 한 줄도 손대지 않았다**. 카드 폭·에디트 폭이 전부 라벨 폭 상수에서 파생되어 자동 전파됐다(단가계산 입력 카드 274→310, 비밀번호 변경 에디트 202→222). 착수 전 결론은 "본문 80 · 다이얼로그 64/96"이었는데 다이얼로그만 보고 낸 판단이었고, `<label style="width:">`를 목업 전체에서 뽑으니 **80은 두 개뿐이고 본문 폼도 64**였다. 폰트 의존 치수는 빌드 없이 PowerShell + `AddFontResourceEx(FR_PRIVATE)` + `GetTextExtentPoint32W`로 실측했다 — GDI+(`System.Drawing.Font`)는 private 폰트를 못 보고 **예외 없이 Microsoft Sans Serif로 폴백**해 8px 과대값을 내놨으므로 `GetTextFaceW`로 선택된 face를 찍어 확인했다. **D4b** — 계획 3항목 중 금액 우측 정렬은 결과 표 3종에 이미 적용돼 있었고(남은 건 `CListCtrl`을 직접 쓰는 단가 2표), Bold는 목업이 **미수금 열 하나만** 굵어 C6의 "금액 컬럼 + Bold"가 과잉이었으며, 음수 표시는 출처가 없어 **범위에서 뺐다**. 대신 계획에 없던 빈 금액 `—` 표시가 나왔다. `CListCtrl`이 0번 열 `LVCFMT`를 무시하므로 `SetCenterFirstColumn(BOOL)`을 `SetFirstColumnAlign(enum)`으로 확장했다. 우측 정렬로 바꾸자 `SagePriceManagePanel` 마지막 열이 남는 폭을 전부 흡수하던 결함이 드러나 균등 분배로 고쳤고, 단가 워크플로 전환 시 미수금 데이터 관리 컨트롤이 남는 버그도 함께 잡았다
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-06] feature/receivables-table-rows
- **목적**: 표의 반복 법인명을 `〃`로 표기하고 그룹 시작 행을 강조 (D4c)
- **변경 내용**: 계획 3항목 중 **1개는 요구사항이 틀렸고 1개는 목업에 없었다.** 「공백 행 skip」의 대상인 `-` 행은 `generate-receivables-form.ps1:232`이 법인이 바뀔 때 넣는 **고객사 요청 행**이라 화면에서도 유지했다 — 문서 3곳이 한목소리로 "제거"라고 적고 있어 합의된 것처럼 보였지만 전부 한 판단이 복제된 결과였다. 「법인 그룹 구분선」은 목업 3-1 표의 모든 행이 같은 `1px #EDE8E0`이고 캡션 산문에만 있어 제외했다. `〃`는 **프리젠터가 아니라 컨트롤**(`CSageListCtrl::SetGroupColumn`)이 판정한다 — 어떤 행이 앞 행인지는 필터를 통과한 뒤 정해지므로, 프리젠터가 넣으면 법인명 검색이 `〃`를 찾게 되고 그룹 첫 행이 필터로 걸러지면 `〃`만 남는다. D4b의 빈 금액 `—`는 행 하나로 결정되어 프리젠터가 맞았고, **판정 기준은 계층이 아니라 그 값을 정하는 데 다른 행이 필요한가**다. 그룹 시작 행 SemiBold를 넣으면서 `DrawGroupColumn`이 폰트를 복원하도록 했다 — 견적서 입력 표는 강조 열이 없어(`nHighlightCount = 0`) 뒤 subitem이 폰트를 다시 지정하지 않아 행 전체가 SemiBold가 됐다. 작업을 마친 뒤 **현재 화면 캡처**를 보고 견적서 입력 표에도 `-` 행이 있고 연속으로 나올 수 있음을 발견해(행 6·8·16·20·22·24·27·30) 두 번째 `-`가 `〃`가 되는 버그를 가드로 막았다 — 목업 3-8은 그 행들을 빼고 그려서 목업만으로는 알 수 없었다
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-06] feature/dialog-inline-error
- **목적**: 다이얼로그 7종의 입력 검증을 모달에서 인라인 에러로 이관 (D5a)
- **변경 내용**: 계획에 없던 항목이 둘 나왔고 둘 다 **"목업이 요구하는 표현을 그릴 주체가 없다"**였다 — `CSageInlineError` 외에 **`CSageEdit`**(에러 테두리를 바꿀 주체)과 **동적 창 높이**가 필요했다. `CSageEdit`은 스킬에 보류로 등재돼 있었으나 사유가 "패널이 컨트롤 **바깥** 1px에 그려서 승격하면 테두리가 이동한다"였고 **다이얼로그는 `WS_BORDER`(NC 안쪽)라 그 사유가 성립하지 않았다** — 보류 딱지만 보고 포기했으면 목업을 못 맞췄고, 사유를 안 읽고 전면 승격했으면 패널 16곳이 깨졌다. 구현은 `WS_BORDER` 위에 덧칠하는 방식으로 두 번 실패한 뒤(테마가 포커스·호버마다 다시 그려 덮고, `RedrawWindow`에 영역을 `NULL`로 주면 `WM_NCPAINT`가 오지 않는다) `WS_BORDER`를 걷어내고 `WM_NCCALCSIZE`로 1px을 직접 확보해 경쟁자를 없앴다. 에러 줄 자리를 넣으며 다이얼로그 높이 상수 6개에 손으로 같은 `+30`을 줬더니 행이 8줄인 단가 범위는 버튼이 잘리고 2줄인 로그인은 공백이 남았다 — `LayoutControls()`가 콘텐츠 바닥을 반환하고 `SageDialogSizer`가 창을 거기 맞추게 하니 **상수 6개가 통째로 사라졌다**. 같은 코드를 7종에 옮기며 지역 변수명 차이로 빌드 오류를 두 번 냈다(`rectEmpty` vs `r`, `nGap` vs `nBtnGap`)
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-06] feature/price-empty-state
- **목적**: 단가 데이터 관리에 빈 상태 화면 추가 (D5b)
- **변경 내용**: `CSageEmptyState`를 신설해 `SagePriceManagePanel`의 빈 격자를 안내문 + 1차 액션으로 교체했다. 작업 중 `TAECHANG_COLOR_SURFACE_MUTED`를 쓰다 빌드가 깨졌는데 **존재한 적이 없는 이름**이었다 — C1의 「신규 상수」 표는 *만들 것*의 목록이었고 D1a가 기존 `TAECHANG_COLOR_LIST_HEADER`의 값만 바꾸는 쪽을 택했으나 그 결정이 표에 반영되지 않았다. 스킬 색상표도 같은 이름을 적고 있었다. 전수 조사하니 신규 7개 중 **2개가 실체 없는 이름**이었다(`SURFACE_MUTED`, `SIDEBAR_ACCENT_WIDTH`). D3b에서 발견한 "스킬 타입 스케일이 코드와 어긋남"과 같은 뿌리이며, 그때는 **값**이 틀렸고 이번엔 **이름**이 틀렸다 — 이름이 틀리면 빌드가 깨지므로 **조용히 틀리는 값 쪽이 더 위험하다**
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged

## [2026-08-06] feature/icon-set
- **목적**: 아이콘 6종을 목업 규격(16px · 32×32 버튼 · 툴팁 필수)으로 통일 (D6)
- **변경 내용**: 유니코드 글리프(`↺`·`…`)를 전부 걷어내고 `CSageButton`이 직접 그리게 했다 — 서체마다 있고 없고가 갈리고 크기가 제각각이었다(진단 9). 추가 십자를 span 9 · 두께 2로 그렸더니 세로획이 가로획 중심에서 0.5px 벗어나 팔 길이가 위·아래 1px씩 달라 보였다. `FillSolidRect(x, y, w, h)`는 `[x, x+w)`를 채우므로 **홀수 길이와 짝수 두께는 같은 정수 격자에 못 앉는다** — span을 10으로 바꿔 해결했고 원·호는 두 획이 만나지 않아 이 문제가 드러나지 않는다. 십자가 이상하다는 지적에 **펜 → 사각형으로 그리는 방법만 두 번 바꿨고 둘 다 똑같이 보였다** — 치수를 그대로 뒀기 때문이다. 목업을 실측하니 span 8.2px인데 구현은 12px로 상자의 86%를 채우는 뭉툭한 십자였다. **화면이 이상하면 구현 방식보다 먼저 목업 수치와 대조한다.** 새로고침 아이콘은 `CDC::Arc`의 시작·끝점 순서가 호의 크기를 정하므로(반시계 방향이라 순서를 바꾸면 90°가 아니라 270°) 좌표 11개를 나열하던 기존 코드를 `Arc` 한 줄로 줄이면서 매직 넘버도 함께 없앴다. 내보내기·성공 2종은 쓸 버튼이 없어 참조 0곳이 되므로 D7-1로 넘겼다
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged


## [2026-08-06] refactor/result-table-panel-wire2
- **목적**: 결과 표·필터를 `SageResultTablePanel` 두 인스턴스로 재배선해 View에서 떼어냄 (3-B-4a, 구 4b 흡수)
- **변경 내용**: 폐기 브랜치에서 되살려둔 패널(`e4268de`)을 실제로 연결했다. **인스턴스 2개** — `m_panelInputTable`(납품·견적 입력 표) · `m_panelResultTable`(미수금 결과 표). 착수 전 조사에서 **납품·견적은 탭이 「입력·실행기록」 둘뿐이고 결과 탭이 없다**는 것이 드러나 두 패널이 동시에 보이는 경로가 없음을 확인했고, 그래서 계획서의 "인스턴스 1개" 중간 단계(구 4a)를 건너뛰고 4a에서 바로 2개로 갔다 — 1개짜리 단계는 필터 키워드·체크·한 페이지 상태를 View가 계속 들고 있어야 하고 그 배선을 다음 단계에서 통째로 버린다. 패널 선택은 워크플로 타입이 아니라 핸들러의 `UsesInputTable()`에 묻고(`FindResultTablePanel`), 배치·표시용으로는 탭 술어로 답하는 `FindVisibleResultTablePanel()`을 따로 뒀다(질문이 둘이다). **좌표 계약**은 계획서대로 `CRect(nLeft, nTop - GetBandHeight(), …)`이며 제목·리스트·필터·필터 카드 네 좌표를 손으로 전개해 이전 `LayoutResultSection`과 픽셀 동일함을 확인했다. `UpdateTaskTabVisibility`를 `LayoutResultSection`보다 **앞으로** 옮겼다 — 패널 `Layout()`이 표시 상태를 읽으므로 상태를 먼저 정해야 하고, 이전 코드는 `LayoutResultSection`이 술어를 다시 계산해 순서 의존이 없었다. **되살린 패널과 D 시리즈 View의 차이를 항목별로 대조해 5건을 더 찾았다** — 표 셀·헤더 폰트가 `SAGE_FONT_CONTENT`(규격은 `SAGE_FONT_LIST`), 검색 버튼이 `SAGE_BUTTON_PRIMARY`(입력 탭에 Primary 2개가 될 뻔함), 초기화 버튼 폰트, 콤보 항목 높이 누락, 필터 입력 길이 제한 누락. 체크박스의 `SetWindowTheme`도 제거했다(View에 없던 코드). `e4268de` 시점에는 3건만 찾았으므로 **되살리기는 "가져왔다"로 끝나지 않는다**는 것이 이번 교훈이다. Enter 검색은 패널 `PreTranslateMessage`로, 파일 드롭 허용은 패널 `EnableFileDrop`으로 이관했다(드롭 대상은 HWND별 지정이고 상위로 버블링되지 않는다 — 3줄 중복이 생겨 DEBT_LOG에 남김). `IsOnePageChecked()`의 가시성 가드는 제거했다 — 있으면 실행기록 탭에서 워크플로를 바꿀 때 체크 상태가 저장되지 않는다. 삭제: 컨트롤 멤버 9개 + 상태 3개, 함수 17개, 메시지맵 6항목. `SageTaechangView.cpp` **2,207 → 1,815줄**. 화면은 바뀌지 않아야 정상이다
- **PR 링크**: 없음 (develop 직접 머지)
- **결과**: merged — **빌드만 확인하고 머지했다. 화면 동작 9항목은 미확인**이며 `REFACTORING_PLAN` 맨 위에 남겨뒀다
