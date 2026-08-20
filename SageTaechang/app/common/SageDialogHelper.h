#pragma once

HRESULT SafeShowDialog(IFileDialog* pDialog, HWND hOwner);

BOOL ShowIFileOpenDialog(
    HWND hOwner,
    LPCWSTR pszTitle,
    LPCWSTR pszDefExt,
    const COMDLG_FILTERSPEC* paTypes,
    UINT nTypes,
    BOOL bMultiSelect,
    std::vector<CString>& outPaths);

HWND GetAppMainWindow();
