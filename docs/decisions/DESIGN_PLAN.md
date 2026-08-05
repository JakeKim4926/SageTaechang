# SageTaechang 디자인 개선 계획

> 작성·갱신 규칙은 `sagetaechang-plan` skill을 따른다.
> Step 완료 시 체크박스와 상태 표를 갱신하고, 상세 체크리스트는 지우고 결과·교훈만 남긴다.
> **구조 리팩토링 계획(`REFACTORING_PLAN.md`)과 별 문서로 둔다.** 검증 기준이 정반대다 —
> 리팩토링은 "화면 표시가 바뀌면 실패", 디자인은 "화면 표시가 바뀌어야 성공"이다.
> 같은 브랜치에 섞으면 둘 다 검증할 수 없다.

## 출처

Claude Design 프로젝트 `d4a524f6-c20a-4550-879c-772ca9fbb521`
파일 `SageTaechang 디자인 개선안.dc.html` (1,208줄, 목업 9세트 + 다이얼로그 6종).

**로컬 사본: `docs/design/sagetaechang-design-proposal.dc.html`**
D7(화면별 적용)은 목업 CSS를 계속 실측해야 한다. Claude Design MCP 접근은 세션마다
재인증이 필요할 수 있고 원본이 수정될 수도 있으므로 **이 사본을 기준으로 삼는다.**
치수·색은 인라인 `style` 속성에 들어 있어 이 파일 하나로 전부 측정된다.

주의: 원본 프로젝트에 첨부된 `_ds/.../colors_and_type.css`는 **EchoQuant**라는
무관한 디자인 시스템(다크 테마·블루 브랜드·Inter 서체)이다. 우리 토큰의 출처가 아니므로
가져오지 않았다. 팔레트의 출처는 목업 자신의 인라인 스타일이다.

문서 구성: 1장 진단 10개 · 2장 토큰 재정의 · 3장 수정안 화면 9세트 · 4장 MFC 적용 가이드.
이 계획은 **2·3·4장 전부**를 대상으로 한다.

---

## Context

문서의 진단은 색이 아니라 **색의 사용량·위계·정렬·밀도** 문제라고 본다. 실제 코드와 대조한 결과 그 진단이 맞다.

| 문서 진단 | 코드 확인 |
|---|---|
| 브랜드 컬러가 면적을 다 먹음 | `TAECHANG_COLOR_LIST_HEADER = RGB(62,52,43)` 풀필 (`TaechangDefine.h:139`) |
| 금액 컬럼 노란 배경 | `LIST_AMOUNT_COL` / `_ALT` 2개로 3열 통칠 (`:137-138`, `SageListCtrl.cpp:99-103`) |
| 디스플레이 폰트를 UI 전역에 사용 | `TAECHANG_CONTROL_FONT_FACE = "Gmarket Sans TTF Medium"` (`:120`) |
| 컨트롤 높이가 제각각 | 28 / 28 / 24 (`:38`, `:39`, `:578`) |
| 라벨 폭이 통일되지 않음 | **7종** — 46 / 68 / 70 / 90 / 92 / 116 / 150 |
| 버튼에 위계가 없음 | `CSageButton` 변형이 Primary / Secondary **2종**뿐 (`SageButton.h`) |

**핵심 발견: 3장 목업은 새 기능을 요구하지 않는다.** `TaechangDefine.h`의 UI 문자열 287개와 대조한 결과,
목업이 보여주는 값은 대부분 이미 구현되어 있고 표시 위치·색·위계만 다르다.
진짜 신규 코드는 5개다 — 결과 요약 바 · 합계 행 · 실행 기록 표 · 순서 ↑↓ 이동 · 인라인 에러 라벨.

---

## 문서 본문과 목업이 어긋나는 지점 (조사로 확정)

문서를 그대로 옮기면 모순이 생기는 곳이다. **어느 쪽을 따를지 여기서 확정한다.**

| # | 문서 본문(4장) | 목업 실측 | 채택 | 근거 |
|---|---|---|---|---|
| 1 | 버튼 위계 **4단계**, Secondary = 외곽선 | 「파일 선택」·「취소」·「법인 수정」 = 테두리 **`#C9BFB1`(중성)** + 텍스트 `#2F2A24` | **목업** | 현재 Secondary는 카멜 테두리 + 카멜 텍스트다. 목업의 중성 테두리가 2장 "갈색을 넓은 면에서 걷어낸다"의 실제 실행이다. **Secondary는 추가가 아니라 재정의다** |
| 2 | 컨트롤 높이 **32 단일** | 30 / 32 / 34 / 36 혼재 (「폴더 열기」30, 「단가 추가」34, 「견적서 생성」36) | **문서 본문(32)** | 목업이 진단 6번("높이가 제각각")을 스스로 위반했다. 진단을 따른다 |
| 3 | 라벨 폭 **80 고정 (다이얼로그 포함)** | **64** 다수 · **96** 비밀번호 변경 · **80** 입력 파일·저장 위치 · 56 법인 DLG · 44 우측정렬 「페이지」 | **목업** | D3b에서 목업의 `<label style="width:">`를 전수 실측했다. **80px는 「입력 파일」·「저장 위치」 둘뿐이고, 로그인·단가관리·단가계산 폼 라벨은 전부 64다.** 본문/다이얼로그로 갈리는 것이 아니었다 |

3번 보충: 목업에서 80px를 쓰는 「입력 파일」·「저장 위치」는 **현재 화면에 없는 라벨이다.**
`m_wndInputLabel`·`m_wndOutputLabel`은 생성 후 `ShowWindow(SW_HIDE)`만 되고 `MoveWindow`가 한 번도 불리지 않는다
(`SageTaechangView.cpp:270,490`). `TAECHANG_LABEL_WIDTH = 90`이 참조 0곳인 이유가 이것이다.
이 라벨을 실제로 띄우면서 80px를 주는 것은 **D7-4** 소관이다.

추가로 확인한 값: 목업의 라벨 텍스트 색은 `#6E655B`이고, 표 헤더 텍스트 색 `RGB(110,101,91)`과 **같은 값**이다.
2장 토큰표의 "보조 텍스트 `#7A7064`"와는 다른 색이므로 별 토큰이 필요하다.

---

## 진행 상황

| Step | 내용 | 상태 |
|---|---|---|
| D0 | `sagetaechang-ui` skill 개정 | **완료** |
| D1a | 색 토큰 (표 헤더 · 금액 컬럼 · 사이드바 선택) | **완료** · 확인받음 |
| D1b | 서체 전환 (Pretendard) | **완료** · 확인받음 |
| D2 | 버튼 위계 4변형 (②) | **완료** · 확인받음 |
| D3a | 컨트롤 높이 32 통일 | **완료** · 확인받음 |
| D3b | 라벨 폭 통일 (64 / 96) | **완료** · 확인받음 |
| D4a | 행 높이 34 · 가로 hairline · 선택 행 | **완료** · 확인받음 |
| D4b | 금액 우측 정렬 · Bold · 음수 표시 | 대기 · **D1b 이후** |
| D4c | 공백 행 skip · `〃` · 법인 그룹 구분선 | 대기 |
| D5 | 공통 신규 요소 4종 (요약 바 · 빈 상태 · 인라인 에러 · 선택 카운터) (④) | 대기 |
| D6 | 아이콘 세트 (⑤) | 대기 |
| D7 | 화면별 적용 — 3장 9세트 | 대기 |
| D8 | DPI 배율 대응 (awareness 전환 + 좌표 스케일링) | 대기 · 디자인 완료 후 |

D0~D4a 커밋 14개는 이 문서에 `fix/design-tokens`로 적혀 있었으나, 실제로는 **`develop`에 직접** 쌓였다
(그 이름의 브랜치는 존재하지 않는다). D3b부터는 develop에서 `design/label-widths`를 파서 작업했다. **푸시·PR 안 함.**
다음 세션은 이 문서를 먼저 읽고 **D4b** 또는 **D4c**부터 이어간다.
화면 표시는 사용자 확인을 받았고 빌드는 사용자가 직접 확인했다.

## 완료된 작업과 교훈

### D0 — 스킬 개정
색·서체·버튼·표·크기 규격을 개선안 기준으로 다시 씀. 목업 원문 HTML과 대조해
스킬에 적은 색 17개 중 16개가 실제로 존재함을 확인했다(예외는 앱 배경 `#F8F6F1`, R8 결정).
스킬의 *검수 워크플로 · 탭 구성 · 결과 컬럼* 서술은 이번 건과 무관한 낡은 내용이라 남겨둠 (아래 미해결).

### D1a · D2 · D3a · D4a — 적용
상수는 **쓰는 Step에서 추가**하는 쪽으로 바꿨다. C1이 토큰 7개를 D1에서 한꺼번에
추가하도록 적었지만, 참조 없는 dead 상수가 되므로 그 Step에서 넣는다.

