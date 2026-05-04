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
- **결과**: pending
