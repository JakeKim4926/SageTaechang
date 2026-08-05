# SageTaechang 구조 리팩토링 계획

> 작성·갱신 규칙은 `sagetaechang-plan` skill을 따른다.
> Step 완료 시 체크박스와 상태 표를 갱신하고, 상세 체크리스트는 지우고 결과·교훈만 남긴다.
> **2026-08-05 전면 재작성** — 스킬에 *ui 계층 구성* 규칙이 신설되어 목표 상태와 순서를 다시 잡았다.
> **2026-08-05 기준 재정의** — 완료 기준을 줄 수 지표에서 **책임 기준(A~E)**으로 바꿨다.
> 줄 수는 증상 지표로 내렸고, 같은 기준을 새로 만드는 패널에도 적용한다.

## Context

`SageTaechangView.cpp`가 **3,822줄, 함수 146개**다. 시작 시점 4,497줄에서 675줄 줄었지만
구조 문제는 그대로다. 원인은 두 겹이다.

**1. 확장점 부재** — 워크플로가 늘 때마다 View의 조건 분기를 고쳐야 했다 (Step 4로 해소 중).

**2. 구성 규칙 부재** — 이게 더 큰 원인이었다. 스킬에는 *금지 규칙*(워크플로 분기 금지,
컨트롤이 자기를 그린다)만 있었고 **무엇이 독립 클래스여야 하는지**가 없었다.
그래서 금지 목록에 없는 것들이 전부 합법적으로 View에 쌓였다.

| View가 지고 있던 책임 | 규모 |
|---|---|
| 컨트롤 소유 | 멤버 **104개** |
| 메시지 라우팅 | 맵 **50항목**, 핸들러 49개 |
| 레이아웃 계산 | **1,032줄** |
| 워크플로 실행 조정 | 워커 시작 · 진행 · 완료 |
| 화면 상태 보관 | `TaechangWorkflowUiState` 멤버 3개 |
| 배경 그리기 | `OnDraw` · `OnEraseBkgnd` |
| 업무 탐색 | 사이드바 구성 · 선택 처리 |
| 인증 상태 표시 | 로그인 · 사용자명 |

2026-08-05에 `coding-design`에 *ui 계층 구성*, `sagetaechang-ui`에 *MFC 화면 패널 규격*을 신설하고
패널 방식을 **비윈도우 헬퍼 → `CWnd` 파생**으로 전환했다. 이 계획은 그 규칙으로 다시 작성한 것이다.

---

## 목표 상태

```
CSageTaechangView                    조립 · 최상위 rect 배분 · 활성 패널 교체 · 앱 이벤트 연결
  ├ SageSidebarPanel                 업무 탐색
  ├ SageHeaderPanel                  제목 · 상태 · 인증 표시
  └ SageWorkspacePanel               탭 + 활성 업무 패널 교체
       ├ SageWorkflowPanel           문서 워크플로 공용 + 확장 탭 host
       │    └ SageCompanyOrderPanel  미수금 확장 탭
       ├ SagePriceManagePanel
       └ SagePriceCalcPanel

app/core/workflow/                   핸들러 · 등록부 (Step 4)
SageWorkflowController               실행 상태 전이 · 워커 수명
```

**도달 기준은 책임이다** (`coding-design` > *완료 기준은 책임이다*)

이 작업의 목표는 View를 짧게 만드는 것이 **아니다.** 한 클래스가 여러 책임을 지지 않게 하는 것이다.
줄 수를 목표로 두면 짐을 패널 하나로 옮겨 View만 얇아지는 통과가 가능하고,
그때 과중화된 클래스는 이름만 바뀐 채 옆으로 옮겨간다.

**A. 파일이 열리는 이유가 하나인가**

- [ ] `CSageTaechangView` — **화면 조립이 바뀔 때** 하나 (현재 8개, 위 책임 표)
- [ ] 각 패널 — 만든 직후 같은 기준으로 점검한다. 이유가 둘 이상이면 그 자리에서 분할을 판단한다

**B. 무엇을 알아야 하는가** — View는 워크플로 종류 · SQL · 그리는 방법 · 픽셀 좌표를 모른다

