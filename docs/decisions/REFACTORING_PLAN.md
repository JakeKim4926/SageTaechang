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
| 3-A | 공통 컨트롤 승격 | **완료** — 3-A-1~8 (3-A-4는 건너뜀) |
| 3-B | 패널 분리 | 다음 |
| 4 | 워크플로 핸들러 + 레지스트리 | **진행 중** — 4-1~4-4 완료 (3-B보다 먼저, 사용자 결정) |
| — | 검수 워크플로(PDF·HWP) 제거 | **완료** — 4-4 이후 삽입, `b2e8169` |
| 4-B | 의존 역전 (core Service ↔ infra Repository) | 대기 — Step 4에서 분리 |
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
그럼에도 **4를 3-B보다 먼저 하기로 했다** — 결과리스트가 3-B에서 가장 큰 패널이라
그것만 뒤로 미루면 3-B를 두 번 나눠 하게 된다.

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

## Step 3-A — 공통 컨트롤 승격 (완료)

브랜치: 하위 단계마다 별도 (`refactor/view-control-extract`, `refactor/sage-button`, …)
규칙: `sagetaechang-ui` > *MFC 공통 컨트롤 규격*, `coding-design` > *화면은 컨트롤을 그리는 방법을 몰라야 한다*

### 원칙

- **컨트롤이 자기를 그린다.** 메시지 리플렉션. 부모 `OnDrawItem`에 컨트롤별 분기를 남기지 않는다
- **`SageUiStyle`은 지금 만들지 않는다.** 컨트롤 하나로는 공유할 대상이 없어 추측 설계가 된다.
  두 번째 컨트롤에서 실제 중복이 드러날 때 추출한다
- **복잡한 사례부터.** 인터페이스 모양은 가장 까다로운 사례가 결정한다.
  다이얼로그(버튼 2~3개, `IDOK`만 Primary)에 맞춰 만들면 View(버튼 27개 + 아이콘 3종)에서 다시 뜯는다

### 회수 대상 698줄 중 581줄 완료 (83%)

| 위치 | 줄 | 상태 |
|---|---|---|
| View `OnDrawItem` | 128 | **완료** — 함수 자체가 사라짐 (3-A-1, 3-A-3) |
| 다이얼로그 8개 `OnDrawItem` | 230 | **완료** — `DrawSimpleButton`까지 (3-A-2) |
| View `OnCtlColor` | 112 | **완료** — 31줄로 (3-A-8). 동적 색 2개 + 체크박스 1블록 + 기본값 2개 잔류 |
| View `ApplyControlFonts` | 124 | **완료** — 라벨 몫 34줄 제거, 76줄로 (3-A-8). 나머지는 비라벨 65개 |
| View `OnListCustomDraw` | 68 | **완료** (3-A-5) |
| View `OnSidebarTreeCustomDraw` | 36 | **완료** (3-A-6) |

**회수 대상이 전부 처리됐다.** 오너드로우·커스텀드로우·색상·폰트 축이 모두 컨트롤로 내려갔다.
View의 오너드로우 컨트롤 34개(버튼 26 + 섹션 라벨 7 + 필터 콤보 1)가 모두 `CSage*`로 승격됐다.

**단, 줄 수는 회수의 척도가 아니었다.** 3-A-8은 분기 119줄을 지우고 역할 지정 113줄을 새로 썼다.
View.cpp는 4,073 → 4,067줄로 6줄만 줄었다. 얻은 것은 길이가 아니라
**"View가 라벨의 색을 판단한다"가 "라벨이 자기 역할을 선언한다"로 바뀐 것**이다.
3-B에서 패널을 분리할 때 라벨이 역할을 들고 함께 이동한다는 점이 실제 이득이다.

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
- [x] **3-A-7 `SageUiStyle`** (`1448aa9`~`cf88dff`) — 화면 확인 완료
      컨트롤 7개를 조사해 **진짜 중복은 콤보 화살표 14줄 하나**임을 확인하고 그것만 추출했다.
      `SageComboBox`·`SageFilterComboBox`가 들여쓰기만 다른 동일 코드를 갖고 있었다.
      **추출하지 않은 후보와 이유** — 배경 채우기(컨트롤마다 색이 다름, `FillSolidRect` 한 줄),
      테두리(`CSageButton` Secondary 한 곳뿐), 가운데 텍스트(6곳이지만 관용구지 로직이 아님.
      폰트 선택→`DrawText`→복원 사이의 색·rect·포맷이 매번 달라 콜백/RAII가 필요해 손해).
      **`SageUiStyle`을 색상 파사드로 만드는 안은 기각** — 색은 이미 `TaechangDefine.h`에 모여 있어
      간접 층만 늘어난다. skill의 예시 3개(`FillControlBackground`/`DrawBorder`/`DrawCenteredText`)를
      실제 API와 "왜 아닌지"로 교체했고, 컨트롤 목록 상태·`CSageEdit` 보류 사유도 반영했다
