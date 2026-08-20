# 기술부채 로그 (DEBT_LOG)

이번 작업 범위 밖이라 남겨둔 위험 요소를 기록한다. 즉시 해결이 아니라 추적이 목적이다.
해결한 항목은 `## 해결됨` 섹션으로 옮긴다.

## 열린 항목

### [2026-08-20] 중복로직 — 「초기화」 문자열 상수가 셋이다
- 위치: `SageDefine.h` — `SAGE_UI_RESULT_RESET_BTN` · `SAGE_UI_INPUT_RESET_BTN` · `SAGE_UI_CALC_RESET_BTN`
- 설명: 값이 전부 `L"초기화"`다. D7-12에서 단가 계산의 아이콘 버튼을 텍스트 버튼으로 바꾸면서 세 번째가 생겼다. 화면별 UI 문자열 상수가 이 프로젝트 방식이지만 같은 한 단어가 셋이면 고칠 때 하나를 빠뜨린다.
- 위험도: 낮음
- 후속: 하나로 합친다. 다른 화면 2곳의 상수를 건드리므로 별 커밋으로 한다

### [2026-08-20] 구조불일치 — 카드 면·행 구분선·합계 밴드를 패널이 직접 그린다
- 위치: `app/ui/panels/SagePriceCalcPanel.cpp` `OnEraseBkgnd` · `DrawCard` (같은 방식이 `SageCompanyOrderPanel` · `SageWorkflowInputPanel`에도 있다)
- 설명: 표의 hairline은 `CSageListCtrl`이 그리는데 카드 안 키-값 행의 구분선은 패널이 그린다. `coding-design`의 「화면은 컨트롤을 그리는 방법을 몰라야 한다」에 어긋나지만, 카드 틀을 그리는 기존 방식이 세 패널에 이미 있어 그것을 따랐다.
- 위험도: 낮음
- 후속: `CSageCard` 승격 시 세 화면을 함께 본다. D7-12에서 헤더 밴드가 카드 테두리를 덮던 결함이 세 화면에 동시에 있었던 것도 **카드 틀에 주인이 없기 때문**이다

### [2026-08-20] 검증누락 — 좁은 창에서 2단 카드가 창 폭을 넘는지 확인하지 못했다
- 위치: `app/ui/panels/SagePriceCalcPanel.cpp` `LayoutChildControls` (`SAGE_CALC_RESULT_CARD_MIN_WIDTH` = 320)
- 설명: 입력 카드 420 고정 + gap 20 + 결과 카드 최소 320이므로 콘텐츠 폭이 760 미만이면 결과 카드가 오른쪽으로 넘친다. 최대화 상태로만 확인했다.
- 위험도: 낮음 — 최근 생성 내역 표가 이미 같은 조건에서 넘친다(컬럼 폭 합 1,127)
- 후속: 창 최소 크기와 함께 판단한다. 넘치는 것이 문제라면 2단을 1단으로 접는 분기가 필요하다

### [2026-08-20] 하드코딩 — 단가 계산의 `DrawEditBorder`만 리터럴 1을 쓴다
- 위치: `app/ui/panels/SagePriceCalcPanel.cpp` `DrawEditBorder` (`InflateRect(1, 1)` 외 4곳)
- 설명: `SageCompanyOrderPanel` · `SageWorkflowInputPanel`의 같은 함수는 `SAGE_EDIT_BORDER_WIDTH`를 쓴다. D7-12에서 이 함수는 손대지 않아 그대로 남았다.
- 위험도: 낮음
- 후속: `CSageEdit` 승격 시 세 함수가 함께 사라진다. 그전에 고칠 거면 상수 치환뿐이다

### [2026-08-20] 구조불일치 — 비활성 에딧 색이 다이얼로그와 패널에서 다르다
- 위치: `app/ui/drawing/SageEdit.cpp` `CtlColor` · `app/ui/panels/SagePriceManagePanel.cpp` · `SagePriceCalcPanel.cpp` · `SageCompanyOrderPanel.cpp`의 `OnCtlColor`
- 설명: 다이얼로그는 `CSageEdit`이 비활성 시 `#F2EEE7` 면 + `#B4ABA0` 글자로 스스로 그린다. 반면 패널의 순수 `CEdit`은 부모 `OnCtlColor`의 `CTLCOLOR_STATIC` 분기를 타 `#F8F6F1` 면 + **본문색 글자**로 나온다(비활성용으로 만든 분기가 아니라 정적 라벨용인데 비활성 에딧이 같은 메시지로 오기 때문이다). **같은 「최대부수 없음」이 단가 추가 다이얼로그와 단가 관리 패널 양쪽에 있어 나란히 비교된다.** 패널 쪽 `OnCtlColor`에 비활성 조건을 넣는 것은 `sagetaechang-ui`가 금지한 「부모의 컨트롤별 스타일 분기」이므로 두었다.
- 위험도: 낮음 — 두 색의 채널 차이가 6~9로 작아 나란히 놓지 않으면 눈에 띄지 않는다
- 후속: 패널·View 에딧 16곳을 `CSageEdit`으로 승격할 때 함께 사라진다 (테두리 1px 이동 때문에 보류 중인 작업)

### [2026-08-20] 검증누락 — 비활성 에딧의 글자색이 실제 적용되는지 확인하지 않았다
- 위치: `app/ui/drawing/SageEdit.cpp` `CtlColor`
- 설명: 반환 브러시로 **배경이 바뀌는 것은 확실하다.** 그러나 비활성 EDIT 컨트롤이 `WM_CTLCOLORSTATIC`으로 넘긴 `SetTextColor`를 존중하는지는 화면으로 확인하지 않았다. 컨트롤이 자체 회색으로 덮으면 글자색 지정만 무효가 된다. `SetWindowTheme(L"", L"")`으로 테마를 끈 상태라 기본 동작을 문서로 단정할 수 없었다.
- 위험도: 낮음 — 무효가 되더라도 배경이 바뀌므로 비활성은 읽힌다
- 후속: 화면 확인. 무시된다면 `CSageEdit`이 비활성 시 텍스트를 직접 그리는 방식을 검토한다

### [2026-08-09] 기존부채 — 행 높이용 1px 스페이서가 첫 열에 아이콘 슬롯을 만든다
- 위치: `app/ui/drawing/SageListCtrl.cpp` `ApplyFixedRowHeight` (`LVSIL_SMALL`, 1×34)
- 설명: `CListCtrl`에 행 높이 API가 없어 1px 더미 이미지리스트로 34를 확보하는데, 그 부작용으로 첫 열에 **1px 아이콘 슬롯**이 생긴다. 첫 열 커스텀드로우가 `LVIR_LABEL` rect에 얹혀 그리던 동안 이 슬롯이 미도색으로 남아 **흰 세로줄**로 드러났다(2026-08-09). 지금은 첫 열 **전체**를 직접 칠해 덮었다.
- 위험도: 낮음 — 덮여 있고, 첫 열에 실제 아이콘을 쓰려 할 때만 다시 부딪힌다
- 후속: 없애려면 `LVS_OWNERDRAWFIXED` + `WM_MEASUREITEM`으로 가야 하고 **모든 열을 직접 그려야 한다**(표 6곳 영향). 스페이서만 빼면 체크박스 없는 표 4곳의 행 높이가 무너진다. **더 싼 길은 높이 확보를 `LVSIL_STATE`로 옮기는 것이다** — 상태 이미지는 `LVS_EX_CHECKBOXES`가 없으면 그려지지 않지만 높이 계산에는 들어가므로, 스몰 아이콘 슬롯이 통째로 비어 첫 열 아이콘이 가능해진다. **선행 조건이던 상태 이미지리스트 소유권은 2026-08-09에 해소됐다**(위 해결 항목) — 이제 컨트롤이 멤버로 소유하고 `OnDestroy`에서 분리하므로 두 슬롯을 같은 방식으로 다룰 수 있다. **첫 열 아이콘 요구가 생길 때 이 경로로 전환한다**

### [2026-08-08] 기능저하 — 금액 자릿수를 눈으로 비교할 수 없다
- 위치: 미수금 결과 표(총액·입금액·미수금액) · 견적서 입력 표(단가·표지·운송) · 단가 표 2종
- 설명: 표 셀을 **전부 가운데 정렬**로 통일하면서 금액 열도 가운데가 됐다(사용자 결정). 우측 정렬일 때는 자릿수가 줄을 맞춰 `1,200,000`과 `980,000`의 크기 차이가 한눈에 보였는데, 가운데 정렬에서는 어긋난다. **Pretendard로 바꾼 이유가 「숫자 폭이 균일해 자릿수를 비교할 수 있게」였던 것과 방향이 반대다.**
- 위험도: 낮음 — 합계 바가 총액을 따로 보여주고, 금액 열 폭이 116~124로 좁아 어긋남이 크지 않다
- 후속: 실사용에서 불편하면 **금액 열만 우측으로 되돌린다.** 그때는 스킬 *정렬* 절의 이력도 함께 고친다

### [2026-08-08] 기존부채 — `m_brushListHeader`가 만들어지기만 하고 쓰이지 않는다
- 위치: `app/ui/view/SageTaechangView.h` · `.cpp` 생성자
- 설명: 생성자에서 `CreateSolidBrush`만 하고 참조하는 곳이 없다. **3-B-5b 이전부터 그랬다**(`develop`에서 확인). `CLAUDE.md` 3장에 따라 기존 dead code는 지우지 않고 알린다.
- 위험도: 낮음
- 후속: 사용자 확인 후 제거한다

### [2026-08-08] 목업이탈 — 사이드바 그룹 라벨의 위아래 여백을 재현하지 못한다
- 위치: `app/ui/drawing/SageSidebarTree.cpp` `DrawTreeItem`
- 설명: 목업 3-1의 그룹 라벨은 `padding:6px 20px 8px`(두 번째 그룹부터 위 18px)로 **항목보다 낮은 행**인데, `CTreeCtrl`은 `SetItemHeight`로 **모든 행이 같은 높이**(34)다. 그룹 라벨이 항목과 같은 34px 행을 차지해 목업보다 성기게 보인다.
- 위험도: 낮음 — 크기·색·자간은 맞췄다
- 후속: `TVS_NONEVENHEIGHT`로 행별 높이를 주거나, 사이드바를 트리가 아닌 커스텀 목록으로 바꾼다. **후자는 3-B-5a 범위를 크게 넘는다**