**교훈 1 — 커스텀 드로잉은 "그리기 코드"만으로 끝나지 않는다.**
D4a에서 세 번 헛짚었다. 원인이 전부 그리기 코드가 아니라 **적용 시점**과 **무효화 범위**였다.

| 증상 | 진짜 원인 |
|---|---|
| 행 높이가 안 변함 | `PreSubclassWindow`는 WM_NCCREATE 시점 → 리스트뷰 내부 초기화 전이라 `LVM_SETIMAGELIST`가 무시된다. `WM_CREATE`에서 기본 처리 뒤에 붙여야 한다 |
| 헤더 글씨가 뭉개짐 | `SetImageList`를 `CDDS_PREPAINT`에서 호출 → 페인트 중 재레이아웃 |
| 선택 액센트 바가 안 지워짐 | 선택 변경 시 이전 행이 재그려지지 않음. 1열을 `CDRF_SKIPDEFAULT`로 직접 그리면 액센트 3px 구간을 아무도 덮지 않는다 |
| 표가 깜빡임 | 전체 `Invalidate()` → 해당 행 rect만 무효화로 축소 |

**다음 커스텀 드로잉 작업에서는 그리기 코드보다 적용 시점·무효화 경로를 먼저 확정한다.**

**교훈 2 — 색을 바꾸면 숨어 있던 누수가 드러난다.**
강조 컬럼 `clrText`를 강조 열에서만 지정하던 기존 코드는 색이 본문색과 같아서 누수가 안 보였다.
카멜로 바꾸자 이후 컬럼(입금 은행·비고)이 물려받았다. 강조가 켜지면 **모든 서브아이템에서 색을 명시**해야 한다.

### D1b — 서체 전환 (Pretendard)

기존에 이미 `AddFontMemResourceEx` 임베드 폰트 구조가 있었다(Gmarket 3종). 그 패턴을 그대로 복제했다 —
`resources/` 평탄 배치 → `.rc`의 `TTF` 리소스 → `Resource.h` ID → `LoadPrivateFonts()`.
폰트가 6개가 되어 개별 핸들 멤버를 배열로 교체했다.

**GDI 패밀리 이름이 굵기마다 다르다** (폰트 name 테이블 직접 확인):

| 파일 | GDI Family | Subfamily |
|---|---|---|
| `PretendardRegular.ttf` | `Pretendard` | Regular |
| `PretendardBold.ttf` | `Pretendard` | **Bold** |
| `PretendardSemiBold.ttf` | **`Pretendard SemiBold`** | Regular |

GDI는 한 패밀리에 Regular/Bold/Italic/BoldItalic 4종만 담으므로 SemiBold는 자기 패밀리를 갖는다.
그래서 `TAECHANG_TITLE_FONT_FACE = L"Pretendard SemiBold"`가 맞다.

**`CreateFontIndirect` 전환은 D4b로 미뤘다.** 두 서체 모두 패밀리 이름만으로 지정되므로
`FW_BOLD`가 실제로 필요한 시점(금액 Bold)까지 `CreatePointFont`로 충분하다.
폰트를 임베드했으므로 **맑은 고딕 폴백도 불필요**해졌다 (R2 해소).

**R1 재결정 — 목업 대비 +1px로 확정.**
목업 그대로(본문 13px) 적용해보니 기존 앱(14.7px) 대비 체감 낙차가 컸다.
위계 비율은 유지하고 절대 크기만 한 단계 올렸다 — 제목 19 · 섹션 15 · 본문 14 · 표 셀 13px.
**의도적으로 목업을 벗어난 유일한 지점이다.** 상수 5개라 되돌리기 쉽다.

**교훈 5 — 폰트 의존 상수는 계산으로 검증할 수 있다.**
`TAECHANG_BUTTON_TEXT_TOP_OFFSET`은 서체 ascent 보정값이라 추측하지 않고 메트릭에서 계산했다.
Gmarket은 `winAsc/winDesc = 800/350`(비대칭)이라 계산값 +1.84 → 기존 튜닝값 `2`와 일치했고,
이것이 계산 모델의 검증이 됐다. Pretendard는 `1949/494`로 거의 대칭이라 계산값 −0.02 → **0**.
같은 방식으로 `TAECHANG_EDIT_TEXT_TOP_PAD`의 적정값은 8이나(현재 9), 확인받은 화면을
1px 때문에 흔들지 않으려고 두었다.

**교훈 3 — 헤더 높이와 행 높이는 다른 메커니즘이다.**
행은 이미지리스트, 헤더는 `HDM_LAYOUT`. 행만 키우면 헤더는 그대로다.

**교훈 4 — 컨트롤이 부모의 통지를 가로채지 않게 한다.**
`LVN_ITEMCHANGED`를 컨트롤에서 쓰려면 `ON_NOTIFY_REFLECT_EX` + `return FALSE`.
일반 `ON_NOTIFY_REFLECT`면 부모 3곳(단가 상세 · 법인 편집 폼 · 결과 표 체크박스 건수)이 죽는다.

### D3b — 라벨 폭

`TaechangDefine.h` 상수 5개만 바뀌었다. **좌표 코드는 한 줄도 손대지 않았다** — 카드 폭·에디트 폭이
전부 라벨 폭 상수에서 파생되므로 자동 전파됐다. 단가계산 입력 카드 274→310, 비밀번호 변경 에디트 202→222.

**교훈 6 — 목업을 부분만 실측하면 반대 결론이 나온다.**
*어긋나는 지점* 3번은 원래 "본문 80 · 다이얼로그 64/96"으로 결론냈는데, 그때는 다이얼로그만 봤다.
`<label style="width:">`를 전수로 뽑으니 **80은 두 개뿐이고 본문 폼도 64**였다.
목업에서 값 하나를 인용할 때는 **같은 속성을 문서 전체에서 뽑아 분포를 본다.**

**교훈 7 — 폰트 의존 치수는 앱을 띄우지 않고 검증할 수 있다.**
D3의 체크리스트는 `DT_CALCRECT` 실측이었지만, 빌드 없이 PowerShell에서 잴 수 있다 —
`AddFontResourceEx(FR_PRIVATE)`로 TTF를 프로세스에 붙이고 `CreateFontW(lfHeight=-14)` +
`GetTextExtentPoint32W`를 쓰면 MFC와 같은 GDI 경로다.

**함정: GDI+로 재면 조용히 틀린 값이 나온다.** 처음에 `System.Drawing.Font("Pretendard", ...)`로 쟀더니
GDI+는 private 로드된 GDI 폰트를 보지 못해 **Microsoft Sans Serif로 폴백**했고, 예외 없이
그럴듯한 숫자를 내놨다(비밀번호 라벨 88 → 96으로 8px 과대). `GetTextFaceW`로 **실제 선택된 face를
찍어 확인하기 전에는 측정값을 믿지 않는다.**

**교훈 8 — Step을 끝낼 때 스킬도 같이 본다. `sagetaechang-plan`의 반대 방향이 빠져 있었다.**
그 규칙은 *"규칙을 고친 뒤에는 계획을 재점검한다"*인데, 실제로 새는 쪽은 반대였다 —
**값을 고친 뒤 스킬을 재점검하지 않는 것.** D3b에서 스킬을 점검하다 D1b가 남긴 불일치를 발견했다:
`sagetaechang-ui`의 타입 스케일 표가 R1 재결정 이전 목업 값(`135/105/98/90/83`) 그대로였다.
그 상태로 D4b(금액 Bold — 폰트 상수를 만지는 Step)에 들어갔으면 첫 줄부터 어긋났다.
**Step 완료 조건에 "스킬의 해당 절이 코드와 일치하는가"를 포함한다.**

### Step 간 의존

```
D0 → D1     규칙을 먼저 고친다. 스킬이 "Gmarket 사용" · "Danger 도입 안 함"이라
            적힌 상태로 D1을 쓰면 첫 줄부터 규칙 위반이 된다

D1 → D2     버튼 4변형이 새 토큰(#C9BFB1 · #E0BDB6 · #6E655B)을 쓴다

D1 → D4     표 헤더·그리드선 색이 토큰에서 온다

D2 → D3     변형이 정해진 뒤에 높이를 만진다. 순서가 바뀌면 Primary/Secondary가
            섞인 상태로 좌표를 두 번 맞추게 된다

D3 → D7     화면별 배치는 높이 32 · 라벨 폭이 확정된 뒤에만 좌표가 한 번에 맞는다

D4 → D7     3-1 · 3-7 · 3-8이 전부 표 화면이다. 표 규격이 먼저다

D5 → D7     요약 바·빈 상태·인라인 에러·선택 카운터가 여러 화면에 반복 등장한다.
            공통 컨트롤로 만들고 화면은 배치만 한다 (`sagetaechang-ui` > 새 UI 요소 규칙)

D6 → D7     아이콘이 없으면 툴바 폭이 확정되지 않는다
```