- [x] **3-A-8 `CSageLabel` + `SageUiResources`** (`827eda3`~`260d1dd`) — 6단계 전부 화면 확인 완료
      색상과 폰트를 `SageUiResources` 한 곳으로 모으고 라벨 37개가 역할로 자기 색을 내게 했다.
      **6단계로 쪼갠 것이 결정적이었다** — 3·4단계는 화면이 변할 수 없는 구조라
      5단계에서 색이 틀어지면 원인이 매핑뿐임이 보장됐다.
      착수 전 39개 라벨의 `(텍스트/배경/폰트)` 매핑을 표로 확정하고 스크립트로 대조(불일치 0건).
      아래 *3-A-8 결과* 참조

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

### 3-A-8 — `CSageLabel` + `SageUiResources` (완료)

색상(`OnCtlColor`)과 폰트(`ApplyControlFonts`)를 하나로 묶었다.
6단계로 나눠 각 단계마다 화면을 확인했다.

| 커밋 | 내용 | 이 시점 화면 |
|---|---|---|
| `827eda3` | `SageUiResources` 신설 + 폰트 4개 이관 | 변화 없음 |
| `5f1d805`~`0163b6d` | 브러시 6개 추가, View 브러시 4개 제거 | 변화 없음 |
| `5b66f49` | `CSageLabel` 신설 (미사용) | 미사용이므로 변화 없음 |
| `b9eb338` | 라벨 37개 타입 교체 + 역할 지정 | View 분기가 이기므로 변화 없음 |
| `6607cce` | `OnCtlColor` 라벨 분기 제거 | **여기서 라벨이 자기 색을 냄** — 이전과 동일 확인 |
| `260d1dd` | `ApplyControlFonts` 라벨 34줄 제거 | 변화 없음 |

**결과**

- `OnCtlColor` 112 → 31줄. 잔류는 동적 색 2개 + 체크박스 1블록 + 기본값 2개
- `ApplyControlFonts` 114 → 76줄. 남은 `SetFont` 65개는 전부 비라벨
- 폰트가 `ApplyControlFonts` 안에서 생성되던 구조(두 번 호출 불가)가
  `InitInstance`/`ExitInstance` 수명으로 정리됐다. `LoadPrivateFonts()` 이후여야 한다
- 헤더 상태 브러시의 `DeleteObject`/`CreateSolidBrush` 재생성이 사라졌다
- View.cpp 4,073 → 4,067줄. **줄 수는 6줄만 줄었다** (분기 119줄 제거, 역할 지정 113줄 추가)

**남긴 것과 이유**

- 동적 색 2개(`m_wndHeaderStatus`·`m_wndActionStatus`)는 `CStatic`으로 남겼다.
  `CSageLabel`은 역할이 고정값이라 런타임에 바뀌는 색을 담을 수 없다
- 체크박스 2개는 `CButton`이라 대상이 아니다. `CSageCheckBox`는 만들지 않았다
- 구분선 3개는 원래 폰트를 받지 않아 `SetFontRole`을 호출하지 않았다
- `m_wndPriceCompanyLabel`의 배경이 APP인 것은 버그지만 현행대로 옮겼다.
  여기서 고치면 5단계 화면 변화의 원인을 구분할 수 없다. `DEBT_LOG` 기록

**교훈**

- **역할 매핑을 착수 전에 표로 확정하고 확인받은 것이 이 단계를 살렸다.**
  39개를 옮기고 나서 색이 틀어지면 어디가 원인인지 찾을 수 없다.
  매핑을 문서로 고정한 뒤 코드와 스크립트로 대조했다 (불일치 0건)