- [ ] `View.cpp`에 워크플로 타입 분기 0곳 (현재 36곳, 사이드바 등록 데이터 제외)
- [ ] `View.cpp`에 `#include "app/infra/..."` 0줄 (현재 6줄)

**C. 컨트롤 · 메시지맵 · 레이아웃이 한 클래스에 함께 있는가**

- [ ] 각 패널이 자기 컨트롤 · 메시지맵 · 내부 레이아웃을 소유

**D. 하나 추가할 때 고칠 지점**

- [ ] 워크플로 1종 = `handlers/` 파일 1쌍 + 등록부 1곳
- [ ] 화면 영역 1개 = 패널 1개 + View의 rect 배분 1줄

**E. 위임 스텁 0개**

- [ ] View가 패널 대신 받아 넘기는 핸들러가 없다

**증상 지표** — 넘어야 할 선이 아니라 A~E를 통과했는데 값이 크면 못 본 책임이 있다는 신호다.
수치를 맞추려고 코드를 옮기지 않는다.

| 지표 | 현재 | 참고값 |
|---|---|---|
| View 업무 컨트롤 멤버 | 104 | 0 |
| View 메시지맵 항목 | 50 | 10 이하 |
| View 줄 수 | 3,822 | 600 내외 |

---

## 진행 상황

| Step | 내용 | 상태 |
|---|---|---|
| 0 | 스킬 정비 (계층 · 컨트롤 규격) | **완료** |
| 0-B | 스킬 보강 (ui 계층 구성 · 패널 규격 · 패널 네이밍) | **완료** 2026-08-05 |
| 1 | 파일을 `core`/`infra`/`ui`로 이동 | **완료** |
| 2 | 잔여 정리 (앱 헤더 DB 의존, `SqlInitializer`) | **완료** |
| 3-A | 공통 컨트롤 승격 | **완료** |
| — | 검수 워크플로(PDF·HWP) 제거 | **완료** `b2e8169` |
| 4 | 워크플로 핸들러 + 등록부 | **진행 중** — 4-1~4-6a 완료 |
| **3-B** | **화면 패널 분리 (8단계)** | **다음** — 책임을 실제로 떼는 단계 |
| 4-B | 의존 역전 (core Service ↔ infra Repository) | 대기 |
| 5 | `Sage` 접두사 전환 (상수 648개) | 대기 |

### Step 간 의존

```
3-A ──→ 3-B      컨트롤이 스스로 그리므로 부모가 View든 패널이든 무관하다.
                 CWnd 파생의 주된 위험(오너드로우 라우팅)이 사라진 뒤에야 패널 승격이 가능하다

3-B-1·2 ⊥ 4      단가 패널은 워크플로와 무관하다(전환 시 조기 반환). 지금 당장 착수 가능

4-6b·4-6c·4-7 ──→ 3-B-4
                 결과 표 · 필터 · 응답 표시 코드가 SageWorkflowPanel로 이동한다.
                 먼저 분기를 핸들러로 걷어낸 뒤 옮겨야 패널 안에서 다시 걷어내지 않는다

3-B-4 ──→ 3-B-6a 워커 결과를 받을 주체가 패널 HWND가 되므로
                 패널이 자리잡은 뒤에 실행 축을 뗀다

3-B-6a ──→ 3-B-6b 컨트롤러 추출은 실행 상태의 소유자를 바꾸고, 의존 역전은 infra 호출자를 바꾼다.
                 이유가 다르므로 나눠 진행한다. R7이 6a에 있어 실패 원인이 섞이지 않아야 한다

5   ←── 3-B · 4  새로 쓴 코드도 접두사 치환 대상이다
```

**순서 결정 근거**: Step 4는 View의 책임을 **하나도 떼지 않는다.** 워크플로 고유 지식을 핸들러로
옮기는 작업이라 기준 B(무엇을 알아야 하는가)는 나아지지만, 컨트롤 소유 · 메시지맵 · 레이아웃은
그대로 남는다 (그래서 4-5는 View가 오히려 늘었다). 책임을 실제로 떼는 것은 3-B뿐이다.