### [2026-08-08] 미완성 — 결과 표 필터가 아직 통합 검색 박스가 아니다
- 위치: `app/ui/panels/SageResultTablePanel.cpp` — `m_wndCriteria` · `m_wndFilter` · `m_wndSearchBtn`
- 설명: D7-6에서 `CSageSearchBox`를 만들어 데이터 관리에 적용했지만, **결과 표(목업 3-1)는 같은 테두리 안에 「기준 ▾」 칸이 하나 더 있는 3분할**이라 콤보를 박스 안으로 넣어야 한다. 지금은 두 화면의 검색 모양이 다르다.
- 위험도: 낮음 — 컨트롤은 공유하므로 구현이 갈라지지는 않았다
- 후속: `CSageSearchBox`에 **좌측 칸(콤보) 지원**을 추가해 결과 표에 적용한다. `DESIGN_PLAN` D7-11 등재 9번

### [2026-08-08] 판단보류 — 검색으로 거른 상태에서 ↑↓가 전체 순서 기준으로 움직인다
- 위치: `app/ui/panels/SageCompanyOrderPanel.cpp` `MoveSelected`
- 설명: 「출력 순서」는 전체 목록의 성질이라 **필터와 무관하게 `m_arrOrders`의 이웃**과 맞바꾼다. 다만 필터가 걸린 상태에서는 옮긴 항목이 화면 밖으로 갈 수 있다. **목업 3-6에 이 상황이 없어 내가 정했다.**
- 위험도: 낮음
- 후속: 화면에서 어색하면 필터 중 ↑↓를 비활성하거나, 필터를 해제하고 이동한다

### [2026-08-08] 구조불일치 — `SS_NOTIFY` 정적 컨트롤이 클릭 명령을 두 번 보낸다
- 위치: `app/ui/drawing/SageFilterPillBar.cpp`
- 설명: `SS_NOTIFY` 정적은 클릭 시 `STN_CLICKED`를 보내는데 **값이 `BN_CLICKED`와 같다(둘 다 0).** pill 바는 창 ID와 명령 ID가 같아서 **자기 `OnLButtonDown`이 보내는 것과 정적이 보내는 것이 겹친다.** 같은 필터로 한 번 더 그릴 뿐이라 화면상 무해하다. `CSageSearchBox`는 같은 함정을 **입력창 클릭만으로 검색이 실행되는 버그**로 겪어 ID를 갈랐다.
- 위험도: 낮음 — 결과가 같은 재그리기다
- 후속: pill 바에도 창 ID와 명령 ID를 갈라 준다. **`SS_NOTIFY` 컨트롤을 새로 만들 때는 두 ID를 반드시 다르게 둔다**

### [2026-08-08] 기존부채 — 데이터 관리 목록에 빈 상태가 없다
- 위치: `app/ui/panels/SageCompanyOrderPanel.cpp` · `SAGE_UI_CO_EMPTY_HINT`(미사용)
- 설명: 등록된 법인이 0개면 **빈 격자**가 뜬다 — 스킬이 금지하는 그림이다. 문구 상수는 있는데 **쓰인 적이 없다.** 목업 3-6에 빈 상태가 없고 D7-6 계획에도 없어 손대지 않았다.
- 위험도: 낮음 — 실사용에서 법인이 0개인 경우는 드물다
- 후속: 실행 기록(D7-5)처럼 `CSageEmptyState`를 붙인다

### [2026-08-07] 작업사고 — 일괄 치환 도구가 **네 번째로** 인코딩을 깨뜨렸다
- 위치: 작업 방식. 4d-3(`sed -i`, CRLF 1,683줄) · D7-4 3단계(`sed -i`, `ClearStatusCard` 무한 재귀 + CRLF) · D7-5 1단계(python heredoc, 한글이 깨져 치환 실패 + CRLF 939줄) · **표 가운데 정렬(`sed -i`, CRLF 646줄)**
- 설명: 소스가 **UTF-8 BOM + CRLF**인데 `sed -i`와 python `io.open` 재작성이 둘 다 이것을 보존하지 않는다. 매번 복구했지만 **같은 실수가 네 번 반복**됐고, D7-4에서는 치환이 새 함수 본문까지 먹어 **무한 재귀**를 만들었다(빌드 전에 발견).
- **후속을 적어두고도 네 번째를 냈다.** 「한글 든 파일은 Edit 도구로」를 2026-08-07에 써두고 다음 날 어겼다 — **규칙을 적는 것만으로는 안 걸러진다.**
- 위험도: 중 — 조용히 깨지고, 커밋 뒤에 발견하면 diff 전체가 오염된다
- 후속: **치환 도구를 쓸 거면 같은 명령 안에서 CRLF 복구까지 묶는다.** 나눠 실행하면 복구를 잊는다. `sed`/스크립트는 ASCII 한 줄 치환에만 쓰고, **쓴 직후 BOM·CRLF를 확인**한다. 함수 이름을 바꿀 때는 **정의부가 같이 걸리는지 먼저 본다**

### [2026-08-07] 목업이탈 — 선택된 필터 pill이 굵지 않다
- 위치: `app/ui/drawing/SageFilterPillBar.cpp` `DrawPill`
- 설명: 목업 3-5의 선택 pill은 `font-weight:600`인데 **12px SemiBold 폰트 역할이 없다**(`SAGE_FONT_CAPTION`은 Regular뿐). 역할을 새로 만들기보다 면·테두리·글자색 셋으로 선택을 표현했다.
- 위험도: 낮음 — 선택 표시가 세 겹이라 읽힌다
- 후속: 12px SemiBold가 다른 화면에서도 필요해지면 그때 폰트 역할을 추가하고 함께 적용한다. **한 곳 때문에 역할을 늘리지 않는다**

### [2026-08-07] 기존부채 — `SAGE_UI_HISTORY_SEPARATOR`가 쓰이지 않는다
- 위치: `SageDefine.h`
- 설명: D7-5에서 텍스트 조립 상수 8개를 지울 때 확인해 보니 **이 하나는 내 변경 전부터 사용처가 0**이었다. `CLAUDE.md` 3장이 「기존 dead code는 언급하되 지우지 않는다」이므로 남겼다.
- 위험도: 낮음
- 후속: 사용자 확인 후 제거한다

### [2026-08-07] 미완성 — 진행 문구가 단계와 무관한 고정 문장이다
- 위치: `SageDefine.h` `SAGE_UI_STATUS_CARD_RUNNING` · `SageWorkflowInputPanel::SetRunningState`
- 설명: 상태 카드가 「처리 중 — 엑셀 데이터를 읽는 중입니다」를 **항상 같은 문장으로** 띄운다. D7-4 명세의 「진행 문구 세분화」는 워커가 단계를 알려야 가능한데, **현재 워커는 진행 통지를 전혀 보내지 않는다** — 퍼센트도 300ms 타이머로 95%까지 흉내 내는 값이다(`SAGE_PROGRESS_STEP`). 문구만 바꾸면 거짓 정보가 된다.
- 위험도: 낮음 — 문장 자체는 모든 워크플로에서 사실이다(입력은 셋 다 엑셀)
- 후속: 워커 → 패널 진행 통지(`WM_` 메시지 + 단계 코드)를 넣는 Step에서 함께 한다. **퍼센트 흉내와 한 몸이므로 따로 고치지 않는다**

### [2026-08-07] 목업이탈 — 상태 카드가 사각이고 진행바도 pill이 아니다
- 위치: `app/ui/drawing/SageStatusCard.cpp` `DrawCardSurface` · `DrawProgressBar`
- 설명: 목업 3-4는 카드 `border-radius:6px`, 진행바 `border-radius:999px`(6px 높이의 완전 pill)이다. 둘 다 **사각으로 그렸다.** 라운드 코너는 범위 밖 Step R3이고, D7-4 1단계에서 입력 카드를 같은 이유로 사각으로 그렸으므로 **한 화면 안에서 카드 둘의 모서리가 갈리지 않게** 맞춘 것이다.
- 위험도: 낮음
- 후속: R3에서 카드 3종(입력 · 상태 · 완료)을 함께 라운드로 바꾼다. 진행바는 `RoundRect`로 따로 처리해야 한다

### [2026-08-07] 축소 — 입력 탭 표 영역이 38px 줄었다
- 위치: `app/ui/panels/SageWorkflowInputPanel.cpp` `GetTableAreaTop`
- 설명: 상태 영역이 `SAGE_BUTTON_HEIGHT`(32) → `SAGE_STATUS_CARD_HEIGHT`(70)로 커지면서 그만큼 표가 내려갔다. **미수금은 입력 탭에 표가 없어 무해하지만 납품·견적은 입력 표가 있다.** 목업 3-4에는 입력 표가 없어 실측 근거가 없다.
- 위험도: 낮음 — 표 최소 높이(`SAGE_RESULT_MIN_HEIGHT`)가 걸려 있어 잘리지는 않는다
- 후속: 납품·견적 입력 탭 화면을 확인할 때 표가 너무 짧아 보이면, 상태 카드를 표 위가 아니라 **입력 카드 우측**에 두는 안을 검토한다

### [2026-08-07] 구조불일치 — 워크플로 컨트롤러가 `ui → infra`를 직접 부른다
- 위치: `app/ui/workflow/SageWorkflowController.cpp` — `SageDeliveryExcelService` · `SageEstimateExcelService` · `SageReceivablesExcelService`
- 설명: 워커가 Excel 서비스를 직접 생성해 호출한다. `coding-design`의 계층 방향(`ui → core ← infra`)을 어긴다. **4d-3 이전부터 View에 있던 위반이고 이번 이동은 위치만 바꿨다** — 그 대가로 **View의 `app/infra` include가 6줄에서 0줄이 됐다.**
- 위험도: 중 — 워크플로를 하나 더 추가하면 이 `if` 사슬을 또 고쳐야 한다. 실행 경로가 서비스 구현에 묶여 있다
- 후속: Step 4 계열에서 `core`가 「워크플로 실행」 인터페이스를 정의하고 `infra`가 구현하게 한다. 그러면 워커의 워크플로 분기 3중 `if`도 함께 사라진다. 데이터 관리 패널·실행 기록 패널의 부채와 같은 뿌리다