- **화면이 변할 수 없는 단계와 변할 수 있는 단계를 분리한 것**이 핵심이었다.
  3·4단계는 View 분기가 이기는 구조라 화면 변화가 원천적으로 불가능하고 5단계에서만 변할 수 있다.
  그래서 5단계에서 이상이 생기면 원인이 매핑 하나로 좁혀진다
- 리플렉션 동작을 추측하지 않고 MFC 원본에서 확인했다.
  `CWnd::OnCtlColor`(`wincore.cpp:4308`)가 `SendChildNotifyLastMsg`로 자식에게 먼저 넘기고
  **자식이 반환한 브러시를 그대로 돌려준다.** 그래서 부모는 `CView::OnCtlColor`의 반환값을
  그대로 조기 반환하면 된다. 자식이 NULL을 반환할 때만 부모가 처리한다 (`wincore.cpp:3515`)
- `GetBrush`는 `HBRUSH`를 반환해야 한다. `CBrush*`로 두면 C2440 —
  `OnCtlColor`에서 되던 것은 `CBrush::operator HBRUSH()` 때문이고 포인터에는 적용되지 않는다
- 계획서의 "약 140줄 회수" 예상은 빗나갔다. **제거할 줄만 세고 새로 쓸 줄을 세지 않았다.**
  다음 단계 견적에서는 양쪽을 다 센다
- 죽은 코드 2건(헤더 상태 기능 전체, `m_brushListHeader`)을 발견해 `DEBT_LOG`에 기록했다.
  이번 변경이 만든 것이 아니라 손대지 않았다

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

## Step 4 — 워크플로 핸들러 (진행 중)

브랜치: 하위 단계마다 별도
규칙: `coding-design` > *View 비대화 방지*, `sagetaechang-ui` > *컨트롤은 도메인 개념을 알면 안 된다*

### 조사로 확정된 사실

- 워크플로 타입 분기가 **View의 함수 27개**에 흩어져 있다 (`TAECHANG_WORKFLOW_*` 사용 75곳)
- 대상은 **워크플로 1~3**이다. 4·5(PDF·HWP 검수)는 2026-08-04에 제거했고(`b2e8169`),
  6·7(단가관리·계산)은 패널이라 핸들러가 없다 (`IsPriceWorkflowType()`이 이미 가른다)
- `RunWorkflowWorker`(View.cpp의 static 함수)가 **infra 서비스를 직접 생성·호출**한다
  (검수 제거 후 3개: 미수금·납품·견적 엑셀). View.cpp의 `#include "app/infra/..."`가 그 결과다
- `app/core/workflow/`에는 현재 `TaechangWorkflowResponse`·`TaechangWorkflowResultPresenter` 2쌍만 있다
- `CSageListCtrl`은 이미 도메인을 모른다 (`SetHighlightColumns(시작, 개수)`).
  **컨트롤 API는 그대로 두고 판단 주체만 핸들러로 옮긴다**

### 분기의 축 9개

| 축 | 대표 함수 | 분기 수 |
|---|---|---|
| 요청 ID·실행 | `GetTaskRequestId`, `RunWorkflowWorker` | 10 |
| 실행 전 검증 | `RunWorkflowTask` | 7 |
| 응답 표시 | `DisplayResponse` | 8 |
| 입력 선택 | `OnSelectInput`, `ApplyDroppedInputPaths`, `OnInputReset` | 9 |
| 결과 컬럼 | `ApplyResultColumns` + 술어 3개 | — |
| 탭 구성 | `ApplyWorkflowTabs`, `HasDocumentResultTab` | 2 |
| 레이아웃·전환 | `LayoutChildControls`, `OnWorkflowChanged`, `OnSidebarSelectionChanged` | 9 |
| 라벨 | `UpdateWorkflowLabels` | 4 |
| 결과 필터 | `GetDefaultFilterCriteria` 외 3개 | 6 |

### 배치

```
app/core/workflow/
  ISageWorkflowHandler.h          탭·컬럼·라벨·검증·필터를 답한다
  ISageWorkflowRunner.h           실행 (core가 정의, infra가 구현)
  SageWorkflowResultTable.h/.cpp  컬럼 값 객체 + 표 표시 속성 + 공용 기본 표 (Win32 상수 금지, 자체 enum)
  SageWorkflowRegistry.h/.cpp     등록부 1곳. FindHandler(int) — NULL 가능
  handlers/                       핸들러 3쌍 (미수금·납품·견적)
```