### 구조 리팩토링과의 관계

**`REFACTORING_PLAN.md`의 3-B-4a(패널 View 연결)가 먼저다.** D7의 3-1 · 3-7 · 3-8은
`SageResultTablePanel`을 손댄다. 지금 그 패널은 신설됐지만 아무도 참조하지 않는 상태(`533ab0d`)라,
연결 전에 디자인을 넣으면 연결 커밋에서 충돌한다.

**D1~D4는 3-B-4a와 무관하다.** `TaechangDefine.h`와 `app/ui/drawing/` 클래스만 만지고
`SageTaechangView.cpp`의 구조는 건드리지 않는다. 병행 가능하다.

**Step 5(`TAECHANG_` → `SAGE_` 치환)와의 관계**: 이 계획에서 추가하는 상수는 **`TAECHANG_` 접두사로 만든다.**
접두사가 섞이면 Step 5의 기계적 치환이 깨진다. 새 상수도 Step 5에서 함께 치환된다.

---

## 미해결 리스크

| # | 내용 | 영향 | 대응 |
|---|---|---|---|
| R1 | **타입 스케일이 px 기준이다.** 문서 18/14/13/12/11px를 pt로 환산하면 13.5/10.5/9.8/9/8.3pt다. 현재는 제목 16pt · 본문 11pt이므로 **전체가 12~16% 작아진다** | 화면 전체 밀도가 바뀐다. 한글 9.8pt는 현재보다 눈에 띄게 작다 | **해소 — 목업대로 환산한다(ⓐ).** 근거: 라벨 폭 80/64/96은 13px 폰트를 전제로 계산된 값이다. 폰트를 11pt(14.7px)로 유지한 채 라벨만 줄이면 「새 비밀번호 확인」이 잘린다(R9). 폰트와 라벨 폭은 한 세트다. 상수 5개라 되돌리기 쉽다 |
| R2 | Pretendard 미설치 PC에서 `CreatePointFont`는 조용히 시스템 기본으로 떨어진다 (맑은 고딕이 아니다) | 배포 PC마다 다른 글꼴로 보인다 | TTF 동봉 + `AddFontResourceEx(FR_PRIVATE)`로 확정 로드. 실패 시 `CreateFontIndirect`로 맑은 고딕 명시 폴백 |
| R3 | GDI는 안티에일리어싱이 없다. 목업의 `border-radius:4px`·부드러운 hairline이 각지게 나온다 | 목업과 미세하게 다르다 | 라운드 코너를 포기하고 각진 사각형으로 간다. GDI+ 도입은 범위 밖 |
| R4 | 금액 tabular 정렬을 GDI `DrawText`로 만들 수 없다 (OpenType `tnum` 미지원) | 자릿수가 미세하게 어긋난다 | 문서 4장이 지시한 "우측 정렬 + Bold"로 대체. 그래도 어긋나면 금액 셀만 고정폭 폰트를 별도 지정 |
| R5 | `LVS_EX_GRIDLINES`는 가로·세로가 한 플래그다. "가로 hairline만"을 이 플래그로 만들 수 없다 | 플래그 사용 4곳을 전부 커스텀 드로잉으로 옮겨야 한다 | D4에서 `CSageListCtrl`이 `CDDS_ITEMPOSTPAINT`로 하단 1px만 긋는다. 대상: `SageTaechangView.cpp:465`, `SageResultTablePanel.cpp:234`, `SagePriceManagePanel.cpp:70`, `SageTaechangView.cpp:1832` |
| R6 | `CListCtrl`은 행 높이를 직접 지정할 수 없다 (폰트·이미지리스트 종속) | 34px 고정이 안 된다 | 1×34 더미 이미지리스트를 `SetImageList(LVSIL_SMALL)`로 물린다. 아이콘 열을 쓰지 않으므로 부작용 없음 |
| R7 | `AfxMessageBox`가 11파일 **90곳**에 있다. 목업은 다이얼로그 검증을 인라인 라벨로 보여준다 | 전부 바꾸면 범위가 폭발한다 | **다이얼로그 입력 검증분만** 인라인으로 옮긴다. 파일 I/O 실패·DB 오류 등 화면 밖 사유는 `MessageBox` 유지 |
| R8 | 앱 배경이 문서 `#F7F6F1`, 코드·스킬 `#F8F6F1`로 1/255 다르다 | 육안 차이 없음 | **현재 값 유지.** 바꾸면 코드+스킬 2곳을 건드리는데 얻는 것이 없다 |
| R9 | 라벨 폭을 줄이면 긴 라벨이 잘린다 (「현재 비밀번호」·「새 비밀번호 확인」) | 문구가 `...`로 잘린다 | **해소 — D3b에서 실측했고 잘리는 라벨이 없다.** 최장이 「변경할 비밀번호」 88px(96에 여유 8), 나머지 폼은 최장 52px(64에 여유 12). 우려했던 **「새 비밀번호 확인」은 코드에 없는 문구다** — 실제 상수는 `변경할 비밀번호`·`비밀번호 확인`이다. 목업 문구로 바꾸는 것은 D7-4 |
| R11 | **DPI awareness가 없다.** manifest 설정 0건, `GetDpiForWindow`/`GetDeviceCaps(LOGPIXELS*)` 호출 0곳. 반면 `CreatePointFont`는 pt→px 변환을 하므로 **폰트만 DPI에 반응한다** | awareness를 켜면 폰트는 커지고 고정 px 컨트롤은 그대로여서 글자가 버튼을 넘친다. 지금은 비트맵 확대로 흐릿하게 보인다 | C9-2 — 이번 상수를 「96dpi 논리 px」로 정의해 길만 열어두고, awareness 전환은 **Step D8**로 분리한다 |
| R10 | 미검증 커밋이 쌓여 있다 — `533ab0d`(`SageResultTablePanel` 688줄)는 빌드·동작 확인을 받지 않았다 | 디자인 변경 후 문제가 나면 원인 분리가 안 된다 | D1 착수 전에 사용자에게 현재 상태 빌드 확인을 요청한다 |

---

# 공통 규격 (먼저 확정 — 모든 화면이 이것을 참조한다)

3장 화면별 항목에서 반복 서술하지 않기 위해 여기 한 번만 적는다.
D7의 각 화면은 "여기 규격을 배치한다"만 한다.

## C1. 색상 토큰

### 값이 바뀌는 상수

| 상수 | 현재 | 변경안 |
|---|---|---|
| `TAECHANG_COLOR_LIST_HEADER` | `RGB(62,52,43)` | `RGB(242,238,231)` |
| `TAECHANG_COLOR_SIDEBAR_SELECTED` | `RGB(77,60,42)` | `RGB(58,49,41)` |

### 신규 상수

| 상수 | 값 | 용도 |
|---|---|---|
| `TAECHANG_COLOR_SURFACE_MUTED` | `RGB(242,238,231)` | 표 헤더 · 툴바 배경 (문서 2장이 한 항목으로 묶었으므로 상수 하나를 공유) |
| `TAECHANG_COLOR_TEXT_MUTED` | `RGB(110,101,91)` | 표 헤더 텍스트 · 폼 라벨 · Ghost 버튼 텍스트 (`#6E655B`) |
| `TAECHANG_COLOR_LIST_GRID` | `RGB(237,232,224)` | 표 가로 hairline (`#EDE8E0`) |
| `TAECHANG_COLOR_BUTTON_BORDER` | `RGB(201,191,177)` | Secondary 버튼 중성 테두리 (`#C9BFB1`) |
| `TAECHANG_COLOR_DANGER_BORDER` | `RGB(224,189,182)` | Danger 버튼 테두리 (`#E0BDB6`) |
| `TAECHANG_SIDEBAR_ACCENT_WIDTH` | `3` | 사이드바 선택 항목 좌측 액센트 바 폭 |
| `TAECHANG_LIST_ROW_HEIGHT` | `34` | 표 행 높이 |

사이드바 액센트 바 색은 `TAECHANG_COLOR_PRIMARY`를 재사용한다 (문서 지정값 `RGB(154,107,63)`과 동일).

### 제거하는 상수

| 상수 | 사유 |
|---|---|
| `TAECHANG_COLOR_LIST_AMOUNT_COL` | 금액 컬럼 배경 제거 (진단 4) |
| `TAECHANG_COLOR_LIST_AMOUNT_COL_ALT` | 동일 |
| `TAECHANG_COLOR_LIST_HEADER_DIVIDER` | 헤더 세로 구분선 제거 (세로선 제거 방침과 일관) |

### 값이 바뀌지 않는 것 (확인용)

