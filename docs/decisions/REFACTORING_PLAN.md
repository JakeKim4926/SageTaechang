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
| ~~R2~~ | ~~폰트 불일치~~ | — | **해소** — `fa65896`에서 `SetFont(&m_fontHeader)`로 일치 |
| ~~R3~~ | ~~상수 접두사 규칙 충돌~~ | — | **해소** — `coding-rules`에서 공용/모듈 전용 분리 |
| ~~R4~~ | ~~enum 배치 관례 없음~~ | — | **해소** — `coding-rules`에 *Enum 위치* 규칙 추가 |

현재 열린 리스크는 없다.

### 해소된 선행 작업 — `coding-rules` 상수·enum 규칙

기존 문구는 "상수 접두사만은 `TAECHANG_`을 유지한다"였고, 이유는 *"`TaechangDefine.h` 한 곳에
600개가 모여 있어 신규만 `SAGE_`로 하면 한 파일에 두 접두사가 섞인다"* 였다.
`SAGE_BUTTON_*`은 `SageButton.h`에 들어가므로 그 이유가 성립하지 않아 **위치 기준으로 갈랐다.**

- [x] **모듈 전용 상수·enum**은 해당 헤더에 두고 `SAGE_` 접두사를 쓴다
- [x] **`TaechangDefine.h`의 공용 상수**는 `TAECHANG_`을 유지하고 Step 5에서 일괄 전환한다
- [x] **Enum 위치** 규칙 신설 — 한 모듈 전용이면 그 헤더, 공유하면 `TaechangDefine.h`.
      타입명은 PascalCase, 값은 대문자 + `_`, 값에 타입명을 반복하지 않는다

---

## Step 3-A — 공통 컨트롤 승격 (진행 중)

브랜치: 하위 단계마다 별도 (`refactor/view-control-extract`, `refactor/sage-button`, …)
규칙: `sagetaechang-ui` > *MFC 공통 컨트롤 규격*, `coding-design` > *화면은 컨트롤을 그리는 방법을 몰라야 한다*

### 원칙

- **컨트롤이 자기를 그린다.** 메시지 리플렉션. 부모 `OnDrawItem`에 컨트롤별 분기를 남기지 않는다
- **`SageUiStyle`은 지금 만들지 않는다.** 컨트롤 하나로는 공유할 대상이 없어 추측 설계가 된다.
  두 번째 컨트롤에서 실제 중복이 드러날 때 추출한다
- **복잡한 사례부터.** 인터페이스 모양은 가장 까다로운 사례가 결정한다.
  다이얼로그(버튼 2~3개, `IDOK`만 Primary)에 맞춰 만들면 View(버튼 27개 + 아이콘 3종)에서 다시 뜯는다

### 회수 대상 698줄 중 462줄 완료 (66%)

| 위치 | 줄 | 상태 |
|---|---|---|
| View `OnDrawItem` | 128 | **완료** — 함수 자체가 사라짐 (3-A-1, 3-A-3) |
| 다이얼로그 8개 `OnDrawItem` | 230 | **완료** — `DrawSimpleButton`까지 (3-A-2) |
| View `ApplyControlFonts` | 124 | 폰트 저장소 필요, 3-A 이후 별도 작업 |
| View `OnCtlColor` | 112 | 3-A-8 |
| View `OnListCustomDraw` | 68 | **완료** (3-A-5) |
| View `OnSidebarTreeCustomDraw` | 36 | **완료** (3-A-6) |

**`OnDrawItem`·커스텀드로우 계열이 전부 제거됐다.** 남은 것은 폰트·색상 축이다.
View의 오너드로우 컨트롤 34개(버튼 26 + 섹션 라벨 7 + 필터 콤보 1)가 모두 `CSage*`로 승격됐다.

### 전체 진행

- [x] 컨트롤 서브클래스 4개 → `app/ui/drawing/` (`c2696c3`) — 빌드 통과
- [x] 접두사 `Sage` 전환 (`ddf4aa3`) — 빌드 통과
- [x] **3-A-1 `CSageButton`** (`034ffb4`~`46b0081`) — 빌드·화면 확인 완료
- [x] **3-A-2 다이얼로그 8개 교체** (`60460c7`~`7d13b5e`) — 289줄 삭제, 화면 확인 완료
      파일은 7개지만 `TaechangPriceSimpleDlg`에 클래스가 2개 있어 실제 8개.
      전부 `bPrimary = (nIDCtl == IDOK)` 하나뿐이라 단순했다.
      이미 있던 `static DrawSimpleButton`(두 클래스가 공유하던 공통화 시도)도 함께 제거