3-B-1을 첫 순서로 두는 이유는 크기가 아니라 **R5(레이아웃 좌표계 전환) 검증**이다.
모든 레이아웃이 View 클라이언트 좌표 기준이어서 패널 기준으로 바뀌는 순간 전부 어긋날 수 있고,
그 위험을 워크플로와 무관한 패널 하나로 먼저 확인해야 나머지에 적용할 수 있다.
워크플로 축(Step 4 잔여)은 그 결과를 기다리는 동안 마무리한다.

---

## 완료된 작업

**Step 0 — 스킬 정비**
`coding-rules`(어떻게 쓰나) / `coding-design`(어디에 만드나) 분리, 타 프로젝트 잔재 제거,
포인터 반환 계약 규약화(`Get*` 항상 유효 / `Find*` NULL 가능), MFC 공통 컨트롤 규격 신설.

**Step 0-B — 스킬 보강** (2026-08-05)
`coding-design`에 *ui 계층 구성* 신설 — 화면 클래스 구성, 소유 규칙 4개, View의 역할 4가지,
측정 가능한 완료 기준, 패널 단위 수직 분리. `sagetaechang-ui`에 *MFC 화면 패널 규격* 신설.
`coding-rules` 네이밍 표에 `CWnd` 파생 패널(`Sage*Panel`, `C` 없음) 추가.
**교훈: 금지 규칙만으로는 비대화를 막지 못한다. 무엇이 독립 클래스인지를 규정해야 한다.**

**Step 1 — 계층 재배치** (`1bcf839`~`9e91f9f`)
`core`(MFC 비의존) / `infra` / `ui` 3계층. 이동 커밋 0 insertions · 0 deletions.

**Step 2 — 잔여 정리** (`19d95eb`~`3e09d87`)
앱 헤더의 DB 의존 제거, `SQLInitializer` → `SqlInitializer`.

**Step 3-A — 공통 컨트롤 승격** (`c2696c3`~`260d1dd`)
컨트롤 8종(`CSageButton` · `CSageLabel` · `CSageListCtrl` · `CSageSidebarTree` 등)이 리플렉션으로
자기를 그린다. View의 `OnDrawItem`이 사라졌고 `OnCtlColor`는 112 → 31줄.
`SageUiResources`(폰트 · 브러시)와 `SageUiStyle`(공유 조각 1개) 도입.
**교훈: 줄 수는 회수의 척도가 아니다.** 3-A-8은 분기 119줄을 지우고 역할 지정 113줄을 새로 썼다.
얻은 것은 "View가 판단한다"에서 "컨트롤이 선언한다"로 바뀐 것이다.

**검수 워크플로 제거** (`b2e8169`, 20파일 −1,295줄)
PDF · HWP 표지 검수가 실제로 쓰지 않는 기능임이 확인되어 제거했다. 사이드바에 진입점이 없어
화면 변화가 없었다. `IsCompareWorkflow` 15곳 소멸, 탭 상수 값 충돌 부채 해소.
**교훈: 삭제는 단계를 나눠도 중간 커밋이 빌드되지 않을 수 있다.** 조건식을 지울 때 뒤에서 쓰는
지역변수 선언까지 함께 지웠다. 결국 5커밋을 1커밋으로 squash했다.

**Step 4-1~4-6a — 워크플로 핸들러** (`92468e4`~`07f043b`)
`ISageWorkflowHandler`가 라벨 4종 · 탭 목록 · 결과 컬럼/표시 속성 · 입력 정책 · 선택 행 검증을 답한다.
핸들러 3쌍(미수금 · 납품 · 견적) + 등록부 1곳. View 조회는 `FindCurrentHandler()`로 모았다(`e8a0c56`).
**교훈: 착수 전 매핑 표를 확정하고 상수까지 대조한 단계는 전부 한 번에 통과했다.**

---

## 미해결 리스크