문서 2장 ACCENT 5개는 **전부 현재 값과 동일**하다 — Primary `#9A6B3F` · Press `#76502A` ·
Success `#5F7F5F` · Warning `#B88746` · Error `#B85C4A`.
SURFACE 중 패널 `#FFFFFF` · 사이드바 `#241F1A` · 테두리 `#DCD6CD` · 본문 `#2F2A24` ·
보조 `#7A7064`도 동일하다. 앱 배경은 R8대로 현재 값 유지.

**즉 색 작업의 실체는 "표 헤더 1개 · 사이드바 선택 1개 변경 + 신규 5개 추가 + 제거 3개"다.**

## C2. 타입 스케일

**R1 해소 — 목업 px를 pt로 환산하되 한 단계 올린다(D1b 재결정, 아래).**
`CreatePointFont`는 0.1pt 단위이므로 ×10 값을 쓴다.

| 역할 | 목업 | 목업대로면 | **채택값** | 상수 | 상태 |
|---|---|---|---|---|---|
| 화면 제목 | 18px Bold | `135` | **`143`** (19px) | `TAECHANG_TITLE_FONT_POINT_SIZE` | 적용됨 |
| 섹션 제목 | 14px Bold | `105` | **`113`** (15px) | `TAECHANG_HEADER_FONT_POINT_SIZE` | 적용됨 |
| 본문 · 라벨 · 버튼 | 13px | `98` | **`105`** (14px) | `TAECHANG_CONTENT_FONT_POINT_SIZE` · `TAECHANG_CONTROL_FONT_POINT_SIZE` | 적용됨 |
| 표 셀 | 12px tabular | `90` | **`98`** (13px) | `TAECHANG_LIST_FONT_POINT_SIZE` | 적용됨 |
| 캡션 · 상태바 | 11px | `83` | **`90`** (12px) | — | **미도입** |

**이 표가 기준이다. 목업 px를 그대로 옮기지 않는다.** 목업 대비 +1px는 D1b에서 의도적으로 택한 이탈이며
사유는 아래 *D1b* 교훈에 있다. 표만 보고 작업해도 틀리지 않도록 채택값을 여기에 못박는다.

캡션은 쓰는 Step에서 추가한다(D1a 이후의 원칙 — 참조 없는 dead 상수를 미리 만들지 않는다).
본문과 컨트롤은 현재 같은 값이지만 상수가 둘이라 한쪽만 바꾸면 갈라진다.

## C3. 서체

| 상수 | 현재 | 변경안 |
|---|---|---|
| `TAECHANG_CONTROL_FONT_FACE` | `Gmarket Sans TTF Medium` | `Pretendard` |
| `TAECHANG_TITLE_FONT_FACE` | `Gmarket Sans TTF Bold` | `Pretendard SemiBold` |
| `TAECHANG_LOGO_FONT_FACE` | — (신규) | `Gmarket Sans TTF Bold` |

Gmarket Sans는 **로고(사이드바 상단 "SageTaechang")에만** 남긴다.
TTF는 프로젝트에 동봉하고 `AddFontResourceEx(..., FR_PRIVATE, 0)`로 앱 시작 시 로드,
종료 시 `RemoveFontResourceEx`로 해제한다. 배치는 `coding-design`으로 확정한다.

## C4. 컨트롤 규격

| 상수 | 현재 | 변경안 |
|---|---|---|
| `TAECHANG_BUTTON_HEIGHT` | 28 | 32 |
| `TAECHANG_EDIT_HEIGHT` | 28 | 32 |
| `TAECHANG_PRICE_EDIT_HEIGHT` | 24 | 32 |
| `TAECHANG_LABEL_WIDTH` | 90 | — (참조 0곳, 죽은 상수라 두었다) |
| `TAECHANG_CALC_INPUT_LABEL_WIDTH` | 46 | 64 |
| `TAECHANG_CALC_RESULT_LABEL_WIDTH` | 92 | 64 |
| `TAECHANG_PRICE_FORM_LABEL_WIDTH` | 70 | 64 |
| `TAECHANG_LOGIN_DLG_LABEL_WIDTH` | 68 | 64 |
| `TAECHANG_PASSWORD_DLG_LABEL_WIDTH` | 116 | 96 |

**라벨 폭 목표를 80에서 64로 정정했다** (D3b, 위 *어긋나는 지점* 3번). 80px는 목업에서 아직 화면에 없는
「입력 파일」·「저장 위치」 두 라벨에만 쓰이므로 D7-4로 넘어간다.

`TAECHANG_USER_LABEL_WIDTH = 150`은 헤더 사용자명 표시 폭이므로 폼 라벨이 아니다. **유지한다.**

아이콘 버튼은 32×32 정사각. 간격은 4의 배수만 사용한다.

## C5. 버튼 4변형

`SageButtonVariant`에 2개를 추가하고 Secondary를 재정의한다.

| 변형 | 배경 | 테두리 | 텍스트 | 굵기 | 용도 |
|---|---|---|---|---|---|
| `SAGE_BUTTON_PRIMARY` | `PRIMARY` | `PRIMARY` | 흰색 | Bold | 화면당 **1개**. 생성·실행·저장·확인 |
| `SAGE_BUTTON_SECONDARY` **(재정의)** | `PANEL` | `BUTTON_BORDER` | `TEXT` | Regular | 파일 선택 · 폴더 선택 · 취소 · 법인 수정 |
| `SAGE_BUTTON_GHOST` **(신규)** | 투명 | 없음 | `TEXT_MUTED` | Regular | 초기화 · 선택 해제 |
| `SAGE_BUTTON_DANGER` **(신규)** | `PANEL` | `DANGER_BORDER` | `ERROR` | Bold | 삭제 계열 전용 |

### 화면별 Primary 배정 (한 화면에 하나)

| 화면 | Primary | Secondary | Ghost | Danger |
|---|---|---|---|---|
| 미수금 입력 | 내역서 생성 | 파일 선택 · 폴더 선택 | 초기화 | — |
| 미수금 결과 | — | 내보내기 | 초기화 | — |
| 미수금 실행 기록 | — | 기록 내보내기 · 다시 실행 | — | — |
| 미수금 데이터 관리 | 법인 추가 | 저장 · 취소 | — | 이 법인 삭제 |
| 납품서 입력 | 선택 N건 납품서 생성 | 파일 선택 · 폴더 선택 · 전체 선택 | 선택 해제 | — |
| 견적서 입력 | 선택 N건 견적서 생성 | 파일 선택 · 폴더 선택 · 전체 선택 | 선택 해제 | — |
| 단가 데이터 관리 | 단가 추가 | 법인 추가 · 법인 수정 · 수정 | — | 법인 삭제 |
| 단가 계산 | 견적서 생성 | — | 초기화 | — |
| 다이얼로그 6종 | 확인측 (로그인·추가·변경·수정·선택) | 취소 | — | — |

미수금 결과·실행 기록 탭에 Primary가 없는 것은 의도다. 그 탭의 주 동작은 읽기다.

## C6. 표 규격

`CSageListCtrl`에 적용한다. 표를 쓰는 모든 화면이 이 규격을 공유한다.

- 행 높이 34 고정 (R6의 이미지리스트 방식), **헤더 높이 36** (`HDM_LAYOUT`)
- **선택 행**: 배경 `RGB(251,247,241)` + **좌측 3px 카멜 액센트 바**.
  Windows 기본 파란색·포커스 사각형은 `CDIS_SELECTED`/`CDIS_FOCUS`를 걷어내 차단한다.
  교대 행(`#FAF9F6`)과 배경이 거의 같으므로 **실제 구분 신호는 액센트 바다** — 빼면 선택이 안 보인다.
  선택 행에서도 미수금 열 카멜 강조를 유지한다 (목업 3-1 확인).
  **이 항목은 최초 계획에 빠져 있었다** — 스킬의 사용량 규칙에는 "표의 선택된 행"이 있었는데 Step에 옮기지 않았다
- 가로 hairline `LIST_GRID` 1px만. **세로선 없음** (R5)
- 헤더: 배경 `SURFACE_MUTED`, 텍스트 `TEXT_MUTED`, 세로 구분선 없음
- 교대 행 배경(`LIST_ROW_ALT`)은 **유지**한다. 문서가 제거를 지시하지 않았고 가로선과 역할이 다르다
- 금액 컬럼: 배경 없음, 우측 정렬 + Bold. **미수금 열 하나만** 색 텍스트
- 반복되는 법인명은 `〃`로 표기 (목업 3-1 · 3-8)
- 공백 행은 표에 넣지 않는다. `TaechangWorkflowResultPresenter`가 skip한다
- 법인이 바뀌는 지점에만 그룹 구분선

`CSageHeaderCtrl::OnPaint`의 `SetTextColor(TAECHANG_COLOR_BUTTON_TEXT)` (`SageHeaderCtrl.cpp:20`)를
반드시 함께 고친다. 배경만 밝게 하면 흰 글자가 흰 배경에 놓인다.