### [2026-08-07] 구조불일치 — 데이터 관리 패널이 `ui → infra`를 직접 부른다
- 위치: `app/ui/panels/SageCompanyOrderPanel.cpp` — `sageDBMgr.GetReceivableCompanyOrderService()`의 `LoadAll` · `Add` · `Change` · `Remove` 4곳
- 설명: `coding-design`의 계층 방향(`ui → core ← infra`)을 어긴다. **4d-2 이전부터 View에 있던 위반이고 이번 이동은 위치만 바꿨다** — 「옮기기만 한다」 원칙에 따랐다. 다만 View에서는 `#include "app/infra/db/SageDBMgr.h"`가 사라져 **View 자체는 DB 매니저를 모르게 됐다.**
- 위험도: 중 — 화면이 서비스 생성·수명을 직접 알고 있어 테스트나 교체가 불가능하다
- 후속: Step 4 계열(핸들러·서비스 인터페이스)에서 `core`에 인터페이스를 세우고 주입받는다. 실행 기록 패널의 JSON 파싱과 같은 성격의 부채다

### [2026-08-07] 구조불일치 — 실행 기록 패널이 응답 JSON을 직접 파싱한다
- 위치: `app/ui/panels/SageWorkflowHistoryPanel.cpp` `BuildEntryLine` — `JsonExtractString` 4회
- 설명: `coding-design`은 「`ui`에 파싱·변환 코드가 들어가지 않았는가」를 묻는데 기록 한 줄을 만들려고 응답 JSON에서 파일 경로·메시지·코드를 직접 꺼낸다. **3-B-4c 이전부터 View에 있던 위반이고 이번 이동은 위치만 바꿨다** — 이번 Step 원칙이 「옮기기만 한다」였으므로 그대로 가져왔다.
- 위험도: 낮음 — 동작에는 문제가 없고 JSON 키 상수는 `SageDefine.h`에 있다
- 후속: **D7-5**에서 텍스트 로그를 5컬럼 표로 바꿀 때 항목 조립을 `app/core/`로 뺀다. 그때 텍스트 조립 상수 8개(`HISTORY_ENTRY_PREFIX` 등)도 함께 사라진다

### [2026-08-07] 구조불일치 — 데이터 관리 리스트만 하단 여백이 이중으로 걸린다
- 위치: `app/ui/view/SageTaechangView.cpp` `LayoutCompanyOrderPanel` — `nListHeight = nHeight - (nListTop - nTop) - SAGE_MARGIN`
- 설명: D7-11에서 `nHeight`가 `nContentBottom - nContentTop`(하단 패딩이 이미 빠진 정확한 남은 높이)으로 바뀌었는데 여기서 `SAGE_MARGIN`을 **또** 뺀다. 결과로 데이터 관리 탭 리스트 하단 여백만 다른 화면보다 16px 크다. 기존 대비 변화는 4px뿐이어서 화면 확인을 통과했다.
- 위험도: 낮음 — 여백이 넓을 뿐 잘리거나 겹치지 않는다
- 후속: D7-6(데이터 관리, 목업 3-6)에서 `- SAGE_MARGIN`을 걷어낸다. 그때 카드 내부 패딩(`nPad = SAGE_MARGIN`)이 목업과 맞는지도 함께 본다

### [2026-08-07] 기존부채 — View의 `ON_WM_DROPFILES`는 도달할 수 없고 프레임 드롭 등록은 무동작이다
- 위치: `app/ui/view/SageTaechangView.cpp` 메시지맵 `ON_WM_DROPFILES` · `PreTranslateMessage` · `OnCreate`의 `EnableFileDropForWindow(*pFrame)`
- 설명: `PreTranslateMessage`가 `WM_DROPFILES`를 잡고 `TRUE`를 반환하므로 **메시지맵 핸들러는 호출되지 않는다.** 또 프레임에 `DragAcceptFiles`를 걸어 두었지만 프레임에는 핸들러가 없고 `WalkPreTranslateTree`는 타깃에서 부모로 올라가므로 **프레임에 떨어뜨린 파일은 아무 일도 일으키지 않는다.** 3-B-4c 드롭 경계를 조사하다 발견했다.
- 위험도: 낮음 — 실제 진입점이 전부 View의 자손이라 현재 동작에는 영향이 없다
- 후속: 3-B-4d에서 드롭 소유자를 정할 때 둘 중 하나로 정리한다 — 핸들러를 살리거나 등록을 걷어낸다

### [2026-08-07] 중복로직 — `DrawEditBorder`가 세 곳에 복제됐다
- 위치: `app/ui/view/SageTaechangView.cpp` · `app/ui/panels/SageResultTablePanel.cpp` · `app/ui/panels/SageWorkflowInputPanel.cpp`
- 설명: 컨트롤 **바깥** 1px에 테두리를 그리는 같은 함수가 화면마다 복사돼 있다. 패널은 자기 에디트를 스스로 그려야 하므로 이번에 세 번째가 생겼다. 같은 이유로 입력 패널의 rect가 오른쪽으로 1px 넓다(`SAGE_EDIT_BORDER_WIDTH`) — 그렇지 않으면 우측 세로선이 클리핑된다.
- 위험도: 낮음 — 세 복사본이 아직 동일하다
- 후속: `CSageEdit` 승격(패널·View 16곳)이 끝나면 셋 다 사라지고 1px 보정도 함께 없어진다. `sagetaechang-ui` > *`CSageEdit`은 화면에 따라 갈린다*

### [2026-08-06] 구조불일치 — `SAGE_CALC_MAX_HISTORY`가 실제 상한이 아니다
- 위치: `app/ui/panels/SagePriceCalcPanel.cpp` `GetHistoryVisibleCapacity` · `TrimHistoryToVisibleCapacity`
- 설명: 이름은 「최대 보관 10건」으로 읽히지만 실제 보관량은 `GetCountPerPage()`(화면에 보이는 행 수)로 잘린다. `MAX_HISTORY`는 **창이 아직 없을 때의 폴백**으로만 쓰인다. 창이 작으면 3~4건이 남고, 크면 10건을 넘길 수도 있다. D7-2에서 「최근 10건」 배지를 붙이려다 발견했고, 배지는 **동적 「최근 N건」**으로 우회했다(사용자 결정).
- 위험도: 낮음 — 동작은 의도대로(스크롤 없이 보이는 만큼 유지)이고 이름만 어긋난다
- 후속: 상수를 폴백 의미에 맞는 이름으로 바꾸거나, 보관 상한을 10으로 고정하고 표를 스크롤 가능하게 만든다. **어느 쪽이든 동작 결정이 먼저다**

### [2026-08-06] 구조불일치 — 표에서 키보드 포커스 표시가 사라졌다
- 위치: `app/ui/drawing/SageListCtrl.cpp` `OnNMCustomDraw` (`CDDS_ITEMPREPAINT`)
- 설명: 선택이 풀린 행에 남던 점선 포커스 사각형을 없애려고 `CDIS_FOCUS`를 **선택 여부와 무관하게 항상** 걷어냈다. 단일 선택 표에서는 선택과 포커스가 함께 움직여 문제가 없지만, **Ctrl+화살표로 포커스만 옮기면 지금 어느 행에 있는지 화면에 표시가 없다.** `sagetaechang-ui` 접근성 절은 「테이블 행에 명확한 포커스/선택 상태가 있어야 한다」고 한다.
- 위험도: 낮음 — 마우스 조작에서는 드러나지 않는다
- 후속: 키보드 탐색을 실제로 쓰게 되면 점선 대신 **선택 행과 같은 액센트 바**로 포커스를 표시한다

### [2026-08-06] 기존부채 — 690행 표에 `InsertItem` 221ms가 남아 있다 (①은 적용됨)
- 위치: `app/ui/panels/SageResultTablePanel.cpp` `RefreshRows`
- 설명: 견적서 690행 파일에서 **로드·워크플로 전환·필터 초기화 모두 약 1초** 멈추던 문제다. 일회용 계측 브랜치로 구간을 갈라 재고 브랜치는 버렸다. **원인은 리스트 컨트롤에 데이터를 밀어넣는 메시지 자체**다.

  | 구간 | 측정 | 횟수 | 회당 | 상태 |
  |---|---|---|---|---|
  | `InsertItem` | **221 ms** | 690 | 320µs | **남음** |
  | `SetItemText` | **305 ms** | 5,520 | 55µs | 2026-08-07 제거 (①) |
  | 셀 문자열 생성(`GetRowText`) | 5 ms | 6,210 | 0.8µs | 보이는 행만으로 축소 |
  | 행 복사(`m_arrVisibleRows`) | 1 ms | | | |
  | 필터 판정 · 선택바 동기화 | 0 · 1 ms | | | |

  일반적인 ListView 삽입은 회당 1~5µs인데 320µs다. 컨트롤 설정(체크박스 상태 이미지 · 행 높이용 1×34 이미지리스트 · 커스텀드로우)과 얽힌 내부 비용으로 보이나 **그 이유는 확정하지 못했다.** `SetItemCount`로 아이템 개수를 미리 예약해도 변화가 없어(526 → 523ms) 내부 배열 재할당 가설은 폐기했다.

  **①(`LPSTR_TEXTCALLBACK`) 적용 완료 — `fix/large-table-render`, 2026-08-07.** 행당 메시지가 10회에서 1회로 줄었다. **적용 후 재측정은 하지 않았다** — 사용자가 690행 파일에서 체감으로 빨라진 것과 표 내용이 그대로인 것을 확인했을 뿐이다. 예상값 약 270ms는 계측된 수치가 아니다.
