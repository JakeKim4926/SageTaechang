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
- **결과**: pending