## C7. 공통 신규 요소 4종

`sagetaechang-ui`의 *새 UI 요소는 처음부터 공통 컨트롤로 만든다*를 따라 `app/ui/drawing/`에 만든다.
한 곳에서만 쓰여도 화면에 그리기 코드를 두지 않는다.

| 컨트롤 | 표현 | 쓰이는 화면 |
|---|---|---|
| `CSageSummaryBar` | 표 상단 밴드. `라벨 + 강조 수치 + 단위` 항목 N개 | 3-1(총 건수·미수금 합계·기타 처리) · 3-5(전체·성공·실패) · 3-6(36개 법인) · 3-3(단가 범위 4개) |
| `CSageEmptyState` | 표 영역 중앙에 안내문 + 1차 액션 버튼 | 3-3(등록된 단가 범위 없음) · 미수금 결과 미실행 상태 |
| `CSageInlineError` | 폼 하단 좌측, `ERROR` 색 1줄. 값이 비면 숨김 | 다이얼로그 6종 전부 |
| `CSageSelectionBar` | `N건 중 M건 선택됨` + 액션 버튼 묶음 | 3-7 · 3-8 |

`CSageSummaryBar`는 데이터를 문자열 배열로만 받는다.
업무 개념(워크플로 타입·미수금 여부)을 넘기지 않는다 — `sagetaechang-ui` > *컨트롤은 도메인 개념을 알면 안 된다*.

## C8. 아이콘 세트

16px · 1.5px 선 · 6종: 검색 / 새로고침 / 추가 / 삭제 / 더보기 / 내보내기.
현재 상태는 제각각이다 — 검색은 `CSageButton::DrawSearchIcon`의 GDI 원+선(2px),
초기화는 유니코드 글리프 `TAECHANG_UI_RESULT_RESET_BTN = L"↺"` (`TaechangDefine.h:232`),
법인 선택은 `TAECHANG_UI_CALC_COMPANY_PICK_LABEL = L"…"` 글자.

`CSageButton`의 `DrawSearchIcon` · `DrawCalculateIcon` · `DrawResetIcon` (`SageButton.cpp:64-133`, 70줄)을
16px·1.5px 기준으로 다시 그린다. 아이콘 단독 버튼에는 **툴팁을 필수로 붙인다**
(`sagetaechang-ui:229` "의미가 명확하지 않은 아이콘 단독 버튼은 사용하지 않는다"를 만족시키는 조건).

## C9. 크기의 동적 대응 (CRITICAL)

"크기를 비율로 맞춘다"는 요구가 두 갈래인데 **현재 코드의 상태가 서로 다르다.**
둘을 섞으면 검증이 불가능하므로 분리해서 다룬다.

### C9-1. 창 크기 변화(리사이즈) — **이미 되어 있다. 깨지 않는다**

| 확인한 것 | 위치 |
|---|---|
| `OnSize` → `LayoutChildControls()` 전면 재계산 | `SageTaechangView.cpp:584-589` |
| 표 컬럼 폭 비율 배분 — `MulDiv(column.nWidth, nWidth, nDefinedWidth)` | `SageTaechangView.cpp:577` |
| stretch 컬럼(남는 폭을 한 컬럼이 흡수) | `SageTaechangView.cpp:563`, `SageWorkflowColumn::bStretch` |
| 세로 중앙 정렬 계산 | `SageTaechangView.cpp:609`, `:847`, `:1888` |

**가로는 비율, 세로는 고정 높이 누적** — 업무 폼에서 맞는 방식이다. 이 구조를 유지한다.

디자인 작업이 지켜야 할 규칙:

- 새 요소(`CSageSummaryBar` · `CSageSelectionBar` · `CSageEmptyState` · `CSageInlineError`)의 좌표는
  **반드시 `LayoutChildControls` 또는 해당 패널의 `LayoutControls` 안에서 계산한다.**
  생성 시점 rect를 그대로 두는 요소를 만들지 않는다
- 요약 바·선택 바의 폭은 표 폭을 따라간다. 고정 폭을 주지 않는다
- 빈 상태 안내문은 표 영역 **중앙**이므로 표 rect에서 파생시킨다
- 표 컬럼 폭은 `SageWorkflowColumn`의 기존 비율 배분을 그대로 쓴다. 컬럼을 추가·제거할 때
  `nDefinedWidth` 합이 바뀌므로 나머지 컬럼 폭도 함께 다시 정한다

완료 기준: 창을 최소 크기에서 최대화까지 끌어도 새 요소가 표와 어긋나지 않는다.

### C9-2. DPI 배율 — **현재 대응이 전혀 없다. 이번 범위에서 분리한다**

조사 결과:

| 항목 | 현재 |
|---|---|
| manifest DPI awareness 설정 | **없음** (`SageTaechang.vcxproj`에 관련 설정 0건) |
| `GetDpiForWindow` / `GetDeviceCaps(LOGPIXELS*)` 호출 | **코드 전체 0곳** |
| 좌표 상수 | 전부 고정 px (28 · 32 · 80 · 34 …) |

지금은 프로세스가 DPI unaware이므로 150% 스케일 PC에서 Windows가 96dpi로 그린 뒤
**비트맵 확대**한다. 레이아웃은 깨지지 않고 대신 흐릿해진다.

**여기서 함정이 하나 있다.** `CreatePointFont`는 pt를 device caps로 px 변환하므로
**폰트만 DPI에 반응한다.** 지금은 시스템이 96을 보고해서 일관되지만,
DPI awareness를 켜는 순간 **폰트는 커지고 컨트롤 높이(고정 px)는 그대로**여서
32px 버튼 안에 안 들어가는 글자가 생긴다. 즉 awareness를 켜는 작업은
좌표 스케일링과 **반드시 같은 단위로 묶여야 한다.**

그래서 이번 디자인 작업에서는 다음만 한다.

- [ ] **이번에 정의하는 모든 크기 상수는 「96dpi 기준 논리 픽셀」로 간주한다.**
      값 자체는 목업 그대로(32 · 80 · 34 · 3 · 16) 두되, 이 문서와 스킬에 단위를 명시한다
- [ ] 좌표 계산에 **새로운 하드코딩 숫자를 만들지 않는다.** 전부 상수를 거친다
      (`coding-rules` 하드코딩 금지). 나중에 스케일 헬퍼 하나를 상수 참조 지점에 끼우면 되게 둔다

DPI awareness 전환 자체는 **Step D8**로 분리한다 (아래).

---

# Step D0 — `sagetaechang-ui` skill 개정

브랜치: `design/ui-skill-revision`
규칙 출처: `sagetaechang-plan` > *규칙을 고친 뒤에는 계획을 재점검한다*

이 계획이 스킬과 정면으로 충돌하는 항목이 있다. 스킬을 먼저 고치지 않으면 D1의 첫 줄이 규칙 위반이 된다.

## 이번 디자인 변경이 요구하는 개정

- [ ] **폰트 섹션(149-159)** — Gmarket Sans 기본 → Pretendard 기본, Gmarket은 로고 전용
- [ ] **폰트 결정(452-457)** — pt 값을 C2 확정값으로 갱신, 표 셀·캡션 2종 추가
- [ ] **색상 시스템(131-145)** — C1의 신규 토큰 5개 추가, 역할별 사용처 명시
- [ ] **버튼 변형 표(357-360)** — Secondary 재정의(카멜 테두리 → 중성 테두리), Ghost·Danger 추가
- [ ] **362행 "Text / Link형 버튼은 쓰지 않는다"** — Ghost 도입으로 삭제·개정
- [ ] **364-365행 "Danger 변형은 도입하지 않는다"** — 도입으로 개정. *"삭제 버튼 색을 바꾸는 것은 별도 UI 결정 항목"*이라 남긴 그 결정이 이 계획이다. 사유를 링크로 남긴다
- [ ] **버튼 결정(472-477)** — 2변형 서술을 4변형으로
- [ ] **에디트 높이(465)** — `TAECHANG_EDIT_HEIGHT = 28` → 32
- [ ] **229행 아이콘 규칙** — 아이콘 단독 버튼 허용 조건(16px 세트 + 툴팁 필수)으로 보강
- [ ] **신설: 표 규격** — C6 전체 (행 높이 · hairline · 금액 정렬 · `〃` · 공백 행)
- [ ] **신설: 공통 신규 요소** — C7 4종을 공통 컨트롤 목록(332-341 표)에 추가
- [ ] **신설: 버튼 위계 배분 원칙** — 화면당 Primary 1개, C5 배정표
- [ ] **디자인 검토 체크리스트(510-521)** — "화면당 Primary가 하나인가", "갈색이 강조에만 쓰였는가" 추가

## 이번 건과 무관하게 스킬이 실제와 어긋난 항목