| # | 내용 | 영향 | 대응 |
|---|---|---|---|
| R5 | **레이아웃 좌표계 전환** — 현재 모든 레이아웃이 View 클라이언트 좌표 기준. 패널로 옮기면 패널 기준이 된다 | 패널 내부 컨트롤 위치가 통째로 어긋날 수 있다 | 3-B-1(단가 계산)로 먼저 검증. 실패 시 그 패널만 되돌린다 |
| R6 | **notification 라우팅 변경** — `NM_CUSTOMDRAW`는 컨트롤이 리플렉션으로 처리하지만 `TVN_SELCHANGED` · `LVN_ITEMCHANGED` · `CBN_SELCHANGE`는 부모가 받는다. 부모가 View → 패널로 바뀐다 | 핸들러가 안 불리면 선택 · 체크 동작이 죽는다 | 패널 이동 전에 해당 notification 목록을 세고 함께 옮긴다 |
| R7 | **워커 결과 수신처 변경** — `PostMessage(WM_TAECHANG_WORKFLOW_COMPLETE)` 대상이 View HWND다 | 전환 중 결과가 유실되면 진행바가 95%에서 멈춘 채 남는다 | 3-B-6a에서 한 번에 전환. 그 전까지는 View가 받아 패널로 넘긴다 |
| R8 | **미수금 데이터 관리 탭의 부모** — 법인 발주는 워크플로 패널의 확장 탭이다 | 독립 패널로 먼저 만들면 부모를 두 번 바꾸게 된다 | 3-B-4에서 워크플로 패널과 함께 처리한다 |
| R9 | **헤더 상태 컨트롤 처리 미결정** — `m_wndHeaderStatus`가 표시된 적이 없는데 멤버 2개 · 함수 2개 · 분기 · 상수 3개가 딸려 있다 | 확인 없이 옮기면 죽은 코드가 새 패널로 복제된다 | 3-B-5b 착수 전에 숨김이 의도인지 확인한다. 미완성이면 표시 복구, 불필요하면 관련 코드 일괄 제거 |

열린 기술부채는 `docs/DEBT_LOG.md` 13건.

---

## Step 3-B — 화면 패널 분리

규칙: `coding-design` > *ui 계층 구성*, `sagetaechang-ui` > *MFC 화면 패널 규격*

**분리는 패널 단위 수직으로 한다.** 한 패널의 컨트롤 · 레이아웃 · 메시지맵을 한 번에 옮긴다.
축을 나눠 옮기면 위임 스텁을 만들었다 지우는 중간 상태가 생긴다.

### 공통 절차 (모든 하위 단계에 적용)

- [ ] 1. 옮길 대상을 센다 — 컨트롤 멤버 / 메시지맵 항목 / 함수 / 줄 수
- [ ] 2. 부모가 받던 notification 목록을 센다 (R6)
- [ ] 3. `app/ui/panels/`에 `Sage*Panel` 생성 + vcxproj · filters 등록
- [ ] 4. 컨트롤 멤버 · 생성 · 레이아웃 · 메시지맵 · 핸들러를 **한 커밋으로** 이동
- [ ] 5. View에서 해당 코드 제거, 패널 생성과 `Layout(rect)` 호출만 남김
- [ ] 6. **만든 패널에 기준 A를 적용한다** — 이 패널이 열리는 이유가 하나인가.
      둘 이상이면 분할 여부를 그 자리에서 판단한다 (View가 얇아진 것은 완료 근거가 아니다)
- [ ] 7. 빌드 → 화면 확인 → 머지

### 3-B-1 — 단가 계산 화면 (1호, 패턴 검증)

**1호로 고른 이유**: 워크플로 전환에서 조기 반환으로 갈려 있어 문서 업무에 영향이 없다.
집계에서 계산 함수의 워크플로 상수 참조가 **0곳**임을 확인했다. 실패해도 반경이 이 화면 하나이고,
새 패널 패턴(생성 · 레이아웃 · 메시지맵 · 좌표계)을 전부 검증한다.

**집계 결과** (2026-08-05, `SageTaechangView` 기준)

| 항목 | 수량 |
|---|---|
| 컨트롤 멤버 | 25 (View 104 중) |
| 비컨트롤 상태 멤버 | 7 — `CRect` 2 · 내역 배열 1 · 금액 3 · 포맷 가드 1 |
| 메시지맵 항목 | 8 (View 50 중) |
| 전용 함수 | 18개 / 561줄 |
| 공유 함수 잔재 | 81줄 — `ApplyLabelRoles` 30 · `OnDraw` 15 · `OnEraseBkgnd` 14 · `ApplyControlFonts` 11 · `PreTranslateMessage` 7 · 생성·배치 4 |