`core`는 `afxwin.h`를 넣지 않는다. `CString`은 허용된다.
정렬은 `LVCFMT_*`를 쓰지 않고 자체 enum으로 두고 View가 변환한다.

### 8단계 분할

| # | 내용 | 화면 | 상태 |
|---|---|---|---|
| 4-1 | 인터페이스 + 등록부 + 핸들러 5개 골격. **View는 아직 쓰지 않는다** | 무변화 | **완료** `92468e4` (#92) |
| 4-2 | 라벨 축 — `UpdateWorkflowLabels` | 무변화 | **완료** `8a72d27` (#93) |
| 4-3 | 탭 **구성** 축 — `ApplyWorkflowTabs`, `HasDocumentResultTab`, 인덱스 변환 2개 | 무변화 | **완료** (빌드 확인 대기) |
| 4-4 | 결과 컬럼 축 — `ApplyResultColumns` · `UpdateResultColumns` | 무변화 | **완료** `54e51bd` |
| 4-5 | 입력 축 — `OnSelectInput`, `ApplyDroppedInputPaths`, `OnInputReset` | 무변화 | **완료** `e6b616b` |
| 4-6 | 검증·필터·UI상태 축 — `RunWorkflowTask` 검증, 필터 4개, `GetWorkflowUiState` | 무변화 | 다음 |
| 4-7 | 응답 표시 축 — `DisplayResponse` | 무변화 | 대기 |
| 4-8 | **실행 축 + infra 역전** — `ISageWorkflowRunner`, infra 3개 구현, View의 infra include 제거 | 무변화 | 대기 |

각 단계마다 빌드 가능한 상태를 유지한다. **화면 변화가 생기면 즉시 중단하고 원인을 보고한다.**

3-A-8의 교훈을 적용한다 — **단계마다 제거할 줄과 새로 쓸 줄을 함께 센다.**

### 4-1~4-4 결과

**4-1** (`92468e4`) — 인터페이스·등록부·핸들러 5개 골격. View 미사용이라 화면이 변할 수 없는 단계.
정적 인스턴스 5개를 익명 네임스페이스 배열에 두고 `FindHandler`가 선형 탐색한다.

**4-2** (`8a72d27`) — 라벨 4종(`GetHeaderTitle` / `GetInputSectionLabel` /
`GetActionButtonLabel` / `GetDetailSectionLabel`, 전부 `LPCWSTR` 반환)을 핸들러가 답한다.
워크플로 5개 순회로 화면 무변화 확인.

- **`FindHandler`의 NULL 경로는 도달하지 않는다.** `OnWorkflowChanged`가 `IsPriceWorkflowType`으로
  조기 반환하므로 `UpdateWorkflowLabels`에 오는 것은 워크플로 1~5뿐이다.
  View는 NULL이면 조기 반환한다 — 기존 `else`(미수금 겸 catch-all)를 남기면 완료 기준이 깨진다
- 실행 버튼은 `GetActionButtonLabel`이다. 검수 워크플로에서는 "생성"이 아니라 "실행"이라 `Generate`로 짓지 않았다
- PDF와 HWP의 버튼 상수는 값이 같지만(`검수 실행`) 통합하지 않았다. 값이 같은 것과 개념이 같은 것은 다르다
- `m_wndDetail` 본문은 라벨이 아니라 View 상태(`m_strExecutionHistory`)에 의존해 범위 밖으로 뒀다
- **줄 수 견적이 맞았다** — 예상 View -15 / 전체 +110, 실제 **View -13 / 전체 +97**.
  3-A-8에서 빗나간 이유(제거할 줄만 세고 새로 쓸 줄을 세지 않음)가 해소됐다.
  단 이 단계 단독으로는 손해이며, 회수는 4-3 이후 같은 핸들러에 축이 누적되면서 일어난다

**4-3** — 탭 목록을 `(semantic 인덱스, 라벨)` 순서 배열(`SageWorkflowTab`)로 표현해
`ApplyWorkflowTabs` · `GetTaskTabVisualIndex` · `GetTaskTabSemanticIndex`가 모두 배열 조회가 됐고,
`HasDocumentResultTab`은 호출자가 사라져 제거했다.

- 납품·견적만 결과 탭이 없어 **실행 기록이 semantic 2인데 visual 1**이다.
  인덱스 변환 함수 2개가 존재하는 이유가 이 한 줄이었고, 배열이 그 매핑을 그대로 담는다
- **계획서의 4-3 범위 기재를 정정한다.** 이전 표기는 `IsCompareWorkflow`를 대상에 넣었으나
  4-3에서 3곳만 줄어 **15곳이 남는다.** 남은 곳의 소관은 아래와 같다

**탭 축은 "구성"과 "판정"이 다른 단계다.** 구성(어떤 탭을 어떤 순서로 그리는가)은 4-3,
판정(지금 선택된 탭이 결과 탭인가)은 원래 4-6 소관이었다.

> **후속 정정 (2026-08-04)** — 위에서 "15곳이 남는다"고 적었던 `IsCompareWorkflow` 잔존분은
> **검수 워크플로 제거(`b2e8169`)로 전부 사라졌다.** 술어 자체가 없어졌으므로 3-B·4-6·4-7이
> 이 축을 이어받을 일이 없다. 아래 *검수 워크플로 제거* 참조.

**4-4** (`54e51bd`) — 결과 표 정의를 핸들러가 답한다. **View에서 108줄이 빠지고 25줄이 들어와 순 -83줄**로,
지금까지 축 중 회수가 가장 컸다.

- **컬럼 목록이 두 함수에 따로 적혀 있던 것이 하나가 됐다.** `ApplyResultColumns`(삽입)와
  `UpdateResultColumns`(너비)가 같은 표를 순회한다. 이전에는 한쪽만 고치면 어긋나는 구조였다
- 값 컬럼이 창 폭에 맞춰 늘어나는 동작은 `bStretch` 한 필드로 표현했다
- 기본 표(항목·값·상태·사유)는 `SageWorkflowResultTable`에 한 벌만 두고 3개 핸들러가 공유한다.
  검수 핸들러는 앞에 파일명 하나를 얹는다. **5벌 복제를 피한 이유**는 기본 표에 컬럼을 추가할 때
  5곳을 고쳐야 하는 구조가 이 리팩토링이 없애려는 병증 그 자체이기 때문이다
- 계획서의 파일명을 정정했다 — `SageWorkflowColumn.h` → `SageWorkflowResultTable.h/.cpp`.
  컬럼 값 객체만으로는 부족했고 표시 속성(체크박스·그리드선·강조)과 공용 기본 표가 함께 필요했다

**착수 전 확정한 사실 — `m_nLastWorkflowType`은 항상 `0` 아니면 선택된 워크플로다.**
`SetRunningState(TRUE)`가 사이드바를 비활성화해(`:1811`) 실행 중 전환이 막히고,
문서 워크플로는 상태가 워크플로별로 보관되며, 그 외는 전환 시 `0`으로 초기화된다.
이 불변식이 없었으면 "직전 워크플로"의 핸들러를 따로 조회해야 했다.

**범위 밖으로 뺀 화면 변경** — 미수금·납품·견적 표가 창 폭을 채우지 못하고 오른쪽에 여백이 남는 문제는
4-4가 만든 것이 아니라 원래 있던 동작이다. 리팩토링과 섞지 않고 별도 `style:` 커밋(`5a456bc`)으로
컬럼 비례 확대를 적용했다. 가변 컬럼이 없는 표에만 적용되므로 기본·검수 표는 그대로다.

**4-5** (`e6b616b`) — 입력 축. 검수 제거 후라 분기가 둘뿐이었다.

- `GetInputDialogTitle` (제목 3종) + `UsesInputTable` (선택·드롭 자동 불러오기 + 입력 초기화 가능)
- **술어 3개가 아니라 성질 1개다.** 납품·견적은 입력 파일을 표로 펼쳐 행을 골라 생성하고,
  미수금은 불러온 결과 자체가 결과 표다. 이 성질을 4-6(선택 행 수집)·4-7(생성 결과 무표시,
  실행 후 탭 전환)에서도 쓰므로 `LoadsOnSelect` 같은 좁은 이름을 피했다
- View는 `+13 / -11`로 **순 +2줄**. 분기는 사라졌지만 핸들러 조회 3벌이 들어와 상쇄됐다.
  얻은 것은 줄 수가 아니라 세 함수에서 워크플로 상수가 사라진 것

### 검수 워크플로 제거 (2026-08-04, `b2e8169`)

4-4를 마친 뒤 **PDF·HWP 표지 검수(워크플로 4·5)가 실제로 쓰지 않는 기능**임이 확인되어 제거했다.
Step 4를 잠시 멈추고 먼저 처리한 이유는 **지울 코드를 핸들러로 옮기는 낭비를 막기 위해서**다.
남은 4-5~4-8에서 검수 분기가 차지하는 비중이 절반 가까이였다.

- **화면 변화 없음이 보장됐다.** `BuildSidebarTree`에 검수 그룹이 이미 없어 UI에서 도달할 수 없는 상태였다
- 삭제 규모: **20파일, -1,295줄** (infra 서비스 3개 766줄, 핸들러 2쌍, View 168줄, 상수 31개)
- `IsCompareWorkflow` 15곳이 통째로 사라져 탭 판정·레이아웃·행 삽입 분기가 접혔다
- `DEBT_LOG`의 *탭 semantic 상수 충돌* 항목이 함께 해소됐다 (겹치던 상수 3개가 모두 검수 전용이었다)
- 함께 정리한 고아: `ShowIFileSaveDialog`, `TaechangResultRow::m_strFile`, `ComposeReason`,
  `BuildRows`의 `outDetailText` 매개변수

**교훈 — 삭제는 3단계(호출부 → 파일 → 상수)로 나눠도 중간 커밋이 빌드되지 않을 수 있다.**
호출부에서 조건식을 지울 때 **뒤에서 쓰는 지역변수 선언까지 함께 지운 실수**가 있었다.
빌드 확인 후 5커밋을 1커밋으로 squash해 "항상 빌드 가능한 이력"을 회복했다.
다음 삭제 작업에서는 단계마다 빌드를 받거나 처음부터 한 커밋으로 간다.

### 조회는 `Find*`다

```cpp
ISageWorkflowHandler* FindHandler(int nWorkflowType);   // NULL 가능, 호출부가 검사
```

워크플로 6·7은 핸들러가 없으므로 `Get*`으로 지으면 계약이 거짓이 된다.

### 완료 기준

> **워크플로 추가 = `handlers/`에 파일 1쌍 + 등록부 1곳 수정.**
> View / 결과 컬럼 / 응답 파싱 / 탭 구성 어디도 고칠 필요가 없다.

측정 가능한 형태로:

- [ ] `View.cpp`에 `TAECHANG_WORKFLOW_RECEIVABLES`·`_DELIVERY`·`_ESTIMATE` 사용이 **0곳**
      (6·7 단가 워크플로와 사이드바 등록 데이터는 제외)
- [ ] `View.cpp`에 `#include "app/infra/office/..."`가 **0줄**
- [ ] 각 단계에서 화면 표시가 바뀌면 실패

자동 등록(정적 초기화 트릭)은 초기화 순서와 링커의 미참조 오브젝트 제거 문제가 있어 만들지 않는다.
등록부 1곳은 허용한다.

### 범위 밖

- **워크플로 6·7(단가관리·계산)** — 패널이지 워크플로 실행이 아니다. 핸들러를 만들지 않는다
- **레이아웃·전환 축** (`LayoutChildControls` 등 9분기) — 어느 패널을 보일지 결정하는 코드라
  3-B(패널 분리) 소관이다. 여기서 건드리면 두 작업의 검증 지점이 섞인다
- **결과리스트 패널 분리** — Step 4 완료 후 3-B의 첫 항목으로 진행한다
- **`CSageEdit` / 테두리 정책** — 3-A-4에서 보류한 그대로

### DEBT_LOG 3건 중 해소되는 것은 1건의 일부다

계획서는 이전에 "열린 3건을 여기서 해소한다"고 적었으나 **정정한다.**

| 부채 | Step 4 |
|---|---|
| UI→infra 직접 호출 | **부분 해소** — View의 office 서비스 5개는 4-8에서. 다이얼로그 3개가 `SageDBMgr`을 부르는 것은 단가·인증 경로라 워크플로와 무관 |
| core Service 헤더가 infra Repository 헤더에 의존 | **해소 안 됨** — auth·price·receivable Service 3개의 문제로 접점이 없다 |
| infra/office가 infra/db 참조 | **해소 안 됨** — `TaechangReceivablesExcelService`의 infra 내부 문제 |

남는 2.5건은 **Step 4-후속(가칭 Step 4-B) 의존 역전**으로 별도 처리한다.
Step 4에서 `core`가 인터페이스를 정의하는 패턴이 자리잡은 뒤에 하는 것이 순서상 맞다.

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