발견했으므로 적어둔다. **고칠지는 별도 결정이다** (`CLAUDE.md` 3장 — 요청 범위 밖은 언급하고 지우지 않는다).

| 위치 | 스킬 내용 | 실제 |
|---|---|---|
| 135 | 구분선 `#E2DED6` | 코드는 `RGB(220,214,205)` = `#DCD6CD`. 문서 2장도 `#DCD6CD` — **스킬만 틀렸다** |
| 67-79 | 사이드바 그룹에 "검수(PDF·HWP 표지)" · "관리(설정·실행 기록·템플릿 관리)" | 검수 워크플로는 2026-08-04에 제거됨. 실제 그룹은 문서 생성 · 단가 관리 · 기타 |
| 87-92 | 문서 생성 탭 = 입력 / 미리보기 / 결과 / 실행 기록 | 실제는 입력 / 결과 / 실행 기록 / 데이터 관리 (`TAECHANG_UI_TAB_*`) |
| 94-99, 116-125, 200-206 | 검수 업무 화면·탭·컬럼 규격 | 해당 워크플로가 없다 |
| 193-198 | 결과 테이블 기본 컬럼 = 항목/값/상태/사유 | 미수금은 10컬럼(`RECEIVABLES_COL_*`). 4컬럼 표는 현재 도달 경로가 없다 |

완료 기준: 스킬의 폰트·색·버튼 서술이 D1~D2 계획과 한 군데도 어긋나지 않는다.
`REFACTORING_PLAN.md`를 재점검해 스킬 변경으로 무효가 된 Step이 없는지 확인한다.

---

# Step D1 — 토큰 · 서체 (우선순위 ①)

브랜치: `design/tokens-and-font`
전제: R1(타입 스케일) 결정 완료

- [ ] Pretendard TTF를 프로젝트에 추가하고 `AddFontResourceEx(FR_PRIVATE)` 로드·해제 (배치는 `coding-design`)
- [ ] C3 서체 상수 3개 (변경 2 · 신규 1)
- [ ] C2 타입 스케일 — 기존 4개 갱신 + 표 셀·캡션 2개 신규, `SageUiResources.cpp`에 폰트 2개 추가
- [ ] C1 색 토큰 — 변경 2 · 신규 7 · 제거 3
- [ ] `SageHeaderCtrl.cpp:13,20,26,36` — 배경 `SURFACE_MUTED`, 텍스트 `TEXT_MUTED`, 세로 구분선 제거
- [ ] `SageListCtrl.cpp:99-103` — 금액 컬럼 배경칠 제거. `SetHighlightColumns`는 **의미를 바꿔 재사용**한다(배경 → 텍스트 색 강조). 호출부 2곳(`SageTaechangView.cpp:467` · `SageResultTablePanel.cpp:236`)은 인자를 미수금 열 1개로 좁힌다
- [ ] `SageSidebarTree.cpp` — 선택 배경 `RGB(58,49,41)` + 좌측 3px 액센트 바
- [ ] 제거한 3개 상수를 참조하는 곳이 남지 않았는지 확인

완료 기준
- 표 헤더가 밝은 베이지 + 회색 글자로 보인다. **흰 글자가 남아 있으면 실패**
- 금액 3열의 노란 배경이 사라지고, 미수금 열만 색 텍스트다
- 갈색 채움 면이 화면에 Primary 버튼 1개 · 사이드바 선택 항목뿐이다
- Pretendard 미설치 PC에서도 같은 글꼴로 보인다

범위 밖
- 라운드 코너 — R3. GDI 특성이며 우회 비용이 이득보다 크다
- 버튼 4변형 — D2. 색 토큰이 먼저 있어야 한다

---

# Step D2 — 버튼 위계 (우선순위 ②)

브랜치: `design/button-hierarchy`

- [ ] `SageButton.h` — `SageButtonVariant`에 `SAGE_BUTTON_GHOST` · `SAGE_BUTTON_DANGER` 추가
- [ ] `SageButton.cpp:28-39` — Secondary 재정의(중성 테두리 + 본문색 텍스트) + 신규 2변형 그리기
- [ ] disabled·pressed 표현을 4변형 전부에 정의 (현재는 2변형 기준)
- [ ] C5 배정표대로 전 화면 `SetVariant()` 재지정
- [ ] 다이얼로그 7개는 이미 `CSageButton`을 쓴다 — 취소 버튼을 Secondary로 확정
- [ ] `SageTaechangView.cpp`의 `OnDrawItem`에 ID 나열 분기가 남아 있으면 제거 (`sagetaechang-ui:475`)

완료 기준
- 한 화면에 채움 버튼이 **하나**다. 단가 데이터 관리에서 4개가 동시에 채움이면 실패
- 삭제 계열 버튼만 적색 테두리다
- 부모의 `OnDrawItem`에 컨트롤 ID로 스타일을 가르는 식이 없다

범위 밖
- 목업의 30/34/36px 버튼 높이 — *어긋나는 지점* 2번대로 32 단일을 채택했다

---

# Step D3 — 컨트롤 높이 · 라벨 폭 (우선순위 ③)

브랜치: `design/control-metrics`

- [x] C4 상수 변경 — 높이 3개(D3a) · 라벨 폭 5개(D3b). `TAECHANG_LABEL_WIDTH`는 죽은 상수라 제외
- [x] 높이 28→32 · 24→32에 따라 세로 좌표 재계산 (`LayoutChildControls` · 각 패널·다이얼로그 `LayoutControls`)
- [x] `TAECHANG_BUTTON_TEXT_TOP_OFFSET = 2`는 Gmarket ascent 보정값이다. **Pretendard 기준으로 다시 재야 한다** → 0
- [x] 라벨별 실측으로 잘림 확인 (R9) — 잘리는 라벨 없음
- [ ] `TAECHANG_EDIT_TEXT_TOP_PAD`를 32px 기준으로 재조정 (계산값 8, 현재 9 — 1px 때문에 확인받은 화면을 흔들지 않으려 보류)
- [ ] 간격을 4의 배수로 정리

완료 기준
- 한 화면의 버튼·에디트·콤보 높이가 전부 32다
- 라벨이 `...`로 잘리는 곳이 없다
- 폼 라벨 폭이 64 또는 96이다

범위 밖
- `CSageEdit` 승격 — 테두리 배치 문제이며 `DEBT_LOG`에 별도 기록되어 있다. 높이만 바꾼다

---

# Step D4 — 표 렌더링

브랜치: `design/list-rendering`

- [ ] `CSageListCtrl` — 1×34 이미지리스트로 행 높이 고정 (R6)
- [ ] `LVS_EX_GRIDLINES` 4곳 제거 (R5) + `CDDS_ITEMPOSTPAINT`로 하단 1px hairline
- [ ] 금액 컬럼 우측 정렬 + Bold. 미수금 열만 색 텍스트
- [ ] `TaechangWorkflowResultPresenter` — 공백 행 skip
- [ ] 법인 변경 지점 그룹 구분선
- [ ] 반복 법인명 `〃` 표기

완료 기준
- 세로 격자선이 한 곳도 없다
- 24건 데이터가 24행으로 보인다 (현재는 `-` 행 때문에 40행처럼 보인다)
- 모든 표의 행 높이가 34다

범위 밖
- tabular 숫자 — R4. 우측 정렬로 대체한다
- 교대 행 배경 제거 — 문서가 지시하지 않았다

---

# Step D5 — 공통 신규 요소 (우선순위 ④)

브랜치: `design/common-elements`

- [ ] `CSageSummaryBar` · `CSageEmptyState` · `CSageInlineError` · `CSageSelectionBar` 4종 (배치는 `coding-design`)
- [ ] 다이얼로그 6종에 `CSageInlineError` 배치, 입력 검증 `AfxMessageBox`를 인라인으로 이관 (R7 — 검증분만)
- [ ] `sagetaechang-ui` 공통 컨트롤 목록에 4종 등재

완료 기준
- 4종 전부 `app/ui/drawing/`에 있고, 화면에는 선언·생성·값 설정 코드만 있다
- 컨트롤이 워크플로 타입을 인자로 받지 않는다
- 다이얼로그 입력 검증에서 모달 경고창이 뜨지 않는다

---

# Step D6 — 아이콘 세트 (우선순위 ⑤)

브랜치: `design/icon-set`

- [ ] C8의 6종을 16px · 1.5px로 통일
- [ ] `TAECHANG_UI_RESULT_RESET_BTN = L"↺"` · `TAECHANG_UI_INPUT_RESET_BTN` · `TAECHANG_UI_CALC_COMPANY_PICK_LABEL = L"…"` 글리프를 아이콘으로 교체
- [ ] 아이콘 단독 버튼에 툴팁 부착
- [ ] 아이콘 버튼 32×32 정사각