- 위험도: 중 — 690행짜리 파일이 일상적이면 매 조작마다 0.2초 이상 멈춘다
- 후속: **② 가상 리스트(`LVS_OWNERDATA`)** — `InsertItem` 221ms까지 없애지만 체크 상태를 우리가 보관해야 해서 선택·복원 로직을 다시 짜야 한다. 착수 전에 **현재 상태를 먼저 재측정한다** — ② 이후의 개선폭을 알아야 값어치를 판단할 수 있다

### [2026-08-07] 구조불일치 — 리스트 아이템 인덱스와 `m_arrVisibleRows` 인덱스가 1:1이라는 전제가 생겼다
- 위치: `app/ui/panels/SageResultTablePanel.cpp` `RefreshRows` · `OnListGetDispInfo`
- 설명: 셀 텍스트가 콜백이 되면서 표에 보이는 값이 `m_arrVisibleRows[iItem]`으로 결정된다. 지금은 삽입 순서와 벡터 순서가 같아 성립하지만, **정렬(`SortItems`)이나 부분 삽입·삭제를 도입하면 화면 값이 조용히 어긋난다.** 범위 밖 인덱스는 빈 문자열이 되어 크래시 대신 빈 셀로 나타나므로 눈치채기 어렵다.
- 위험도: 중
- 후속: 정렬을 넣게 되면 `lParam`(원본 행 번호)으로 되찾는 방식으로 바꾼다

### [2026-08-06] 구조불일치 — 「선택 해제」가 견적서 화면에도 보인다
- 위치: `app/ui/panels/SageResultTablePanel.cpp` (입력 표 인스턴스는 납품·견적 공용)
- 설명: 목업 3-7(납품)에는 「선택 해제」가 있고 **3-8(견적)에는 없다.** 두 화면이 같은 패널 인스턴스라 한쪽만 숨기려면 패널을 쪼개거나 표시 술어를 하나 더 만들어야 한다.
- 위험도: 낮음 — 동작은 정상이고 목업과만 다르다
- 후속: 견적 화면에서 실제로 거슬리는지 먼저 보고, 필요하면 `ShowSelectionClear(BOOL)`를 추가한다

### [2026-08-06] 기존부채 — 도달할 수 없는 6행 초과 경고가 핸들러에 남아 있다
- 위치: `app/core/workflow/handlers/SageEstimateWorkflowHandler.cpp` `ValidateSelectedRows` (`SAGE_UI_ESTIMATE_ONE_PAGE_LIMIT`)
- 설명: D7-8 2단계에서 패널의 경고창 3곳을 없앴다. 6행 초과는 **체크 시점에 자동 해제**되므로 생성 시점 검증은 도달할 수 없는 방어 코드가 됐다. 상수는 이 한 곳이 유일한 참조다.
- 위험도: 낮음
- 후속: 자동 해제를 신뢰한다면 검증과 상수를 함께 지운다. 남긴다면 왜 남기는지 근거가 필요하다

### [2026-08-06] 구조불일치 — 체크박스가 한 화면에 두 종류다
- 위치: `app/ui/drawing/SageSelectionBar.cpp` `m_wndSelectAll` (네이티브 `BS_AUTOCHECKBOX`)
- 설명: **D7-8 2단계에서 절반 해소됐다.** 표 안 체크박스와 「한 페이지 작성」이 `SageUiStyle::DrawCheckBox`를 공유해 둘 다 카멜색이 됐고, 선택 바의 **「전체 선택」만 네이티브로 남아** 선택 시 파란색이다.
- 위험도: 낮음
- 후속: `CSageCheckBox`로 승격해 셋을 통일한다. 그리기 조각은 이미 `SageUiStyle`에 있으므로 라벨·클릭 처리만 감싸면 된다

### [2026-08-06] 기존부채 — 입력 파일이 워크플로와 맞는지 검사하지 않는다
- 위치: `app/ui/panels/SageWorkspacePanel.cpp` `DisplayResponse` · `tools/load-*.ps1` (3-B에서 View → 워크스페이스 패널로 옮겨졌다)
- 설명: 견적서 생성에 미수금 파일(`receivables_202603.xls`)을 넣으면 **오류 없이** 견적서 열 배치로 읽어, 날짜 자리에 Excel 일련번호(`46084`), 부수 자리에 「법인/회사」가 들어간 표가 나온다. 사용자가 실제로 겪었다.
- 위험도: 중 — 잘못된 문서가 생성될 수 있다
- 시도와 철회 (2026-08-13): **검사를 넣지 않기로 결정했다**(사용자 결정). 세 방식을 검토했고 앞의 둘은 정상 파일을 막는 것이 확인됐다 — ① **기대 열의 숫자 비율 판정**(커밋 `18a48c0`, 폐기): 납품서의 부수 3열이 실제로는 자유 텍스트라 **정상 백업 파일이 100% 차단**됐다. 실측값이 `10부` · `각7부` · `X` · `여분(2부)` · 빈칸이다. ② **헤더 문구 대조**(부채의 원래 후속안): 헤더는 파일마다 달라질 수 있다는 사용자 지적으로 폐기됐다. ③ **세 서식 중 최근접을 고르는 상대 판정**(셀 서식으로 날짜 열 판별 + 데이터 시작 행 · 마지막 사용 열 · 개행 포함 텍스트 열 지문): 임계값이 없어 이상한 정상 파일에 강하지만, **검사 자체를 빼는 쪽으로 결정**됐다
- 후속: **「헤더 대조」 원안은 무효다.** 위험은 그대로 남아 있으므로 항목을 닫지 않는다. 다시 열 때는 **차단이 아니라 경고**부터 검토한다 — 오늘 사고의 피해는 「엉뚱한 문서」가 아니라 「정상 파일을 못 쓰는 것」이었다. 착수 전에 워크플로 3종의 **실제 입력 파일을 하나씩 열어 본다**: 이번 실패의 원인은 불일치 케이스만 검증하고 정상 케이스를 데이터로 확인하지 않은 것이다

### [2026-08-06] 구조불일치 — 합계 밴드 셀 역할 enum이 core와 ui에 각각 있다
- 위치: `app/core/workflow/SageWorkflowResultTable.h` `SageResultTotalRole` ↔ `app/ui/drawing/SageTableTotalBar.h` `SageTotalBarCellStyle`, 매핑은 `app/ui/panels/SageResultTablePanel.cpp` `ToTotalBarCellStyle`
- 설명: 값 4개짜리 enum이 **두 벌**이고 패널이 변환한다. 하나로 합치려면 컨트롤이 `core/workflow` 헤더를 물어야 하는데, `CSageSummaryBar`가 자기 `SageSummaryBarItem`을 갖고 패널이 변환하는 **1단계 선례**를 따라 갈랐다 (`sagetaechang-ui` > *컨트롤은 도메인 개념을 알면 안 된다*).
- 위험도: 낮음 — 표현 역할이 늘면 두 곳을 함께 고쳐야 한다
- 후속: 세 번째 표에 밴드가 필요해질 때 유지할지 다시 본다. 합칠 거면 컨트롤이 core를 무는 방향이 아니라 **렌더링 전용 헤더를 따로 두는** 방향이다

### [2026-08-06] 기존부채 — 리스트 세로 스크롤바가 뜨면 밴드 우측 끝이 남는다
- 위치: `app/ui/panels/SageResultTablePanel.cpp` `LayoutTableArea` · `UpdateTotalBarCells`
- 설명: 밴드는 표 폭(패널 폭) 전체를 차지하지만 셀 좌표는 **리스트 클라이언트 폭**에서 나온다. 스크롤바 폭만큼 우측이 밴드 배경으로 남는다. 맨 끝 「비고」 열이 합계 행에서 빈칸이라 값이 어긋나 보이지는 않는다.
- 위험도: 낮음
- 후속: 거슬리면 밴드 폭을 리스트 클라이언트 폭에 맞춘다. 그러면 밴드 배경이 표보다 좁아져 하단 경계선이 끊긴다 — 어느 쪽을 택할지가 결정 사항이다

### [2026-08-06] 임시구현 — 합계 금액이 0이면 `—`가 아니라 「0」으로 나온다
- 위치: `app/core/workflow/handlers/SageReceivablesWorkflowHandler.cpp` `BuildResultTotals`
- 설명: 표의 빈 금액은 `—`(`SAGE_UI_AMOUNT_EMPTY_MARK`)인데 밴드는 0을 그대로 쓴다. **빈 값이 아니라 계산된 0**이라 구분한 것이다. 입금이 하나도 없는 파일에서 입금 합계가 「0」으로 보인다.
- 위험도: 낮음
- 후속: 사용자가 어색해하면 0일 때만 `—`로 바꾼다

### [2026-08-06] 기존부채 — 창을 극단적으로 줄이면 합계 밴드가 패널 아래로 밀린다
- 위치: `app/ui/panels/SageResultTablePanel.cpp` `LayoutTableArea`
- 설명: 리스트 높이가 `SAGE_RESULT_MIN_HEIGHT`로 클램프되면 밴드 top이 패널 바닥을 넘는다. 밴드 높이를 빼는 만큼 **클램프가 이전보다 일찍 걸린다.**
- 위험도: 낮음 — 정상 창 크기에서는 도달하지 않는다
- 후속: 클램프가 걸리면 밴드를 패널 바닥에 고정하고 리스트를 그만큼 줄인다

### [2026-08-06] 기존부채 — 미수금 내역서 생성이 서식 복사를 클립보드로 9번 왕복한다
- 위치: `tools/generate-receivables-form.ps1` `Copy-RowFormat`(:214) ← `Apply-OutputFormats`(:320)
- 설명: `Rows.Copy()` + `PasteSpecial` + `CutCopyMode=$false`로 Excel 클립보드를 왕복한다. 전체 범위 1회 + **구분 행(`-`)마다 1회**라 19건·구분 8개인 파일에서 9회 돈다. 좀비 Excel을 정리한 깨끗한 상태에서 생성이 **4.0초**, 로드는 같은 파일에 **2.7초** — 차이의 상당 부분이 이 왕복이다.
- 위험도: 낮음 — 느리지만 결과는 정확하다. 행이 늘면 구분 행도 늘어 선형으로 커진다
- 후속: 구분 행 서식을 개별 복사하지 않고 **한 번에 처리**하거나(연속 구간 묶기), `Copy/PasteSpecial` 대신 서식 속성을 직접 지정한다. 성능 작업이므로 D 시리즈와 섞지 않고 별도 브랜치로 한다

