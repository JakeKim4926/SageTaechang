# SageTaechang 구조 리팩토링 계획

> 작성·갱신 규칙은 `sagetaechang-plan` skill을 따른다.
> Step 완료 시 체크박스와 상태 표를 갱신하고, 상세 체크리스트는 지우고 결과·교훈만 남긴다.

## Context

`SageTaechangView.cpp`가 4,497줄(전체의 29%, 함수 146개)로 비대해졌다.
원인은 아키텍처 부재가 아니라 **확장점 부재**다. 같은 병증이 두 축에서 나타난다.

- **워크플로 축** — 워크플로가 7개까지 늘었는데 타입별 조건 분기가 여러 함수에 흩어져 있다
- **UI 축** — 버튼·라벨 스타일을 부모 `OnDrawItem`에서 **컨트롤 ID를 나열해** 가른다

둘 다 "하나 추가할 때마다 기존 코드를 고쳐야 하는" 구조다.

근본 원인은 스킬에 있었다. `coding-rules`/`code-review-expert`가 다른 프로젝트(SageNexus,
WebView2 기반) 문서였고 존재하지 않는 폴더 구조를 규정했다. 흉내내다 만 흔적이 이중구조로
남았고, 새 파일을 어디 둘지 판단이 서지 않으니 결국 View로 몰렸다.

**목표**: 계층 경계와 확장점을 만들어, 워크플로·UI 요소를 추가할 때
**새 파일과 등록부 1곳만 손대고 기존 로직은 고치지 않아도** 되게 한다.

---

## 진행 상황

| Step | 내용 | 상태 |
|---|---|---|
| 0 | 스킬 정비 | **완료** (로컬, 커밋 대상 아님) |
| 1 | 파일을 `core`/`infra`/`ui`로 이동 | **완료** — develop 머지 |
| 2 | 잔여 정리 (앱 헤더 DB 의존, `SqlInitializer`) | **완료** — develop 머지 |
| 3-A | 공통 컨트롤 승격 | 진행 중 |
| 3-B | 패널 분리 | 대기 |
| 4 | 워크플로 핸들러 + 레지스트리 | 대기 |
| 5 | `Sage` 접두사 전환 (상수·Define) | 대기 |

### Step 간 의존

```
3-A ──→ 3-B(법인발주 · 단가계산 · 단가관리)
                 컨트롤이 스스로 그리면 패널 분리가 단순해지고
                 CWnd 파생 승격 재검토가 가능해진다

4   ──→ 3-B(결과리스트 + 필터)
                 핸들러가 "결과 컬럼 정의"와 "응답→행 변환"을 가져간 뒤에야
                 이 패널의 경계가 정해진다. 순서가 반대다

4   ──→ DEBT_LOG 열린 3건(ui→infra, core→infra, office→db)이 여기서 해소된다

5   ←── 3 · 4    3·4에서 새로 쓴 코드도 Step 5 치환 대상이다
```

3-B 전체가 Step 4에 막혀 있는 것이 아니다. **결과리스트+필터 하나만** 4 이후다.

`.claude/`는 `.gitignore` 대상이므로 스킬 변경은 브랜치·커밋 단위에서 제외한다.

---

## 완료된 작업

**Step 0 — 스킬 정비**
- `coding-rules`(어떻게 쓰나) / `coding-design`(어디에 만드나) 분리, 트리거 중복 0
- 타 프로젝트 잔재 제거, 깨진 참조 4건 해소, `git-workflow`를 실제 관례에 맞춤
- 포인터 반환: 금지 → **계약 규약** (비소유 기본 / `Get*`은 항상 유효·`Find*`는 NULL 가능 /
  소유권 이전은 `Create`·`Detach`·`Release`·`Post` / 출력 매개변수에 포인터 금지)
- `sagetaechang-ui`에 **MFC 공통 컨트롤 규격** 신설
  — 컨트롤이 자기를 그린다(리플렉션), 세 계층, `SageUiStyle`은 보조,
  새 UI 요소는 한 곳에서만 쓰여도 컨트롤로, 기존 구현과 신규 원칙 분리(에디트·버튼)
