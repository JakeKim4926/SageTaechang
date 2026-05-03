# SageTaechang UI Progress

## 진행 완료

- SageNexus의 Gmarket Sans TTF 3종을 SageTaechang 리소스로 추가했다.
- 앱 시작 시 private font로 Gmarket Sans를 로드하고 MFC 컨트롤에 적용했다.
- `feature/gmarket-fonts` PR을 `develop`에 squash merge했다.
- SageTaechang UI 방향을 ERP형 업무 도구로 정했다.
- 로컬 skill `sagetaechang-ui`를 만들어 UI 작업 기준을 남겼다.
- `feature/erp-workflow-ui` 브랜치를 만들었다.
- 기존 워크플로우 콤보 중심 화면을 좌측 업무 메뉴 + 우측 작업 영역 구조로 개편했다.
- 문서 생성 업무와 PDF/HWP 검수 업무의 탭 구성을 분리했다.
- 탭 선택에 따라 입력, 결과, 상세, CSV 내보내기 영역이 전환되도록 했다.
- 검수 업무에서는 출력 폴더와 미리보기 버튼을 숨기고 검수 실행 중심으로 정리했다.
- 결과 테이블 컬럼 폭을 화면 폭에 맞춰 조정하도록 했다.
- `Debug|x64` 빌드를 통과했다.
- UI 개편 1차 커밋을 만들었다: `d8ca818 feat: ERP형 업무 UI 구조 적용`

## 다음 진행

- 실제 실행 화면을 띄워 레이아웃 겹침, 여백, 탭 높이, 버튼 밀도를 확인한다.
- 사이드 메뉴 선택 상태가 더 명확하게 보이도록 시각 피드백을 보강한다.
- 아이보리/화이트/실버 베이지/카멜 브라운 팔레트를 더 일관되게 적용한다.
- 검수 결과 요약 카운터를 추가할지 검토한다.
- 실행 기록 탭은 현재 자리만 있으므로 실제 기록 기능과 연결할지 별도 작업으로 판단한다.
- 화면 확인 후 `feature/erp-workflow-ui` PR을 `develop` 대상으로 생성한다.