- [x] **3-A-3 `CSageSectionLabel`** (`f6450cf`~`4cde74f`) — 화면 확인 완료
      `SS_OWNERDRAW` 7개 승격. **View의 `OnDrawItem`이 완전히 사라짐** (128 → 0줄).
      7개 모두 `SetFont(&m_fontContent)`와 그리기 폰트가 일치해 R2 같은 함정이 없었다
- [~] **3-A-4 `CSageEdit`** — **건너뜀.** 컨트롤 승격 문제가 아니라 테두리 배치·테마 정책 문제다.
      `DrawEditBorder`가 `InflateRect(1,1)`로 **컨트롤 바깥 1px**에 그리므로 리플렉션으로 옮기면
      픽셀 위치가 이동한다. 3-A 완료 기준(화면 무변화)에 걸려 현행 유지.
      대상 16개 중 콤보박스 3개가 섞여 있어 `CSageEdit` 하나로 해결되지도 않는다.
      `DEBT_LOG`에 기록했고, `sagetaechang-ui`의 `WS_BORDER` 금지 문구는 View 입력 컨트롤 한정으로 수정
- [x] **3-A-5 `CSageListCtrl`** (`550df9e`~`7b1ce1b`) — 화면·속도 확인 완료
      `OnListCustomDraw`를 렌더링 속성 3종으로 분해
      (`SetAlternateRowColor` / `SetCenterFirstColumn` / `SetHighlightColumns`).
      컨트롤은 워크플로 타입을 모르고 View가 컬럼 범위만 전달한다.
      계산 내역 리스트는 메시지맵 미등록으로 원래 커스텀드로우가 걸리지 않았어서 OFF로 재현.
      **작업 중 O(N²) 리페인트 병목을 발견해 리스트 4곳에 `SetRedraw` 억제를 추가**(`perf:` 커밋 2개)
- [x] **3-A-6 `CSageSidebarTree`** (`5821403`~`676e0fa`) — 화면 확인 완료
      **설정 메서드가 필요 없었다.** `CSageTabCtrl`·`CSageSectionLabel`과 같은 단일 스타일 컨트롤.
      그룹 헤더 판정이 `GetParentItem(hItem) == NULL`로 트리 내부 정보만 쓰기 때문이다.
      워크플로 타입은 `SetItemData`에 실려 있으나 그리기 로직이 보지 않아 도메인 무지 기준을 원래부터 지켰다.
      `TVN_SELCHANGED`는 View 잔류 — 리플렉션은 `NM_CUSTOMDRAW`만 가져간다
- [ ] 3-A-7 중복이 드러나면 `SageUiStyle`로 추출 ← 다음 (컨트롤 7개 확보, 판단 시점)
- [ ] 3-A-8 `OnCtlColor` 잔여 정리 (핸들러는 남을 수 있으나 컨트롤 종류 분기는 제거)

> 3-A-2 이후는 착수 시점에 상세화한다. 앞 단계 결과가 뒤 단계 설계를 바꾼다.

---

### 3-A-1 — `CSageButton` (완료)

커밋 3개로 분리해 각 단계마다 화면을 확인했다.

| 커밋 | 내용 | 이 시점 화면 |
|---|---|---|
| `034ffb4` | `SageButton.h/.cpp` 신설 + vcxproj 등록 | 미사용이므로 변화 없음 |
| `fa65896` | View 버튼 26개를 `CSageButton`으로 교체, `SetVariant` 12 / `SetIcon` 4, R2 해소 | 부모가 계속 그리므로 변화 없음 |
| `46b0081` | `OnDrawItem`의 버튼 블록 117줄 제거, base 위임 | **여기서 컨트롤이 그리기 시작** — 이전과 동일 확인 |

**결과**
- `bPrimary` ID 12개 OR 조건식 소멸. 버튼을 추가해도 `OnDrawItem`을 고칠 필요가 없다
- `OnDrawItem` 128 → 11줄, `SageTaechangView.cpp` 4,305 → 4,204줄
- 체크박스 3개(`EstimateOnePage`, `PriceSingleCheck`, `PriceNoMaxCheck`)는 오너드로우가 아니라 제외

**교훈**
- 아이콘 색이 `variant`별로 달랐는데 확인해보니 **전부 그 시점의 텍스트 색과 같은 값**이었다.
  `GetTextColor()` 한 줄로 대체해 분기를 없애면서 결과는 동일하게 유지했다
- R2(폰트 불일치)는 `GetFont()`를 쓰기로 한 순간 드러났다.
  화면에 의존하는 값을 컨트롤로 옮길 때는 **설정값과 실제 사용값이 같은지** 먼저 확인해야 한다
- CRLF 파일에서 Python 정규식의 행끝 매칭(`$`)은 캐리지 리턴 때문에 실패한다.
  라인 단위로 캐리지 리턴을 떼어낸 뒤 매칭해야 한다 (Step 1의 `sed` 백슬래시 사고와 같은 계열)

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