- `coding-design`에 "화면은 컨트롤을 그리는 방법을 몰라야 한다" + 패널 분리 방식
- `sagetaechang-plan` 신설 (이 문서의 작성·갱신 절차)

**Step 1 — 계층 재배치** (`1bcf839`~`9e91f9f`, 빌드 통과)
`core`(MFC 비의존) / `infra` / `ui` 3계층. 이동 커밋 0 insertions·0 deletions.

**Step 2 — 잔여 정리** (`19d95eb`~`3e09d87`, 빌드 통과)
앱 헤더의 DB 의존 제거, `SQLInitializer` → `SqlInitializer`.
과정에서 헤더 간접 의존이 드러나 전방 선언·직접 include로 정리.

**교훈**: Windows 경로 일괄 치환은 `sed`가 아니라 Python bytes 치환.

---

## 미해결 리스크

| # | 내용 | 영향 | 대응 |
|---|---|---|---|
| ~~R1~~ | ~~미검증 커밋 2개~~ | — | **해소** — 빌드 통과 확인, develop 머지 예정 |
| R2 | **폰트 불일치** — `m_wndResultResetBtn`은 `SetFont(m_fontContent)`인데 `OnDrawItem`은 `m_fontHeader`로 그림 | `GetFont()` 사용 시 화면 변경 | 승격 시 `SetFont(&m_fontHeader)`로 일치 |
| R3 | `coding-rules`에 "상수 접두사는 `TAECHANG_` 유지"가 남아 있는데 3-A에서 `SAGE_BUTTON_*`을 만든다 | 첫 코드부터 규칙 위반 | 아래 **선행 작업**에서 규칙 분리 |
| R4 | `TaechangDefine.h`에 enum이 0개 — enum 배치 관례가 없다 | `SAGE_BUTTON_VARIANT`를 어디 둘지 불명 | 컨트롤 전용 enum은 컨트롤 헤더에 (규칙에 명시) |

### 선행 작업 — `coding-rules` 상수 규칙 분리

현재 문구는 "상수 접두사만은 `TAECHANG_`을 유지한다"이다. 이유는 *"`TaechangDefine.h` 한 곳에
600개가 모여 있어 신규만 `SAGE_`로 하면 한 파일에 두 접두사가 섞인다"* 였다.
`SAGE_BUTTON_*`은 `SageButton.h`에 들어가므로 그 이유가 성립하지 않는다.

- [ ] **모듈 전용 상수·enum**은 해당 헤더에 두고 `SAGE_` 접두사를 쓴다
- [ ] **`TaechangDefine.h`의 공용 상수**는 `TAECHANG_`을 유지하고 Step 5에서 일괄 전환한다

---

## Step 3-A — 공통 컨트롤 승격 (진행 중)

브랜치: `refactor/view-control-extract`
규칙: `sagetaechang-ui` > *MFC 공통 컨트롤 규격*, `coding-design` > *화면은 컨트롤을 그리는 방법을 몰라야 한다*

### 원칙

- **컨트롤이 자기를 그린다.** 메시지 리플렉션. 부모 `OnDrawItem`에 컨트롤별 분기를 남기지 않는다
- **`SageUiStyle`은 지금 만들지 않는다.** 컨트롤 하나로는 공유할 대상이 없어 추측 설계가 된다.
  두 번째 컨트롤에서 실제 중복이 드러날 때 추출한다
- **복잡한 사례부터.** 인터페이스 모양은 가장 까다로운 사례가 결정한다.
  다이얼로그(버튼 2~3개, `IDOK`만 Primary)에 맞춰 만들면 View(버튼 27개 + 아이콘 3종)에서 다시 뜯는다

### 회수 대상 698줄