**기준 A로 센 변경 이유는 3개였고 하나가 화면 소관이 아니다** — `UpdateCalcPreview`(86줄)가
단가 조회(`sageDBMgr` 직접 호출 2곳)와 금액 계산을 함께 한다. 그래서 1a / 1b로 나눈다.

### 3-B-1a — `SagePriceCalcService` 추출

브랜치: `refactor/price-calc-service`

- [ ] `app/core/price/SagePriceCalcService.h/.cpp` 신설 + vcxproj · filters 등록
- [ ] 부수·페이지 범위 검증, 단가 조회, 인쇄비·소계·합계, 운임 클램프를 서비스로
- [ ] View 멤버 3개(`m_nCalcPrintPrice` · `m_nCalcCoverPrice` · `m_nCalcUnitPrice`) → `SagePriceCalcResult` 1개
- [ ] 화면 확인: 실패 5종의 **메시지와 아이콘** 동일, 정상 계산 3항목 + 합계, 운임 변경, 견적 저장 후 내역 추가

**실패를 `BOOL` + `outFailure` enum으로 나눈 이유**: 실패마다 아이콘이 다르다
(범위 초과 `MB_ICONWARNING` / 조회 실패 `MB_ICONERROR`). `strError` 하나로 합치면 아이콘이
통일되어 화면이 바뀐다. 문자열·아이콘 매핑은 화면 소관이므로 UI에 남긴다.

**헤더는 `TaechangPriceService` 전방 선언만 둔다.** `TaechangPriceService.h`가 infra Repository
헤더를 물고 있어(기존 부채) include하면 그 사슬이 번진다.

**범위 밖**: 입력 파싱(빈 문자열 · 숫자 변환)과 메시지·아이콘 매핑은 UI에 남긴다.
`sageDBMgr` include는 서비스 인스턴스를 얻는 경로라 남는다 — 앱 전체 패턴이므로 4-B · 3-B-6b 소관.

### 3-B-1b — `SagePriceCalcPanel` 이관

브랜치: `refactor/price-calc-panel`

- [ ] `SagePriceCalcPanel` 생성 — `CWnd` 파생 + 자체 메시지맵 8항목
- [ ] 컨트롤 25개 · 전용 함수 18개 · 공유 함수 잔재 81줄 이동
- [ ] `FormatPrice`(`View.cpp:2094`) · `PriceTextToInt`(`:2110`)를 `app/common/SageNumberFormat`으로 승격
      (3-B-2의 단가 관리 패널도 쓴다)
- [ ] View는 패널 생성과 `Layout(rect)`만 유지
- [ ] 기준 A 재점검 — 이 패널을 고치게 만드는 변경 원인이 하나인가
- [ ] 화면 확인: 단가 계산 진입 · 법인 선택 · 계산 · 미리보기 · 견적 저장 · 계산 내역

**패널이 재현할 것**: `OnCtlColor`는 계산 패널 전용 분기가 **0개**이고 일반 폴백
(`CSageLabel` 조기 반환 · `CTLCOLOR_EDIT`→PANEL)만 필요하다. `PreTranslateMessage`는
MFC가 `WalkPreTranslateTree`로 대상 창에서 부모 방향으로 호출하므로 패널이 View보다 먼저 받는다.

**마주칠 부채 — 계산 내역 리스트 커스텀드로우.** `m_wndCalcHistoryList`는 메시지맵 등록이 없어
스타일이 적용된 적이 없다(`DEBT_LOG` 기록됨). 패널로 옮기면서 등록하면 짝수/홀수 배경색과
첫 컬럼 정렬이 새로 적용되어 **화면이 바뀐다. 미등록 상태를 그대로 재현한다.**

**마주칠 중복 — 에디트 테두리 4벌.** 계산 에디트 4개의 `DrawEditBorder` 호출이 `OnDraw`에 2벌
(`:1029`, `:1052`), `OnEraseBkgnd`에 2벌(`:1665`, `:1684`) 있다. `:1029` 벌은 이후 카드
`FillSolidRect`에 덮여 무효다. 이관 시 rect 포함 관계를 확인하고 필요한 벌만 옮긴다.

