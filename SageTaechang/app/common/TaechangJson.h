#pragma once

std::string WideToUtf8(const CString& strWide);
CString Utf8ToWide(const std::string& strUtf8);
CString JsonExtractString(const CString& strJson, const CString& strKey);
BOOL JsonExtractBool(const CString& strJson, const CString& strKey);
CString JsonEscapeString(const CString& strValue);
HWND GetAppMainWindow();

// IFileDialog::Show()를 SEH로 감싸 버그 있는 쉘 익스텐션 AV를 방지한다
HRESULT SafeShowDialog(IFileDialog* pDialog, HWND hOwner);

// IFileOpenDialog 기반 파일 선택 (단일 / 다중)
// GetOpenFileNameW 대신 사용: 레거시 쉘 익스텐션 버그 회피
BOOL ShowIFileOpenDialog(
    HWND hOwner,
    LPCWSTR pszTitle,
    LPCWSTR pszDefExt,
    const COMDLG_FILTERSPEC* paTypes,
    UINT nTypes,
    BOOL bMultiSelect,
    std::vector<CString>& outPaths);

// IFileSaveDialog 기반 저장 경로 선택
CString ShowIFileSaveDialog(
    HWND hOwner,
    LPCWSTR pszTitle,
    LPCWSTR pszDefExt,
    const COMDLG_FILTERSPEC* paTypes,
    UINT nTypes,
    LPCWSTR pszInitialName);