| 위치 | 줄 |
|---|---|
| View `OnDrawItem` | 128 |
| View `ApplyControlFonts` | 124 |
| View `OnCtlColor` | 112 |
| View `OnListCustomDraw` | 68 |
| View `OnSidebarTreeCustomDraw` | 36 |
| 다이얼로그 7개 `OnDrawItem` | 230 |

### 전체 진행

- [x] 컨트롤 서브클래스 4개 → `app/ui/drawing/` (`c2696c3`) — 빌드 통과
- [x] 접두사 `Sage` 전환 (`ddf4aa3`) — 빌드 통과
- [ ] **3-A-1 `CSageButton`** ← 다음 (아래 상세)
- [ ] 3-A-2 다이얼로그 7개 교체 — 6개는 `bPrimary = (nIDCtl == IDOK)` 뿐이라 단순.
      `TaechangPriceSimpleDlg`는 버튼 그리기 없음
- [ ] 3-A-3 `CSageSectionLabel` — `SS_OWNERDRAW` 7개, `DrawSectionLabel` + ID 7개 OR 제거
- [ ] 3-A-4 `CSageEdit` — `DrawEditBorder` 35곳 흡수
- [ ] 3-A-5 `CSageListCtrl` — `OnListCustomDraw`
- [ ] 3-A-6 `CSageSidebarTree` — `OnSidebarTreeCustomDraw`
- [ ] 3-A-7 중복이 드러나면 `SageUiStyle`로 추출
- [ ] 3-A-8 `OnCtlColor` 잔여 정리 (핸들러는 남을 수 있으나 컨트롤 종류 분기는 제거)

> 3-A-2 이후는 착수 시점에 상세화한다. 앞 단계 결과가 뒤 단계 설계를 바꾼다.

---

### 3-A-1 — `CSageButton` 상세

브랜치: `refactor/sage-button`

속성 셋이 **전부 ID 분기**다. 인스턴스 속성으로 옮긴다.

| 속성 | 현재 | 변경 후 |
|---|---|---|
| variant | ID 12개 OR (`bPrimary`) | `SetVariant(SAGE_BUTTON_PRIMARY)` |
| 아이콘 | ID 분기 4곳 / 종류 3개 (검색·계산·리셋) | `SetIcon(SAGE_BUTTON_ICON_SEARCH)` |
| 폰트 | ID 2개 분기 | `SetFont()` 결과를 `GetFont()`로 (R2) |

enum은 `SageButton.h`에 둔다 (모듈 전용 → `SAGE_` 접두사, R4).

#### 커밋 1 — 컨트롤 신설 (View 미변경)

- [ ] `coding-rules` 상수 규칙 분리 (선행 작업, 로컬이라 커밋 대상 아님)
- [ ] `app/ui/drawing/SageButton.h` — enum 2종 + 클래스 선언
- [ ] `app/ui/drawing/SageButton.cpp` — `DrawItem` + 아이콘 3종 그리기
      (기존 `OnDrawItem` 2350~2462의 로직을 그대로 옮긴다. **로직 변경 금지**)
- [ ] vcxproj / filters 등록 (Python 스크립트)
- [ ] 검증: 빌드 통과 (아직 아무도 안 쓰므로 화면 변화 없어야 함)

#### 커밋 2 — View 버튼 교체

- [ ] `SageTaechangView.h`의 `CButton` 멤버 27개 → `CSageButton`
- [ ] `CreateChildControls` 등에서 Primary 12개에 `SetVariant` 호출
- [ ] 아이콘 4곳에 `SetIcon` 호출
      (`ID_TAECHANG_RESULT_SEARCH_BTN`, `ID_COORDER_SEARCH_BTN`, `ID_CALC_BTN`, `ID_CALC_RESET_BTN`)
- [ ] **R2 처리** — `m_wndResultResetBtn.SetFont(&m_fontHeader)`로 변경
- [ ] 검증: 빌드 + **화면 변화가 없어야 한다**
      이 시점에는 부모 `OnDrawItem`이 `ODT_BUTTON`을 가로채므로
      `CSageButton::DrawItem`은 아직 호출되지 않는다. `SetVariant`/`SetIcon`은 다음 커밋에서 활성화된다