**완료 기준**: View에서 단가 계산 컨트롤 멤버 0개, 관련 메시지맵 0항목, 화면 무변화.

### 3-B-2 — `SagePriceManagePanel`

브랜치: `refactor/price-manage-panel`

- [ ] 3-B-1b에서 확정된 패턴 그대로 적용
- [ ] `PriceTextToInt` 복제(`TaechangPriceSimpleDlg.cpp:60`)를 `SageNumberFormat`으로 통일
- [ ] `LayoutPriceManagePanel`(119줄) 포함 이동
- [ ] 기준 A 점검
- [ ] 화면 확인: 단가 데이터 관리 목록 · 추가 · 수정 · 삭제 · 범위/간편 다이얼로그

**마주칠 부채 — `m_wndPriceCompanyLabel` 배경색.** 이 패널의 라벨 중 이것만 `OnCtlColor` 분기가
없어 APP 배경으로 그려진다(버그로 확인됨, `DEBT_LOG` 기록됨). **이관은 현행(APP)대로 재현하고,**
`SetBackgroundRole(SAGE_BG_PANEL)` 수정은 화면이 바뀌는 변경이라 별도 커밋으로 분리한다.

### 3-B-3 — 워크플로 축 마무리 (Step 4 잔여 삽입)

패널로 옮기기 전에 결과 표 · 필터 · 응답 표시의 워크플로 분기를 걷어낸다. 상세는 *Step 4* 참조.

- [ ] 4-6b 필터 축
- [ ] 4-6c 상태 판정
- [ ] 4-7 응답 표시

### 3-B-4 — `SageWorkflowPanel` + `SageWorkspacePanel` + `SageCompanyOrderPanel`

브랜치: 하위 단계마다 별도

**착수 시 `SageWorkflowPanel`의 분할 여부를 먼저 판단한다.** 현재 계획대로 입력 · 실행 · 결과 ·
실행 기록을 한 패널에 두면 View의 책임 8개 중 컨트롤 소유 · 메시지맵 · 레이아웃 · 실행 조정 ·
상태 보관 **5개를 그대로 물려받는다.** 지표는 통과하고 과중화는 이동만 한다.
지금 미리 쪼개지 않는 이유는 근거가 없기 때문이다 — 착수 시 기준 A로 "무엇이 바뀌면 이 패널을
고치는가"를 세어보고, 입력과 결과가 서로 다른 이유로 바뀌면 그때 분할한다.

- [ ] 기준 A 집계 — 이 패널을 고치게 만드는 변경 원인 나열, 분할 여부 결정
- [ ] `SageWorkspacePanel` 생성 — 탭 컨트롤과 활성 업무 패널 교체를 가져간다
- [ ] `SageWorkflowPanel` 생성 — 문서 워크플로 3종 **공용**. 내부 구성은 위 판단 결과에 따른다
- [ ] `SageCompanyOrderPanel`을 워크플로 패널의 **확장 탭**으로 배치 (R8)
- [ ] 3-B-1 · 3-B-2 패널을 워크스페이스 하위로 재배치
- [ ] 화면 확인: 워크플로 3종 × 탭, 데이터 관리 탭, 워크플로 전환 시 상태 유지

**완료 기준**: 워크플로를 하나 추가할 때 패널을 고칠 필요가 없다
(탭 · 라벨 · 컬럼 · 입력 정책은 핸들러가 답한다).

### 3-B-5a — `SageSidebarPanel`

브랜치: `refactor/sidebar-panel`

- [ ] 사이드바 트리 구성 · 선택 처리 이동 (워크플로 상수는 등록 데이터로 함께 이동)
- [ ] 화면 확인: 업무 전환 · 선택 표시 · 트리 스크롤 없음 유지

### 3-B-5b — `SageHeaderPanel`

브랜치: `refactor/header-panel`

- [ ] 헤더 제목 · 상태 · 인증 표시 이동
- [ ] 화면 확인: 로그인/로그아웃 · 사용자명 표시 · 비밀번호 변경 진입