### [2026-08-06] 검증누락 — Excel 프로세스 종료를 스크립트 5개에서 실행 확인하지 못했다
- 위치: `tools/excel-process.ps1` `Stop-OwnedExcelProcess` ← 워크플로 스크립트 7개
- 설명: 자기가 띄운 Excel PID만 종료하는 수정을 7개에 동일하게 넣었으나, **실제 실행으로 확인한 것은 미수금 생성·로드 2개뿐**이다(리빌드 후 생성 3회 연속 4초대 유지 확인). 납품·견적·단가 경로 5개는 문법 검사와 코드 동일성만 확인했다. Claude 세션은 `Stop-Process`가 Access denied라 종료 자체를 검증할 수 없었다.
- 위험도: 낮음 — `try/catch`로 감싸 실패해도 동작에 영향이 없다
- 후속: 납품서·견적서 생성을 실제로 돌릴 때 `Get-Process EXCEL | Where-Object { $_.Handles -gt 0 }`로 살아있는 인스턴스가 남지 않는지 확인한다. **핸들 0개 항목은 이미 죽은 껍데기이므로 개수로 판정하지 않는다**

### [2026-08-06] 미완 — 미수금 결과의 「내보내기」 버튼과 아이콘 2종이 아직 없다
- 위치: `DESIGN_PLAN` D7-1 표 · D6 아이콘 목록(내보내기 15 · 성공 18)
- 설명: D7-1 착수 시 사용자 결정으로 **만들지 않았다.** 결과 표를 파일로 내보내는 기능이 코드에 없어, 버튼만 두면 D7-4의 *없는 기능을 안내하지 않는다* 원칙을 어긴다. D6이 D7-1로 넘긴 아이콘 2종도 그대로 **참조 0곳**이다.
- 위험도: 낮음
- 후속: 내보내기의 동작(결과 표를 엑셀로 저장 vs 저장 폴더 열기)을 먼저 정하고, 정해지면 버튼과 아이콘 2종을 함께 넣는다. **D5 공통 완료 기준**(참조 0곳인 컨트롤을 남기지 않는다)이 아이콘에도 걸려 있다

### [2026-08-06] 구조불일치 — 요약 바의 「기타 처리 법인」만 필터와 연동되지 않는다
- 위치: `app/core/workflow/handlers/SageReceivablesWorkflowHandler.cpp` `BuildResultSummary`
- 설명: 총 건수와 미수금 합계는 **보이는 행 기준**으로 갱신되지만(사용자 결정), 기타 처리 법인 건수는 응답 JSON의 `missingCompanies` 배열 길이라 **필터와 무관하게 전체 기준**이다. 행 데이터에 "이 행이 기타 처리인가" 플래그가 없어서 행에서 셀 수 없다.
- 위험도: 낮음 — 필터를 걸면 세 항목 중 하나만 안 바뀌어 오해할 수 있다
- 후속: 행 기준으로 세려면 프리젠터가 `missingCompanies`와 법인명을 대조해 행에 플래그를 담아야 한다. **D7-1 2단계나 그 이후에 필요해지면 한다** — 지금은 목업(총 24건 · 합계 · 기타 처리 2건)과 표시가 일치한다

### [2026-08-06] 기존부채 — `SAGE_UI_RECEIVABLES_PREVIEW_TOTAL`이 참조 0곳이다
- 위치: `SageDefine.h:276`
- 설명: `L"미리보기 건수"`. D7-1이 이 상수를 소비할 것으로 계획서에 적혀 있었으나, 목업 3-1의 라벨은 「총」이라 `SAGE_UI_RECEIVABLES_SUMMARY_TOTAL`을 새로 만들었다. 이 상수는 여전히 아무도 쓰지 않는다(기존 죽은 코드라 삭제하지 않았다).
- 위험도: 낮음
- 후속: D7 전체가 끝난 뒤 죽은 UI 상수를 한 번에 정리할 때 함께 제거한다

### [2026-08-06] 중복로직 — 파일 드롭 허용 코드가 View와 패널에 각각 있다
- 위치: `app/ui/view/SageTaechangView.cpp` `EnableFileDropForWindow` ↔ `app/ui/panels/SageResultTablePanel.cpp` `AcceptDroppedFiles`(static)
- 설명: 3-B-4a에서 결과 표가 패널 안으로 들어가면서 `EnableFileDropForWindow(m_wndResultSection)` · `(m_wndResultList)` 두 줄이 갈 곳이 없어졌다. **드롭 대상은 HWND별로 지정해야 하고 상위 창으로 버블링되지 않으므로** 패널이 자기 자식에게 직접 걸어야 한다. `DragAcceptFiles` + `ChangeWindowMessageFilterEx` 3줄이 두 파일에 생겼다.
- 위험도: 낮음 — 한쪽만 고치면 드롭이 조용히 안 먹는다
- 후속: **3-B-4b~4d에서 패널이 더 늘어날 때 공용 헬퍼로 뽑는다.** `CWnd`를 받으므로 `app/common`이 아니라 `app/ui/`에 둔다. 지금 뽑으면 재배선 커밋에 무관한 파일이 섞인다
- 함께: View 쪽은 필터 상수에 리터럴 `0x0049`를 그대로 쓰고 패널은 `WM_SAGE_COPYGLOBALDATA`를 쓴다. 같은 값이 두 표기로 있으므로 헬퍼를 뽑을 때 한쪽으로 맞춘다

### [2026-08-06] 미완 — 패널·View의 입력 검증이 아직 모달이다
- 위치: `app/ui/panels/SagePriceManagePanel.cpp` · `app/ui/panels/SagePriceCalcPanel.cpp` · `app/ui/panels/SageCompanyOrderPanel.cpp` · `app/ui/panels/SageWorkspacePanel.cpp`
- 설명: D5a에서 다이얼로그 7종의 검증 22곳을 `CSageInlineError`로 옮겼다. **패널·View의 51곳은 그대로 모달이다.** 계획 R7과 D5a가 "다이얼로그 6종"으로 범위를 잡았기 때문인데, 사용자에게는 같은 경험이다 — 단가 데이터 관리에서 법인 미선택으로 「단가 추가」를 누르면 여전히 `AfxMessageBox`가 뜬다.
- **정정 (2026-08-19, D9)**: 위 위치와 「51곳」은 낡은 값이었다. `SageTaechangView.cpp`의 18곳은 3-B 패널 분리로 **0곳**이 됐고 그 몫이 패널로 옮겨갔다. 또 D9에서 모달을 전부 `ShowSageMessageBox`로 바꿔 **모양은 앱 스타일이 됐다** — 남은 부채는 「Windows 기본 창이 뜬다」가 아니라 **「입력 검증이 인라인이 아니라 모달이다」** 하나로 좁혀졌다
- 위험도: 낮음
- 후속: **디자인 근거가 아직 없다.** 목업 3-3에는 인라인 오류 자리가 없고(빈 상태와 상세 카드뿐), 「법인을 선택하세요」는 입력값 오류가 아니라 선택 누락이라 성격도 다르다. **D7-3에서 단가 데이터 관리 화면을 설계할 때 자리와 형태를 함께 정한다.** 전부가 대상은 아니다 — 완료 알림과 서비스 오류는 모달이 맞다

### [2026-08-06] 기존부채 — 헤더 가운데 정렬 강제 코드가 무의미해졌다
- 위치: `app/ui/panels/SagePriceManagePanel.cpp` `CreateControls` (헤더 서브클래싱 직후 `HDITEM` 5줄)
- 설명: 단가 범위 표 헤더 0번 항목에 `HDF_CENTER`를 직접 넣는 코드다. `CListCtrl`이 0번 열 헤더를 좌측으로 두는 것을 보정하려던 것으로 보인다. **D4b에서 `CSageHeaderCtrl::OnPaint`가 `fmt`를 아예 보지 않고 항상 `DT_CENTER`로 그리게 바뀌어 이 5줄은 아무 효과가 없다.** 제거 후보였으나 사용자 판단으로 두었다 — 지금 지우면 D4b 커밋에 무관한 변경이 섞인다.
- 위험도: 낮음
- 후속: **D7-3(단가 데이터 관리 화면)에서 이 파일을 손댈 때 함께 제거한다.** 그 전에 헤더 정렬 방침이 다시 바뀌면 이 코드가 되살아날 수 있으니 먼저 `sagetaechang-ui`의 *헤더 정렬* 절을 확인한다

### [2026-08-06] 미완 — 단가 범위 표가 목업 비율·「관리」 열과 다르다
- 위치: `app/ui/panels/SagePriceManagePanel.cpp` `LayoutChildControls` 컬럼 폭 계산
- 설명: D4b에서 표지 단가 열만 남는 폭을 흡수하던 문제를 고쳐 두 단가 열을 균등 분배했다. 다만 목업 3-3의 실제 비율은 `1fr 1fr 1.2fr 1.2fr 88px`이고 5번째 「관리」 열(행내 「수정」 버튼)이 있다. 현재는 최소/최대가 80 고정이라 비율이 다르고 「관리」 열이 없다.
- 위험도: 낮음
- 후속: **D7-3에서 「관리」 열을 추가할 때 4열 비율을 목업대로 다시 배분한다.** 열이 하나 늘면 폭 계산이 어차피 바뀐다