#### 커밋 3 — View `OnDrawItem`에서 버튼 제거

**MFC 리플렉션은 base 호출을 타고 간다.** `CView::OnDrawItem`이 자식에게 리플렉트하므로,
버튼 블록을 제거하는 것만으로는 부족하고 **미처리 owner-draw를 반드시 base로 넘겨야** 한다.
(현재 콤보박스가 `CSageFilterComboBox::DrawItem`으로 그려지는 것도 이 경로다.)

- [ ] `OnDrawItem`의 `ODT_BUTTON` 처리 블록 제거 (2345~2462)
- [ ] 섹션 라벨(`ODT_STATIC`) 처리는 3-A-3까지 유지
- [ ] 남는 형태 — 미처리 건은 전부 base로

```cpp
void CSageTaechangView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
    if (lpDrawItemStruct->CtlType == ODT_STATIC && <섹션 라벨 ID>) {
        DrawSectionLabel(lpDrawItemStruct);
        return;
    }
    CView::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
```

- [ ] 검증: 빌드 + **워크플로 7개 화면의 버튼 표시가 이전과 동일한지**
      버튼이 전혀 안 그려지면 base 호출이 빠졌다는 신호다

#### 완료 기준

- `OnDrawItem`에 `bPrimary` ID 조건식이 없다
- 버튼을 추가할 때 `OnDrawItem`을 고칠 필요가 없다
- **화면 표시가 이전과 동일하다** — 색·테두리·아이콘·폰트·텍스트 위치

### 이번 범위 제외

- **hover(hot)** — `TrackMouseEvent`/`ODS_HOTLIGHT` 전무. 새 기능이라 제외.
  `itemState`를 그대로 넘기는 시그니처로 두어 나중에 시그니처 변경 없이 추가 가능하게 한다
- **Danger 변형** — 삭제 버튼 색이 바뀌는 디자인 변경. 별도 UI 결정 항목
- **폰트 저장소** — `ApplyControlFonts` 124줄은 같은 병증이나 공용 폰트 저장소가 필요.
  컨트롤 승격 후 별도 작업

---

## Step 3-B — 패널 분리

비윈도우 헬퍼 방식. 컨트롤은 View의 자식으로 두고 패널이 `CSageTaechangView&`를 참조로 든다.
**메시지맵은 View에 남기고 핸들러는 한 줄 위임만 한다.**

- [ ] **법인발주** — 핸들러 6, 멤버 16, 워크플로 무관. `SageTaechangView.cpp` 3923~4290 연속 블록.
      View 잔류: 메시지맵 7줄, `OnDraw`의 `m_rectCoCard`(→ 패널에 `GetCardRect()`)
- [ ] **단가계산** — 핸들러 8, 멤버 32
- [ ] **단가관리** — 핸들러 14, 멤버 30
- [ ] **결과리스트 + 필터** → **Step 4 이후.**
      `ApplyResultColumns` / `UpdateResultColumns` / `RebuildCurrentWorkflowResultList` /
      `DisplayResponse` / `InsertResultRow` / `RefreshDocumentResultFilter` 6개가 워크플로 타입에
      의존한다. Step 4에서 핸들러가 가져갈 책임과 겹치므로 먼저 묶으면 다시 뜯는다

**3-A 완료 후 `CWnd` 파생 승격을 재검토한다.** 컨트롤이 스스로 그리면 부모가 누구든 무관해져
`CWnd` 파생의 주된 위험(오너드로우 라우팅 재배치)이 사라진다.

---

## Step 4 — 워크플로 핸들러

브랜치: `refactor/workflow-handler`
배치: `app/core/workflow/handlers/` (현재 `app/core/workflow/`에 `TaechangWorkflowResponse`,
`TaechangWorkflowResultPresenter` 2개가 있으므로 핸들러 5쌍은 하위 폴더로)