**착수 전 결정 필요 (R9)**: `m_wndHeaderStatus`는 `WS_VISIBLE`이 없고 레이아웃에서 `0,0,0,0`을
받아 **표시된 적이 없다.** 그런데 이 컨트롤을 위해 멤버 2개 · 함수 2개 · `OnCtlColor` 분기 · 상수 3개가
유지되고 있다. 숨김이 의도인지 미완성인지 확인하지 않고 옮기면 **죽은 코드를 새 패널로 복제한다.**

**5a와 5b를 나눈 이유**: 업무 탐색과 인증·상태 표시는 서로 다른 이유로 바뀐다. 기준 A로 별개이고
`git-workflow`의 브랜치 단위 원칙에도 맞는다.

### 3-B-6a — `SageWorkflowController`

브랜치: `refactor/workflow-controller`

- [ ] 실행 상태 전이(대기 → 실행 → 완료/실패)와 워커 수명을 컨트롤러로
- [ ] 워커 결과 수신처를 패널 HWND로 전환 (R7)
- [ ] 화면 확인: 워크플로 3종 실행 · 진행바 · 완료/실패 상태 · 연속 실행

**완료 기준**: 컨트롤러가 컨트롤 API(`SetWindowText` / `InsertItem` 류)를 호출하지 않는다.

### 3-B-6b — 실행 축 의존 역전 (Step 4-8)

브랜치: `refactor/workflow-runner`

- [ ] `ISageWorkflowRunner` 도입, infra 서비스 3개 구현
- [ ] View · 패널의 infra include 6줄 제거
- [ ] 화면 확인: 워크플로 3종 실행 결과가 이전과 동일

**완료 기준**: `View.cpp`에 `app/infra` include 0줄.

**6a와 6b를 나눈 이유**: 6a는 실행 상태의 **소유자**를 바꾸고 6b는 infra **호출자**를 바꾼다.
R7(워커 결과 수신처 전환)이 6a에 있어, 한 단계에 묶으면 결과 유실이 컨트롤러 문제인지
Runner 문제인지 가릴 수 없다.

---

## Step 4 — 워크플로 핸들러 (잔여)

규칙: `coding-design` > *ui 계층 구성*, `sagetaechang-ui` > *컨트롤은 도메인 개념을 알면 안 된다*

완료분은 *완료된 작업* 참조. 남은 넷은 3-B-3과 3-B-6b에 삽입해 진행한다.

| # | 내용 | 잔존 분기 | 배치 |
|---|---|---|---|
| 4-6b | 필터 기준 목록, `IsDocumentResultFilterVisible` | 7곳 | 3-B-3 |
| 4-6c | `IsDocumentWorkflowStateTarget` → 핸들러 존재 여부 | 3곳 | 3-B-3 |
| 4-7 | `DisplayResponse`, 표 술어 3개 | 11곳 | 3-B-3 |
| 4-8 | 실행 축 + infra 역전 | 5곳 + include 6줄 | 3-B-6b |

### 4-6b — 필터 축

- [ ] `SageWorkflowFilterCriteria { int nCriteria; LPCWSTR pszLabel; }` 순서 배열 도입
- [ ] `GetDefaultFilterCriteria`(첫 항목) · `GetEffectiveFilterCriteria`(목록에 있는가) ·
      `PopulateResultFilterCriteria`(목록 순회)를 배열 조회로 대체
- [ ] `UsesCustomResultTable(nTaskType)` 노출로 `IsDocumentResultFilterVisible` 단순화

**확정된 매핑**: 미수금 = 법인명 → 담당자 → 품목명 / 납품 · 견적 = 품목명 → 법인명.
기본값은 각 목록의 첫 항목이다.

### 4-6c — 상태 판정

- [ ] `IsDocumentWorkflowStateTarget(nType)`을 `FindHandler(nType) != NULL`로 대체하고 함수 제거

**범위 밖 — `GetWorkflowUiState`**: 워크플로별 상태 멤버 3개를 고르는 함수다. 핸들러는
정적 인스턴스 3개짜리 **무상태 싱글턴**이라 화면 상태를 들리면 전역 상태가 된다.
상태 보관은 3-B-6a에서 컨트롤러/패널로 간다.