### [2026-08-06] 구조불일치 — 폼 라벨 정렬이 화면마다 다르고 목업과도 어긋난다
- 위치: `app/ui/panels/SagePriceCalcPanel.cpp:50,53,55,64,66,68,70,75` (`SS_RIGHT`) ↔ `app/ui/panels/SagePriceManagePanel.cpp:84,87,91,93` (`SS_LEFT`)
- 설명: 단가 계산 패널 라벨은 전부 우측 정렬인데 단가 관리 상세 폼은 좌측 정렬이다. 목업은 「페이지」 하나만 `text-align:right`이고 나머지 라벨은 전부 좌측 정렬이므로 **양쪽 다 목업과 완전히 맞지는 않는다.** D3b는 "라벨 폭 통일"이 범위라 정렬은 손대지 않았다. 폭이 46→64로 커지면서 `SS_RIGHT` 쪽은 라벨 텍스트가 필드에 더 가까이 붙었다(간격 4px).
- 위험도: 낮음
- **2026-08-20 갱신 (D7-12)**: **단가 계산 쪽은 해소됐다** — 목업 3-2대로 「페이지」만 `SS_RIGHT`이고 나머지는 `SS_LEFT`이며, 폼 라벨 색도 `TEXT_MUTED`(목업 `#6E655B`)가 됐다. **남은 것은 단가 관리 상세 폼 한 곳**이고 그 화면의 목업(3-3) 대조가 필요하다
- 후속: 단가 관리 상세 폼을 목업 3-3과 대조할 때 정렬을 정한다

### [2026-08-05] 미검증 — 금액 tabular 정렬과 선택 행 Bold를 적용하지 못했다
- 위치: `app/ui/drawing/SageListCtrl.cpp`, 개선안 3장 3-1 · 3-3
- 설명: 목업은 금액을 `font-variant-numeric:tabular-nums` + Bold로, 선택 행을 Bold로 표시한다. GDI `DrawText`는 OpenType `tnum`을 못 켜고, Bold는 볼드 폰트 리소스가 필요하다. 지금 Gmarket 볼드로 맞추면 Pretendard 전환 때 다시 재야 한다. 또 목업 자체가 불일치한다 — 3-3 선택 행은 전 셀 weight 700인데 3-1 선택 행은 합계·입금 셀에 굵기가 없다.
- 위험도: 낮음
- 후속: **D4b에서 절반 해결됐다.** 금액 Bold는 `SAGE_FONT_LIST_BOLD`를 만들어 적용했고(미수금 열 한정 — 목업 3-1 실측), tabular는 R4대로 우측 정렬로 대체했다. **선택 행 전체 Bold는 하지 않았다** — 목업 3-1과 3-3이 서로 다르고, 3-1 기준으로는 선택 행에 굵기가 없다. **D7-3에서 3-3 화면을 볼 때 판단한다**

### [2026-08-05] 기존부채 — 참조 없는 상수 2개
- 위치: `SageDefine.h` `SAGE_LABEL_WIDTH`(=90) · `SAGE_RECEIVABLES_COL_IDX_DEPOSIT_AMOUNT`(=6)
- 설명: 둘 다 정의만 있고 참조가 0곳이다. `git grep HEAD`로 **내 변경 이전부터** 참조가 없었음을 확인했다(내 변경이 고아로 만든 `..._COL_IDX_TOTAL_AMOUNT`는 규칙대로 제거함). `SAGE_LABEL_WIDTH`는 계획서에 90→80으로 적혀 있었으나 죽은 상수라 바꿔도 효과가 없다. D3b에서 원인을 확인했다 — 이 상수를 쓸 「입력 파일」·「저장 위치」 라벨(`m_wndInputLabel`·`m_wndOutputLabel`)이 생성 후 `ShowWindow(SW_HIDE)`만 되고 `MoveWindow`가 한 번도 불리지 않는다(`SageTaechangView.cpp:270,490`). 계획서 C4에서 이 항목을 「변경 없음」으로 정정했다.
- 위험도: 낮음
- 후속: `DEPOSIT_AMOUNT`는 **D4b에서 쓰이지 않았다** — 우측 정렬이 핸들러의 `SAGE_COLUMN_ALIGN_RIGHT`로 이미 되어 있어 열 인덱스 상수가 필요 없었다. 여전히 죽은 상수다. 둘 다 Step 5 접두사 전환 때 함께 정리한다

### [2026-08-05] 기존부채 — INPUT_PANEL_HEIGHT가 파생값이 아니라 매직넘버다
- 위치: `SageDefine.h` `SAGE_INPUT_PANEL_HEIGHT`
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
- 후속: 기존 부채 *입력 컨트롤 테두리 방식이 View와 다이얼로그에서 다름*과 같은 뿌리다. `CSageEdit` 승격 때 한 번에 없앤다. **`CSageEdit`은 D5a에서 만들어졌고 다이얼로그에만 적용됐다** — 패널·View로 넓힐 때 이 중복도 사라진다

### [2026-08-05] 구조불일치 — core 서비스 인스턴스 획득 방식이 두 가지다
- 위치: app/ui/view/SageTaechangView.cpp `UpdateCalcPreview` / `UpdateCalcTotal`
- 설명: 기존 서비스 6개는 `SageDBMgr`이 보유하고 `Get*`으로 노출하는데, `SagePriceCalcService`만 호출 지점에서 스택 생성한다(2곳). 무상태라 동작 문제는 없지만 다음 core 서비스를 만들 때 어느 쪽을 따를지 근거가 없다. Step 3-B-1a에서 `SageDBMgr`을 고치지 않는 쪽을 택한 결과다.
- 위험도: 낮음
- 후속: Step 4-B 의존 역전에서 서비스 획득 경로를 정할 때 통일한다