완료 기준: 한 화면의 아이콘 굵기·크기가 육안으로 같다. 유니코드 글리프 버튼이 남아 있지 않다.

---

# Step D7 — 화면별 적용 (문서 3장)

브랜치: 화면군별로 분리 — `design/screen-receivables` · `design/screen-price` · `design/screen-delivery-estimate` · `design/screen-dialogs`

전제: D1~D6 완료 + `REFACTORING_PLAN` 3-B-4a(패널 View 연결) 완료

각 화면은 **공통 규격 C1~C8을 배치**한다. 아래에는 그 화면에만 있는 것을 적는다.

## D7-1. 미수금 내역서 — 결과 (목업 3-1)

대상: `SageResultTablePanel` · `SageTaechangView`(결과 탭)

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 컬럼 10종 (`RECEIVABLES_COL_*`) | 표 상단 `CSageSummaryBar` — 총 24건 · 미수금 합계 4,382,610원 · 기타 처리 2건 |
| 법인명·담당자·품목명 필터 + 검색·초기화 (`FILTER_CRITERIA_*`) | 합계 행 (24건 · 4,655,610 · 273,000 · 4,382,610) |
| **요약 바에 넣을 데이터** — `RECEIVABLES_PREVIEW_TOTAL` · `RECEIVABLES_MISSING_COMPANIES` | 「내보내기」 버튼 (Secondary) |
| | 공백 행 skip · `〃` · 법인 그룹 구분선 (D4 규격 적용) |

**요약 바 데이터는 이미 계산되어 있다.** 지금은 상태바에 `완료 · 24건 처리` 한 줄로만 나온다.
새로 계산하는 게 아니라 표시 위치를 올리는 작업이다.

완료 기준: 표를 보지 않고 요약 바만으로 건수·미수금 합계·기타 처리 건수를 알 수 있다.
하단 상태바에는 진행·경로만 남는다 (진단 10).

## D7-2. 단가 계산 (목업 3-2)

대상: `SagePriceCalcPanel`

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 계산 결과 5줄 전부 (`CALC_PRINT/COVER/SUBTOTAL/FREIGHT/TOTAL_LABEL`) | 「적용 구간 · 100~200부 · 부수 단가 120원 / 표지 800원」 안내 1줄 |
| 최근 생성 내역 10컬럼 전부 (`CALC_HIST_COL_*`) | 「최근 10건」 배지 |
| 계산 조건 입력 폼 | 합계 줄만 강조 (나머지는 보조 텍스트) |

적용 구간 값은 단가 계산이 이미 내부에서 고른 구간이다. 화면에 노출하지 않고 있을 뿐이다.

완료 기준: 어느 단가 구간이 적용됐는지 화면에서 알 수 있다. 지금은 알 수 없다.

## D7-3. 단가 데이터 관리 (목업 3-3)

대상: `SagePriceManagePanel`

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 상세 정보 패널 전체 (`PRICE_DETAIL_HEADER` · `SUMMARY_COUNT_FMT` · `SUMMARY_RANGE_FMT`) | `CSageEmptyState` — 「등록된 단가 범위가 없습니다 / 법인을 선택한 뒤 부수 구간별 단가를 추가하세요」 + 「단가 추가」 |
| 법인 콤보 · 버튼 4개 | 버튼 위계 — 단가 추가=Primary, 법인 추가·수정=Secondary, **법인 삭제=Danger** |
| 단가 범위 표 (`PRICE_COL_*`) | 「관리」 열의 행내 「수정」 (Secondary) |
| `PRICE_SUMMARY_GUIDE` 안내 문구 | `PRICE_MAX_COPIES_NONE = L"-"` → 「제한 없음」 |

빈 상태가 진단 8의 사례다. 진입 시 40행 빈 격자가 나오는 화면이 이곳이다.

완료 기준: 법인 미선택 상태에서 빈 격자가 보이지 않고 무엇을 해야 하는지 문장으로 나온다.

## D7-4. 미수금 내역서 — 입력 · 실행 (목업 3-4)

대상: `SageTaechangView`(입력 탭) → 3-B-4b 이후 `SageWorkflowInputPanel`

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 입력 파일 · 저장 위치 · 파일/폴더 선택 | 「폴더 열기」 · 「결과 보기」 버튼 (완료 후) |
| 진행률 % (`PROGRESS_FORMAT`) · 처리 중 · 완료 | 진행 문구 세분화 — 「엑셀 데이터를 읽는 중입니다」 |
| 「내역서 생성」 · 초기화 | 완료 카드 — 「내역서 생성이 완료되었습니다 · 24건」 + 저장 경로 |

목업은 대기 → 처리 중 → 완료를 **한 영역에서 연속으로** 보여준다.
현재는 진행바가 실행 중에만 나타나고 완료 후 사라진다 (`sagetaechang-ui:492-495`).
이 규격을 바꾸는 것이므로 스킬의 *진행바 / 상태 표시* 절도 함께 고친다.

완료 기준: 완료 후에도 결과 요약과 저장 경로가 같은 자리에 남는다.
「예상 소요 · Esc 취소」는 취소 기능이 없으면 표시하지 않는다 — **없는 기능을 안내하지 않는다.**

## D7-5. 미수금 내역서 — 실행 기록 (목업 3-5)

대상: `SageTaechangView`(실행 기록 탭) → `SageWorkflowHistoryPanel`

**9세트 중 가장 무겁다.** 텍스트 로그를 표로 바꾸는 작업이다.

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 기록 데이터 전부 — 시각 · 결과 · 입력 파일 · 저장 경로 · 원인 | `CSageListCtrl` 5컬럼 표로 전환 |
| 텍스트 조립 상수 (`HISTORY_ENTRY_PREFIX` · `LINE_BREAK` · `FIELD_INDENT` · `INPUT_PREFIX` …) | `CSageSummaryBar` — 전체 12 · 성공 11 · 실패 1 |
| 성공/실패 구분 (`HISTORY_SUCCESS` / `FAILED`) | 「열기」 · 「다시 실행」 행내 버튼 |
| | 「기록 내보내기」 (Secondary) |
| | 실패 행의 원인을 셀에 표시 (`원인 · 필수 컬럼 «입금 은행»을 찾을 수 없습니다`) |

표로 바꾸면 위 텍스트 조립 상수 8개가 쓰이지 않게 된다.
**내 변경이 만든 orphan이므로 함께 제거한다** (`CLAUDE.md` 3장).

완료 기준: 실행 기록에서 성공/실패를 스크롤 없이 세어볼 수 있다.
실패 원인이 로그를 읽지 않고 셀에서 보인다.

## D7-6. 미수금 내역서 — 데이터 관리 (목업 3-6)

대상: `SageTaechangView`(데이터 관리 탭) → `SageCompanyOrderPanel`

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 법인 출력 순서 CRUD (`CO_*` 상수 18개) | 「이동」 열의 ↑↓ 버튼 (Ghost) |
| 선택 항목 편집 — 순서·법인명 + 저장·취소 | `CSageSummaryBar` — 「36개 법인」 |
| 삭제 (`CO_DELETE_CONFIRM_FMT`) | 하단 안내 — 「여기서 정한 순서대로 출력됩니다. 목록에 없는 법인은 «기타»로 마지막에 묶입니다」 |
| 순서 중복 검증 (`CO_ORDER_DUPLICATE`) | 「이 법인 삭제」를 Danger로 |

**↑↓ 이동을 넣으면 순서 입력창의 역할이 겹친다.** 목업은 순서 열을 읽기 전용 번호로 두고
편집 패널에는 법인명만 남긴다. 그러면 `CO_ORDER_DUPLICATE` 검증이 불필요해진다 —
↑↓로는 중복이 생길 수 없다. **이 검증 제거는 별도 확인을 받는다** (동작 변화이므로).

드래그 리오더는 넣지 않는다 — 목업도 ↑↓ 버튼만 그렸다.

완료 기준: 순서를 바꾸는 데 숫자를 입력하지 않는다.

## D7-7. 납품서 생성 — 미리보기 · 행 선택 (목업 3-7)

대상: `SageResultTablePanel`(입력 표 인스턴스)

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 체크박스 열 (`LVS_EX_CHECKBOXES`, `SageTaechangView.cpp:463`) | `CSageSelectionBar` — 「6건 중 3건 선택됨」 |
| 「전체 선택」 (`SELECT_ALL_BUTTON`) | 「선택 해제」 (Ghost) |
| 컬럼 11종 (`DELIVERY_COL_*`) | 「선택 3건 납품서 생성」 — 선택 건수를 버튼 문구에 반영 |
| 선택 행 검증 (`DELIVERY_SELECT_ROW_REQUIRED`) | 0건이면 생성 버튼 비활성 (검증 대신 비활성화 — `sagetaechang-ui:175`) |

완료 기준: 몇 건을 선택했는지 표를 세지 않고 알 수 있다. 0건에서 생성 버튼을 누를 수 없다.