### 4-7 — 응답 표시

- [ ] `DisplayResponse`의 워크플로 분기 8곳을 핸들러로
- [ ] View의 표 술어 3개(`IsReceivablesResultTable` · `IsDeliveryInputTable` · `IsEstimateInputTable`) 제거
      → `DEBT_LOG`의 *결과 표 판정 규칙 이중화* 해소

### 4-8 — 실행 축 + infra 역전

3-B-6b로 진행한다. 위 3-B-6b 체크리스트 참조.

### 완료 기준

- [ ] `View.cpp`에 `TAECHANG_WORKFLOW_RECEIVABLES` · `_DELIVERY` · `_ESTIMATE` 사용 **0곳**
      (사이드바 등록 데이터는 제외)
- [ ] `View.cpp`에 `#include "app/infra/..."` **0줄**
- [ ] 워크플로 추가 = `handlers/`에 파일 1쌍 + 등록부 1곳 수정

### 범위 밖

- **워크플로 6 · 7(단가관리 · 계산)** — 패널이지 워크플로 실행이 아니다. 핸들러를 만들지 않는다
- **사이드바 등록 데이터** — `BuildSidebarTree`가 워크플로 상수를 `SetItemData`에 싣는다.
  분기가 아니라 등록이므로 유지하고, 3-B-5a에서 사이드바 패널로 함께 옮긴다

---

## Step 4-B — 의존 역전

`DEBT_LOG` 3건 중 Step 4에서 해소되지 않는 2.5건.

- [ ] core Service 헤더가 infra Repository 헤더에 컴파일 의존 (auth · price · receivable 3개)
- [ ] `infra/office`가 `infra/db`를 직접 참조 (`TaechangReceivablesExcelService`)
- [ ] 다이얼로그 3개가 `SageDBMgr`을 직접 호출 (단가 · 인증 경로)

Step 4에서 `core`가 인터페이스를 정의하는 패턴이 자리잡은 뒤에 한다.

---

## Step 5 — `Sage` 접두사 전환

브랜치: `refactor/sage-prefix-constants`

- [ ] `TaechangDefine.h` → `SageDefine.h` (`git mv`)
- [ ] 상수 `TAECHANG_` → `SAGE_` (정의 **648개** + 전 소스 참조)
- [ ] `#include` 참조, vcxproj / filters 갱신
- [ ] `coding-rules`의 공용 상수 접두사 규칙을 `SAGE_`로 수정
- [ ] 치환은 **파일로 저장한 Python 스크립트**로 (heredoc은 백슬래시를 먹는다)

**범위 밖**: 나머지 `Taechang*` 클래스와 파일명. 해당 파일을 구조적으로 손댈 때 함께 전환한다.

---

## 검증 방법

1. **빌드는 사용자가 직접 확인한다** (Claude는 MSBuild를 실행하지 않음)
2. **모든 단계에서 화면 표시가 바뀌면 실패다.** 리팩토링이므로 이전과 동일해야 한다
3. 패널 분리는 **패널 단위로** 확인한다 — 해당 화면의 진입 · 입력 · 실행 · 결과 · 전환
4. 워크플로 축은 워크플로 3종의 불러오기 · 생성을 각각 1회씩
5. Step 5는 빌드가 통과하면 사실상 검증 완료 (기계적 치환)

`git-workflow` 준수: 하위 단계마다 별도 브랜치, 커밋은 목적 단위 분리,
`docs/decisions/PR_LOG.md`에 `docs:` 커밋으로 기록.

### 작업 중 사고 방지 체크 (실제로 겪은 것들)

- 조건식을 지울 때 **뒤에서 쓰는 지역변수 선언까지 지우지 않았는지** 확인한다
- 문자열 일괄 치환 후 **정의부를 눈으로 확인한다** (헬퍼가 자기 자신을 호출하게 된 적 있다)
- Python 스크립트는 **파일로 저장해 실행한다.** Bash heredoc은 백슬래시를 소실시킨다
- 편집 후 **UTF-8 BOM + CRLF**가 유지됐는지 확인한다