### [2026-08-05] 구조불일치 — 견적 입력 표가 행 구조의 범용 멤버를 빌려 쓴다
- 위치: app/core/workflow/handlers/SageEstimateWorkflowHandler.cpp `g_inputColumns` ↔ app/core/workflow/SageWorkflowResultPresenter.cpp `AddEstimateInputRows`
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
- 설명: `m_wndHeaderStatus`가 `WS_CHILD | SS_RIGHT`로만 생성되어 **`WS_VISIBLE`이 없고**, `ShowWindow` 호출도 없으며 레이아웃에서 크기를 `0,0,0,0`으로 준다. 즉 화면에 표시된 적이 없다. 그런데 이 컨트롤을 위해 멤버 2개(`m_colorHeaderStatus`/`m_nHeaderStatusBgRole`), 함수 2개(`ResolveStatusColor`/`ResolveStatusBgRole`), `OnCtlColor` 분기 5줄, 상수 3개(`SAGE_COLOR_STATUS_BG_SUCCESS`/`WARNING`/`ERROR` — 다른 사용처 없음)가 유지되고 있다. `SetStatusText`가 같이 호출하는 `pFrame->SetMessageText`(창 하단 상태바)는 정상 동작하므로 상태 표시 자체가 안 되는 것은 아니다. PR_LOG의 상태별 색 기능(PR #15)에서 도입됐고 컨트롤을 보이게 하는 단계가 빠진 것으로 보인다. 3-A-8 2단계에서 브러시를 이관하다 발견했다.
- 위험도: 낮음
- 후속: **3-B-5b(`SageHeaderPanel`) 착수 전에 결정한다 (계획의 R9).** 숨긴 것이 의도인지 미완성인지 확인이 필요하다. 미완성이면 `WS_VISIBLE` 추가와 `MoveWindow` 좌표 부여, 불필요하면 관련 멤버·함수·분기·상수를 함께 제거. 확인 없이 패널로 옮기면 죽은 코드를 새 패널로 복제한다

### [2026-08-01] 기존부채 — m_brushListHeader가 생성만 되고 쓰이지 않음
- 위치: app/ui/view/SageTaechangView.cpp:277 (생성자)
- 설명: `CreateSolidBrush(SAGE_COLOR_LIST_HEADER)`로 만들지만 반환·사용하는 곳이 없다. 리스트 헤더 배경은 `CSageHeaderCtrl::OnPaint`가 `FillSolidRect`로 직접 칠하므로 남은 흔적이다. 3-A-8 2단계에서 나머지 브러시 4개를 저장소로 옮기면서 드러났다. 이번 변경이 만든 것이 아니라 그대로 뒀다.
- 위험도: 낮음
- 후속: 멤버 선언과 생성 2줄 제거

### [2026-08-01] 구조불일치 — 컨트롤의 폰트 선택 가드가 6곳 중 2곳만 다름
- 위치: app/ui/drawing/SageButton.cpp:57, app/ui/drawing/SageSectionLabel.cpp:17
- 설명: 그리기 중 폰트를 거는 6곳 가운데 `CSageTabCtrl`·`CSageHeaderCtrl`·`CSageFilterComboBox`(2곳)는 `pFont ? dc.SelectObject(pFont) : NULL`로 가드하는데, 이 2곳은 `pDC->SelectObject(GetFont())`로 가드가 없다. `GetFont()`가 NULL이면 디버그 빌드에서 ASSERT가 뜬다. 실제로는 두 컨트롤 모두 `ApplyControlFonts`에서 항상 `SetFont`을 받으므로 재현되지 않는다. 3-A-7 중복 조사 중 발견했고, 고치면 3-A-7 범위(콤보 화살표 추출)가 흐려져 남겼다.
- 위험도: 낮음
- 후속: 폰트 저장소 작업(`ApplyControlFonts` 해체) 때 6곳을 한 번에 통일한다. 그때 컨트롤이 자기 폰트를 갖게 되므로 가드 방식도 함께 정해진다

### [2026-08-01] 기존부채 — Excel COM 최적화 설정이 없음
- 위치: app/infra/office/SageReceivablesExcelService.cpp (및 office 계층 전반)
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
- **[2026-08-06 D5a] 다이얼로그 쪽은 해소됐다.** 7종의 `CEdit`을 `CSageEdit`으로 교체했다. `WS_BORDER`를 제거하고 `WM_NCCALCSIZE`로 1px을 확보한 뒤 `WM_NCPAINT`에서 직접 그린다(+`SetWindowTheme`으로 테마 차단). 테두리가 원래 NC 영역 **안쪽**이었으므로 픽셀 위치가 이동하지 않는다 — 위 보류 사유는 패널·View에만 해당한다
- 후속: **패널·View 16곳은 그대로 남아 있다.** 부모가 컨트롤 바깥에 그리므로 옮기면 1px 이동한다. 옮기려면 좌표 보정과 화면 변화 검증이 함께 필요하다. `DrawEditBorder` 중복(2026-08-05 항목)도 이때 같이 없앤다

### [2026-07-31] 기존부채 — SageCoverPriceDlg 호출부 없음
- 위치: app/ui/dialogs/SagePriceSimpleDlg.h:44 및 대응 .cpp
- 설명: 클래스가 정의되어 있으나 프로젝트 어디서도 DoModal 호출이 없다. 3-A-2에서 버튼 교체 대상을 세다가 발견했다.
- 위험도: 낮음
- 후속: 실제 미사용이면 제거. 표지 단가 입력이 계획된 기능이면 진입 경로를 연결

### [2026-07-31] 구조불일치 — SagePriceSimpleDlg 파일명과 내용 불일치
- 위치: app/ui/dialogs/SagePriceSimpleDlg.h / .cpp
- 설명: 파일명은 PriceSimpleDlg인데 실제로는 SageCompanyRenameDlg와 SageCoverPriceDlg 두 클래스가 들어 있다. coding-design의 "파일 하나에 과도하게 많은 클래스를 넣지 않는다"와 어긋난다.
- 위험도: 낮음
- 후속: 클래스당 파일로 분리하고 파일명을 클래스명에 맞춤

### [2026-07-31] 구조불일치 — UI 계층이 infra를 직접 호출
- 위치: app/ui/view/SageTaechangView.cpp, app/ui/panels/SagePriceCalcPanel.cpp, app/ui/dialogs/{SageLoginDlg, SagePasswordChangeDlg, SageCalcEstimateDlg}.cpp
- 설명: coding-design의 의존 방향(ui → core ← infra)을 어기고 SageDBMgr/Repository를 직접 참조한다.
- 위험도: 중
- 후속: View 경로는 **3-B-6b**(`ISageWorkflowRunner` 도입, infra include 6줄 제거), 다이얼로그 3개는 **Step 4-B**로 갈렸다

### [2026-07-31] 구조불일치 — core Service 헤더가 infra Repository 헤더에 컴파일 의존
- 위치: app/core/auth/SageUserService.h:5, app/core/price/SagePriceService.h:5, app/core/receivable/SageReceivableCompanyOrderService.h:5
- 설명: Service 헤더가 Repository 헤더를 include해 core가 infra에 컴파일 타임으로 묶여 있다. core를 독립 라이브러리로 분리하거나 테스트를 붙일 때 걸림돌이 된다.
- 위험도: 중
- 후속: Step 4에서 core에 인터페이스를 두고 infra가 구현하도록 의존 역전

### [2026-07-31] 구조불일치 — infra/office가 infra/db를 직접 참조
- 위치: app/infra/office/SageReceivablesExcelService.cpp
- 설명: 문서 생성 모듈이 DB 매니저를 직접 호출한다. 같은 infra 계층 안이지만 관심사가 섞여 있다.
- 위험도: 낮음
- 후속: Step 4에서 core Service 경유로 전환

### [2026-08-07] 검증구멍 — 4d-3의 「짐 옮기기」를 소비자 관점에서 검증하지 않았다
- 위치: 확인 절차 자체. 4d-3(`5d0f7d5`)이 View 3,800줄 → 493줄로 줄이며 떨어뜨린 것 **2건**이 D7-4 도중에 드러났다
  - ① 시작 시 워크플로 라벨 적용 (`OnInitialUpdate`의 `UpdateWorkflowLabels()` 한 줄) → `ed68f21`
  - ② 표 변경·선택 통지 소비 (View의 `OnResultTableChanged` · `OnResultSelectionChanged`) → `7c03a39`. **패널은 계속 올려보내는데 받는 쪽이 없어져** 행을 선택해도 생성 버튼이 안 켜졌다
- 설명: 4d-3은 **「무엇을 옮겼나」는 전수 대조했지만(헤더 선언 대비 cpp 정의·메시지맵)** 「옮긴 뒤 그것을 **부르던 쪽과 받던 쪽**이 남아 있나」는 보지 않았다. 그래서 **호출부 없는 정의**가 아니라 **정의 없는 호출부**(①)와 **소비자 없는 발신**(②)이 남았고, 둘 다 컴파일·링크를 통과했다. 확인 목록도 「실행 · 완료 · 실패 · 초기화 · 드롭 · 워크플로 전환」이라 두 증상 다 안 걸렸다
- 위험도: 중 — 조용히 통과하는 종류이고, 3-B-5(사이드바 · 헤더 패널)에서 같은 이동을 또 한다
- 후속: **① 화면 Step의 확인 목록 첫 줄을 「앱을 새로 띄운 직후 화면」으로 고정한다** ② 메시지·핸들러를 옮기는 Step에서는 **통지마다 최종 소비자가 있는지 기계적으로 대조한다.** 2026-08-07 전수 점검 결과 남은 끊김은 없다(패널→워크스페이스 7건 · 워크스페이스→View 2건 · `WORKSPACE_STATUS` · `WORKFLOW_COMPLETE` 모두 수신처 있음)

## 해결됨

### [2026-08-07] 해결 — 아이콘 버튼 2곳이 규격 상수를 쓰지 않는다
- 등록: 2026-08-07 (D7-4 1단계) / 해결: 2026-08-20 (D7-12)
- 내용: 「아이콘 단독 버튼은 `SAGE_ICON_BUTTON_SIZE`(32) 정사각」인데 필터 초기화와 계산 초기화 두 곳이 각자 값을 쓰고 있었다.
- 해결: **둘 다 아이콘 단독 버튼이 아니게 되어 규칙의 대상에서 벗어났다.** 계산 초기화는 D7-12에서 목업 3-2대로 텍스트 버튼(카드 안쪽 하단)이 됐고 `SAGE_CALC_ICON_BTN_*` 3개는 제거했다. 필터 초기화는 이미 아이콘+텍스트다. 단가 계산에 남은 아이콘 단독 버튼은 「법인 찾기」 하나이고 `SAGE_ICON_BUTTON_SIZE` 정사각 + 툴팁을 지킨다.
- 교훈: **규격 위반이 「값을 맞춰서」가 아니라 「그 규격의 대상이 아니게 되어」 사라질 수 있다.** 후속을 「상수를 쓰게 바꾼다」로만 적어두면 화면이 바뀔 때 항목이 낡는다.

### [2026-08-06] 해결 — 단가 계산 입력 라벨에 6px 수동 보정이 남아 있다
- 등록: 2026-08-06 (D3b) / 해결: 확인 2026-08-20 (D7-12)
- 내용: 「부수」·「페이지」 라벨만 좌표를 6px 당기는 `SAGE_CALC_INPUT_LABEL_SHIFT`(=6)와 `SAGE_PRICE_COMPANY_LABEL_SHIFT`(=4)가 남아 있었다.
- 해결: **D7-12에서 전수 조사하니 두 상수가 코드에 이미 0곳이었다** — 어느 Step인지 기록 없이 사라졌고 이 항목만 남아 있었다. 항목이 낡은 것이다.
- 교훈: **부채 항목은 해결과 함께 닫지 않으면 유령이 된다.** 다음 사람이 없는 상수를 찾게 된다.

### [2026-08-06] 해결 — `SetColumns`가 확장 스타일을 통째로 덮어쓴다
- 등록: 2026-08-06 / 해결: 2026-08-09 (`fix/list-imagelist-ownership`)
- 내용: 매 호출마다 `LVS_EX_CHECKBOXES`가 잠깐 꺼지고 **그때 comctl32가 상태 이미지리스트를 자기 것으로 알고 파괴했다.** 매번 새로 만들어 `Detach`로 넘기는 우회책으로 막고 있었다.
- 해결: `SetColumns`가 `GetExtendedStyle()`을 보존한 채 필요한 플래그만 켜도록 바꿔 **꺼지는 경로 자체를 없앴다.** 그런데 그것만으로는 방향이 반대인 누수가 생긴다 — 스타일이 꺼지지 않으면 파괴도 없으므로, 재호출마다 새 이미지리스트를 만들어 교체하면 이전 핸들을 아무도 해제하지 않는다(`LVM_SETIMAGELIST`는 이전 것을 반환하고 정리는 애플리케이션 몫이다). 그래서 **`Detach` 우회책을 걷어내고 `CImageList m_imgCheckState`를 컨트롤 멤버로** 올렸다 — 한 번만 만들고 재사용하며, `SetCheckboxes(FALSE)`는 스타일을 끄기 **전에** `SetImageList(NULL, LVSIL_STATE)`로 먼저 분리한다.
- 교훈: **우회책은 전제가 바뀌면 반대 방향의 결함이 된다.** 「매번 새로 만든다」는 「매번 파괴된다」를 전제로 성립했고, 파괴를 막자 그 자리가 누수가 됐다. 원인을 고칠 때는 **그 원인을 전제로 세운 코드를 함께 찾아야 한다.** 함께 발견한 것 하나 더 — `m_imgRowSpacer`는 멤버로 소유하면서 리스트뷰에 붙인 채 두어 **comctl32와 `CImageList` 소멸자가 같은 핸들을 두 번 파괴**할 수 있었다. `OnDestroy`에서 두 슬롯을 모두 분리해 함께 닫았다.
- **함정 (CRITICAL): `LVS_EX_CHECKBOXES`를 켜기 전에 `LVSIL_STATE`를 붙이면 클릭 판정이 죽는다.** 누수를 피하려고 「붙이고 나서 스타일을 켠다」로 순서를 바꿨더니, 스타일이 꺼진 상태에서 붙은 상태 이미지리스트는 **리스트뷰가 영역 폭을 확보하지 않아** 판정이 0이 됐다. 그림은 `ImageList_GetIconSize`로 폭을 읽어 그리므로 **보이는데 눌리지 않는** 상태가 되고, 이 표가 원래 겪었던 버그와 증상이 같아 원인을 헷갈리기 쉽다. **스타일을 먼저 켜고, comctl32가 만든 기본 이미지리스트는 `ListView_SetImageList`가 돌려주는 이전 핸들로 파괴한다** — 순서로 누수를 피하려 하지 않는다.

### [2026-08-07] 해결 — D7-4 1단계가 빌드·화면 확인 없이 `develop`에 있다
- 등록: 2026-08-07 (D7-4 1단계) / 해결: 2026-08-07 (사용자 확인)
- 내용: 사용자 요청으로 확인 전에 머지했고, `CSageSectionLabel`을 통째로 카드 헤더 규격으로 바꿔 사용처 6곳이 동시에 변했다 — 입력 카드 · 결과 표 제목 · 실행 기록 · 데이터 관리 2곳 · 단가 계산 기록.
- 해결: **다음 작업(2단계) 착수 전에 확인을 요구했고, 사용자가 6곳 모두 확인했다.** 우려했던 「밴드가 흰 카드 위가 아닌 곳에서 떠 보인다」는 발생하지 않았다.
- 교훈: **미검증 커밋을 계획서 상단과 부채 로그 양쪽에 적어둔 것이 작동했다.** 다음 세션의 첫 항목이 「확인받기」가 되어 2단계를 미검증 위에 얹지 않았다.

### [2026-08-07] 해결 — 화면에 뜨지 않는 컨트롤 4개를 입력 패널로 그대로 옮겼다
- 등록: 2026-08-07 (3-B-4c) / 해결: 2026-08-07 (`35fb4e3`, D7-4 1단계)
- 내용: `m_wndWorkflowLabel` · `m_wndInputLabel` · `m_wndOutputLabel` · `m_wndLoad` 넷이 생성만 되고 화면에 나타나지 않았다. 3-B-4c가 「옮기기만 한다」였으므로 필요 여부 판단을 D7-4로 미뤘다.
- 해결: 후속에 적어둔 대로 **D7-4 1단계에서 목업 3-4를 실측해 갈랐다.** `m_wndInputLabel` · `m_wndOutputLabel`은 **살렸다** — 목업의 80px 폼 라벨 「입력 파일」·「저장 위치」가 바로 이 둘이다(`DESIGN_PLAN` D3b가 「80px는 현재 화면에 없는 라벨」이라고 적어둔 그 라벨). `m_wndWorkflowLabel`과 `m_wndLoad`(「미리보기」)는 **지웠다** — 목업에 없고 이미 항상 `SW_HIDE`였다. `m_wndLoad`만 쏘던 `OnLoadWorkflow`와 메시지맵 항목도 함께 걷어냈다.
- 교훈: **「지금은 판단하지 않는다」를 부채로 남길 때 판단할 Step을 지정한 것이 정확히 작동했다.** D7-4에 와서 목업을 재니 넷 중 둘은 살릴 것이고 둘은 죽은 것이었다 — 4c 시점에 추측으로 지웠으면 폼 라벨을 다시 만들었을 것이다.

### [2026-08-07] 해결 — `m_bRunning`이 View와 입력 패널에 이중으로 존재했다
- 등록: 2026-08-07 (4d-1 이전) / 해결: 2026-08-07 (4d-3)
- 내용: 실행 상태 소유자가 View인데 입력 패널도 사본을 들었다. 갈라지면 「실행 중인데 진행바가 안 보인다」가 되고 화면만 봐서는 원인을 못 찾는 종류였다.
- 해결: `SageWorkflowController`가 실행 상태를 **단독 소유**하게 됐다. 입력 패널의 `m_bRunning`은 진행바 타이머와 액션 영역 가시성 판단에만 쓰이는 **표시용 사본**으로 역할이 좁아졌고, 워크스페이스의 `SetRunningState` 하나가 유일한 진입점이다.
- 교훈: **후속에 「어느 Step에서 사라진다」를 적어두면 그 Step에서 실제로 회수된다.** 위험도 중이었지만 회수 지점이 명시돼 있어 두 Step을 안심하고 지날 수 있었다.

### [2026-08-07] 해결 — `SageWorkspaceVisibility`가 View와 워크스페이스 사이의 다리였다
- 등록: 2026-08-07 (4d-1) / 해결: 2026-08-07 (4d-3)
- 내용: 가시성 판정이 탭 선택(워크스페이스)과 실행 결과(View) 두 상태에 걸쳐 있어, View가 판정 결과 5개를 구조체로 묶어 넘겼다. 설계 단계에서 임시임을 예고했다.
- 해결: 실행 결과 상태가 컨트롤러로 가면서 워크스페이스가 **양쪽을 다 알게 됐다.** `RefreshVisibility()`가 자기 안에서 판정해 구조체를 만들므로 **View는 이 구조체를 모른다.** 구조체 자체는 워크스페이스 내부 전달 수단으로 남았고 계층을 넘지 않는다.
- 교훈: 임시 구조를 남길 때 **무엇이 갖춰지면 사라지는지**를 후속에 적어두면 판단이 반복되지 않는다.

### [2026-08-07] 해결 — 데이터 관리 패널이 워크스페이스 위에 겹쳐 그려졌다
- 등록: 2026-08-07 (4d-1) / 해결: 2026-08-07 (`0f17bc4`, 4d-2)
- 내용: 4d-1에서 패널 5종은 워크스페이스 하위로 갔는데 데이터 관리 컨트롤 13개는 View 자식으로 남아, 워크스페이스 **위에 겹쳐** 그려졌다. 생성 순서상 Z-order로 위에 와서 동작은 했다.
- 해결: 후속에 적어둔 대로 4d-2가 `SageCompanyOrderPanel`을 신설해 워크스페이스 하위로 넣었다. 겹침이 사라지고 탭 전환이 패널 하나를 보이고 나머지를 숨기는 것으로 통일됐다.
- 교훈: **부모를 두 번 바꾸지 않으려고 미룬 것이 한 Step 동안 Z-order 의존 상태를 만들었다.** R8이 예견한 비용이고, 그 사이 상태를 부채로 적어둔 것이 회수 지점을 놓치지 않게 했다.

### [2026-08-06] 해결 — 다이얼로그 라벨 색이 앱 안에서 갈려 있다
- 등록: 2026-08-06 (D7-9) / 해결: 2026-08-07 (`9753329`, D7-10)
- 내용: D7-9가 목업 6종의 라벨만 `CSageLabel`로 승격해서, 목업에 없는 견적서 생성·표지 단가 다이얼로그는 라벨이 본문색으로 남아 있었다.
- 해결: 후속에 적어둔 대로 **D7-10이 7종 전부를 만지는 김에 함께 승격**했다. 「날짜」·「품목명」·표지 라벨은 `SAGE_TEXT_MUTED`, 날짜 구분자 2개는 `SAGE_TEXT_SECONDARY`. 구분선 2개는 `CStatic`으로 남겼다 — 라벨이 아니라 선이고 `OnCtlColor`가 HWND로 브러시를 준다.
- 교훈: **후속에 「다음 Step에서 함께 한다」고 적었으면 그 Step을 끝낼 때 로그를 다시 본다.** D7-10 구현을 마칠 때까지 이 항목을 잊고 있었고, 완료 보고 직전에야 발견했다.

### [2026-08-05] 해결 — refactor/result-table-panel에 디자인 적용이 누락된다
- 등록: 2026-08-05 / 해결: 2026-08-06 (`e4268de` + 3-B-4a 재배선)
- 내용: `533ab0d`의 패널은 `813e0c1` 기준이라 D1a~D6 39커밋을 몰랐다. 머지되면 규격이 다른 표가 하나 생길 위험이었다.
- 해결: 되살릴 때 `SetRowSeparator`·`SetGroupColumn`·버튼 아이콘·툴팁을 반영했고(`e4268de`), 재배선 시점에 View와 1:1 대조해 남은 5건을 더 맞췄다 — 표 셀·헤더 폰트 `SAGE_FONT_LIST`(패널은 `CONTENT`였다), 초기화 버튼 폰트 `SAGE_FONT_HEADER`, 검색 버튼 variant 제거(패널이 `PRIMARY`를 줘서 화면당 Primary 1개 규칙을 깼다), 필터 콤보 항목 높이, 필터 입력 길이 제한. 체크박스의 `SetWindowTheme` 제거도 함께 — View에 없던 코드라 테마가 꺼져 모양이 달라졌다.
- 교훈: **폐기 브랜치를 되살릴 때 "가져왔다"로 끝내면 안 된다.** 되살린 파일과 현재 화면 코드를 항목별로 대조해야 차이가 드러난다. 커밋 `e4268de` 시점에는 3건만 찾았고 실제로는 8건이었다.

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
- 조치: CWinApp 헤더의 SageDBMgr.h include를 제거하고 실제 사용처인 SageTaechang.cpp에 직접 추가했다. 이 과정에서 SageTaechangView.h가 SageTaechang.h → SageDBMgr.h → SagePriceRepository.h 사슬로 SagePriceDto를 간접 획득하고 있었음이 드러나, 전방 선언과 직접 include로 정리했다.
- 커밋: 19d95eb, 21d5679

### [2026-07-31] 해결 — SqlInitializer 파일명과 클래스명 불일치
- 위치: app/infra/db/SqlInitializer.h
- 조치: 클래스명 SQLInitializer를 SqlInitializer로 변경 (13곳). coding-rules의 약어 표기 규칙에 맞춤.
- 커밋: 6dccd52

### [2026-07-31] 철회 — SageDBMgr Getter 7개가 포인터 반환
- 위치: app/infra/db/SageDBMgr.h:36~45
- 사유: 부채가 아니라 **규칙 쪽을 바꿨다.** coding-rules의 "함수 반환 타입에 포인터 사용 금지"를 제거하고, raw pointer 반환을 허용하되 계약을 규약으로 고정하는 방식으로 전환했다 (기본 비소유 / `Get*`은 항상 유효·`Find*`는 NULL 가능 / 소유권 이전은 Create·Detach·Release·Post 네이밍 / 필수 내부 객체는 참조 우선하되 기존 MFC 패턴은 포인터 유지 허용).
- 결과: 현재 코드는 `Get*` + 항상 유효 + 호출부 미검사로 새 규약과 이미 정합하다. 코드 변경 없음.