## D7-8. 견적서 생성 — 미리보기 · 한 페이지 작성 (목업 3-8)

대상: `SageResultTablePanel`(입력 표 인스턴스)

| 이미 있는 것 | 새로 할 것 |
|---|---|
| 체크박스 · 전체 선택 · 컬럼 9종 (`ESTIMATE_COL_*`) | 「한 페이지 작성」 옆에 「· 최대 6행」 상시 표시 |
| 「한 페이지 작성」 체크박스 (`ESTIMATE_ONE_PAGE_CHECK`) | `CSageSelectionBar` — 「26건 중 4건 선택됨」 |
| **6행 초과 시 자동 해제까지 구현됨** (`SageTaechangView.cpp:1268-1290`) | 초과 안내를 `MessageBox`(`ESTIMATE_ONE_PAGE_LIMIT`) → 인라인 |

문서의 의도는 "6행 초과 오류를 **사전 차단**"이다. 제한을 미리 보여주면 경고창이 뜰 일이 없다.
자동 해제 로직은 이미 있으므로 안내 위치만 바꾼다.

완료 기준: 「최대 6행」을 누르기 전에 안다. 초과로 모달 경고창이 뜨지 않는다.

## D7-9. 다이얼로그 6종 (목업 3-9)

| 목업 | 클래스 | 파일 |
|---|---|---|
| 로그인 | `TaechangLoginDlg` | `TaechangLoginDlg.cpp` |
| 비밀번호 변경 | `TaechangPasswordChangeDlg` | `TaechangPasswordChangeDlg.cpp` |
| 단가 추가 | `TaechangPriceRangeDlg` | `TaechangPriceRangeDlg.cpp` |
| 법인 추가 | `TaechangCompanyDlg` | `TaechangCompanyDlg.cpp` |
| 법인 수정 | `TaechangCompanyRenameDlg` | **`TaechangPriceSimpleDlg.cpp`** (파일명과 클래스명 불일치 — 기존 상태, 이번에 고치지 않음) |
| 법인 선택 | `TaechangCalcCompanyPickerDlg` | `TaechangCalcCompanyPickerDlg.cpp` |

### 6종 공통 (한 번에 적용)

- [ ] `CSageInlineError`를 폼 하단 좌측에 배치. 입력 검증을 `AfxMessageBox` → 인라인으로 (R7)
- [ ] 확인측 버튼 = Primary, 취소 = Secondary(중성 테두리). 6개 다이얼로그 전부 이미 `CSageButton`을 쓴다
- [ ] 라벨 폭 — 로그인·단가 추가·법인 추가·법인 수정 **64**, 비밀번호 변경 **96**
- [ ] 라벨 색 `TEXT_MUTED`, 컨트롤 높이 32
- [ ] 6개 모두 `CreateControls` / `LayoutControls` / `ApplyFont` 같은 구조다 — **같은 패턴으로 한 번에 바꾼다**

### 다이얼로그별로 다른 것

| 다이얼로그 | 변경 |
|---|---|
| 로그인 | 실패 문구(`LOGIN_FAILED` · `EMPTY_ID` · `EMPTY_PW`) 3개를 인라인으로 |
| 비밀번호 변경 | 라벨 문구 — 「변경할 비밀번호」→「새 비밀번호」, 「비밀번호 확인」→「새 비밀번호 확인」. 상시 힌트 「영문 · 숫자 4~15자」 추가. 검증 문구 8개를 인라인으로 |
| 단가 추가 | 「최대부수 없음」·「단일 부수」 체크박스는 이미 있다. `RANGE_OVERLAP` 등 검증 4개를 인라인으로 |
| 법인 추가 | 상시 힌트 「한글 20자 / 영문 40자 이내」. `COMPANY_EXISTS` · `TOO_LONG_KO/EN`을 인라인으로 |
| 법인 수정 | 안내 「이 법인에 등록된 단가 N건은 그대로 유지됩니다」 **신규** — 삭제와의 차이를 알려준다 |
| 법인 선택 | 검색 결과 카운터 「36개 중 18개 일치」 **신규**. `PICKER_SEARCH_CUE` 검색은 이미 동작한다 |

### 목업에 없는 다이얼로그

`TaechangCalcEstimateDlg`(견적서 생성) · 표지 수정 다이얼로그는 목업 6종에 없다.
**공통 규격(C3·C4·C5)만 적용하고 구조는 바꾸지 않는다.** 적용하지 않으면 이 두 개만 옛 톤으로 남는다.

---

# Step D8 — DPI 배율 대응

브랜치: `feature/dpi-awareness`
전제: D1~D7 완료. **디자인이 96dpi에서 확정된 뒤에 한다**

디자인 작업과 섞지 않는 이유: 둘 다 화면 크기를 바꾸는 작업이다. 동시에 하면
"목업과 다른 것"이 디자인 미적용인지 DPI 스케일 오류인지 구분할 수 없다.

- [ ] manifest에 DPI awareness 선언 (Per-Monitor V2 우선 검토)
- [ ] `SageUiMetrics` 성격의 스케일 변환 지점 신설 — 논리 px → 실제 px (`MulDiv(값, nDpi, 96)`)
- [ ] 크기 상수 참조 지점을 스케일 변환에 통과시킨다. **상수 값 자체는 96dpi 논리값으로 유지**
- [ ] `WM_DPICHANGED` 처리 (모니터 간 이동)
- [ ] 폰트를 `CreatePointFont` → DPI 인지 생성으로 전환 (R11의 폰트/좌표 불일치 해소)
- [ ] 아이콘을 배율별로 준비하거나 벡터 드로잉 유지 여부 결정

완료 기준
- 100% / 125% / 150% / 200% 스케일에서 글자가 버튼·에디트를 넘치지 않는다
- 라벨이 잘리지 않는다
- 고DPI에서 흐릿하지 않다 (현재는 비트맵 확대로 흐릿하다)

범위 밖
- 사용자 지정 배율 설정 UI — OS 배율을 따른다

---

## 검증 방법

1. **빌드는 사용자가 직접 확인한다** (Claude는 MSBuild를 실행하지 않음)
2. **화면 표시가 바뀌어야 성공이다.** 리팩토링과 반대 기준이다. 안 바뀌면 적용되지 않은 것이다
3. Step별 검증 지점
   - D1 — 표 헤더 · 금액 컬럼 · 사이드바 선택. 화면 1개만 봐도 판정된다
   - D2 — 화면 9개를 돌며 채움 버튼을 센다. 화면당 1개가 아니면 실패
   - D3 — 라벨 잘림. 다이얼로그 6종 전부 열어본다
   - D4 — 미수금 결과 표에서 세로선 없음 + 24행 확인
   - D5 — 다이얼로그 6종에서 일부러 잘못된 값을 넣어 모달 경고창이 뜨지 않는지 확인
   - D7 — 화면군별로 진입 · 입력 · 실행 · 결과 · 전환 1회씩
4. **목업과 나란히 놓고 본다.** 문서 3장의 화면 캡처가 기준이다
5. R3(각진 모서리) · R4(tabular)는 차이로 인정하고 실패로 보지 않는다

`git-workflow` 준수: Step별 별 브랜치, 커밋은 목적 단위. 커밋 타입은 `refactor:`가 아니라
**`style:`**(표현만) / **`feat:`**(D5·D7의 신규 요소)를 쓴다. `docs/decisions/PR_LOG.md`에 기록한다.

---

## 범위 밖 (이번 계획에서 하지 않는 것)

| 항목 | 사유 |
|---|---|
| 라운드 코너 · 그림자 | R3. GDI 특성. GDI+ 도입은 별 결정 |
| tabular 숫자 | R4. 문서가 지시한 우측 정렬 + Bold로 대체 |
| 드래그 순서 변경 | 목업도 ↑↓ 버튼만 그렸다 |
| `CSageEdit` 승격 | 테두리 배치 문제. `DEBT_LOG`에 기록됨 |
| 앱 배경 `#F8F6F1` → `#F7F6F1` | R8. 육안 차이 없음 |
| 스킬의 검수 워크플로 · 탭 구성 · 결과 컬럼 서술 | D0의 *무관한 어긋남* 표. 이번 디자인 변경과 무관하며 별 결정 |
| 「예상 소요 · Esc 취소」 안내 | 취소 기능이 없다. 없는 기능을 안내하지 않는다 |
| 교대 행 배경 제거 | 문서가 지시하지 않았다 |
| DPI awareness 전환 | **Step D8로 분리.** 디자인과 같이 하면 목업과의 차이가 디자인 미적용인지 스케일 오류인지 구분되지 않는다 (R11) |
| 리사이즈 비율 구조 변경 | C9-1대로 **이미 되어 있다.** 새 요소를 그 구조에 넣기만 한다 |