- [ ] **`ISageWorkflowHandler`** 정의 — 탭 구성 / 출력폴더 필요 여부 / 행 선택 필요 여부 /
      결과 컬럼 정의 / 응답→행 변환
- [ ] 레지스트리 + 워크플로 1~5 핸들러 (미수금·납품·견적·PDF·HWP)
- [ ] View의 워크플로 조건 분기 사슬 제거
- [ ] 결과리스트 패널 분리 (3-B에서 미룬 항목)

### 조회는 `Find*`다

**워크플로 6·7(단가관리·계산)은 핸들러가 없다.** 패널로 유지하기로 했고,
`IsPriceWorkflowType()` 인라인이 이미 그 구분을 하고 있다.

```cpp
ISageWorkflowHandler* FindHandler(int nWorkflowType);   // NULL 가능, 호출부가 검사
```

`Get*`으로 지으면 계약이 거짓이 된다.

### core의 MFC 비의존 유지

핸들러가 "결과 컬럼"과 "탭 구성"을 답해야 하는데 `app/core/`는 MFC 헤더를 넣지 않는다.
컬럼은 이름·너비·정렬을 담은 값 객체로, 탭은 인덱스·라벨로 표현한다. `CString`은 허용된다.

### 완료 기준 — 등록부 1곳은 허용한다

C++에서 핸들러 자동 등록(정적 초기화 트릭)은 초기화 순서 문제와
링커가 참조 없는 오브젝트를 제거하는 문제가 있어 함정이 많다. 현실적인 기준으로 잡는다.

> **워크플로 추가 = 핸들러 파일 1쌍 + 중앙 등록부 1곳 수정.**
> View / 결과 컬럼 / 응답 파싱 / 탭 구성 어디도 고칠 필요가 없다.

"파일 하나만"을 목표로 하려면 자동 등록 구조를 별도로 설계해야 한다. 이번 범위 밖이다.

DEBT_LOG 열린 3건도 여기서 해소한다.

---

## Step 5 — `Sage` 접두사 전환 (상수·Define)

브랜치: `refactor/sage-prefix-constants`

구조 리팩토링과 별개인 네이밍 통일. 기계적 치환이라 위험은 낮지만 파일이 많아 섞지 않는다.
**3-A·3-B·4에서 새로 쓴 코드가 참조하는 `TAECHANG_` 상수도 함께 치환된다.**

- [ ] `TaechangDefine.h` → `SageDefine.h` (`git mv`)
- [ ] 상수 `TAECHANG_` → `SAGE_` (정의 629개 + 전 소스 참조)
- [ ] `#include` 참조, vcxproj / filters 갱신
- [ ] `coding-rules`의 공용 상수 접두사 규칙을 `SAGE_`로 수정
- [ ] 치환은 Python으로 (sed 백슬래시 사고 방지)

**범위 밖**: 나머지 `Taechang*` 클래스 27개와 파일명. `coding-rules`대로 해당 파일을
구조적으로 손댈 때 함께 전환한다 (`app/ui/drawing/` 사례처럼).

---

## 검증 방법

1. **빌드는 사용자가 직접 확인한다** (Claude는 MSBuild를 실행하지 않음)
2. **3-A는 화면 표시가 바뀌면 실패다.** 리팩토링이므로 이전과 동일해야 한다
   - 확인 대상: 워크플로 7개 화면 + **다이얼로그 7개** (로그인, 비밀번호 변경, 법인,
     단가 범위, 단가 간편, 계산 법인선택, 계산 견적)
3. 3-B는 컨트롤 ID와 메시지 라우팅이 이동하므로 화면별 수동 확인
4. Step 4는 워크플로 5개의 조회·생성 동작을 각각 1회씩
5. Step 5는 빌드가 통과하면 사실상 검증 완료 (기계적 치환)

`git-workflow` 준수: Step마다 별도 브랜치, 커밋은 목적 단위 분리,
`docs/decisions/PR_LOG.md`에 `docs:` 커밋으로 기록.
