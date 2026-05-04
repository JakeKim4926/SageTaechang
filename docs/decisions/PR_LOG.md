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
