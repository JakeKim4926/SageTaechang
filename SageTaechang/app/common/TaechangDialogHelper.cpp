#include "pch.h"
#include "app/common/TaechangDialogHelper.h"

HRESULT SafeShowDialog(IFileDialog* pDialog, HWND hOwner) {
	__try {
		return pDialog->Show(hOwner);
	} __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
			   ? EXCEPTION_EXECUTE_HANDLER
			   : EXCEPTION_CONTINUE_SEARCH) {
		return E_FAIL;
	}
}

BOOL ShowIFileOpenDialog(
	HWND hOwner,
	LPCWSTR pszTitle,
	LPCWSTR pszDefExt,
	const COMDLG_FILTERSPEC* paTypes,
	UINT nTypes,
	BOOL bMultiSelect,
	std::vector<CString>& outPaths) {
	IFileOpenDialog* pDialog = nullptr;
	if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
								IID_PPV_ARGS(&pDialog))))
		return FALSE;

	DWORD dwOptions = 0;
	pDialog->GetOptions(&dwOptions);
	dwOptions |= FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST;
	if (bMultiSelect)
		dwOptions |= FOS_ALLOWMULTISELECT;
	pDialog->SetOptions(dwOptions);

	if (nTypes > 0 && paTypes)
		pDialog->SetFileTypes(nTypes, paTypes);
	if (pszDefExt && *pszDefExt)
		pDialog->SetDefaultExtension(pszDefExt);
	if (pszTitle && *pszTitle)
		pDialog->SetTitle(pszTitle);

	BOOL bResult = FALSE;
	if (SUCCEEDED(SafeShowDialog(pDialog, hOwner))) {
		IShellItemArray* pItems = nullptr;
		if (SUCCEEDED(pDialog->GetResults(&pItems)) && pItems) {
			DWORD nCount = 0;
			pItems->GetCount(&nCount);
			for (DWORD i = 0; i < nCount; ++i) {
				IShellItem* pItem = nullptr;
				if (SUCCEEDED(pItems->GetItemAt(i, &pItem)) && pItem) {
					LPWSTR pszPath = nullptr;
					if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
						outPaths.push_back(CString(pszPath));
						CoTaskMemFree(pszPath);
					}
					pItem->Release();
				}
			}
			pItems->Release();
			bResult = !outPaths.empty() ? TRUE : FALSE;
		}
	}
	pDialog->Release();
	return bResult;
}

CString ShowIFileSaveDialog(
	HWND hOwner,
	LPCWSTR pszTitle,
	LPCWSTR pszDefExt,
	const COMDLG_FILTERSPEC* paTypes,
	UINT nTypes,
	LPCWSTR pszInitialName) {
	IFileSaveDialog* pDialog = nullptr;
	if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL,
								IID_PPV_ARGS(&pDialog))))
		return L"";

	DWORD dwOptions = 0;
	pDialog->GetOptions(&dwOptions);
	pDialog->SetOptions(dwOptions | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT);

	if (nTypes > 0 && paTypes)
		pDialog->SetFileTypes(nTypes, paTypes);
	if (pszDefExt && *pszDefExt)
		pDialog->SetDefaultExtension(pszDefExt);
	if (pszTitle && *pszTitle)
		pDialog->SetTitle(pszTitle);
	if (pszInitialName && *pszInitialName)
		pDialog->SetFileName(pszInitialName);

	CString strResult;
	if (SUCCEEDED(SafeShowDialog(pDialog, hOwner))) {
		IShellItem* pItem = nullptr;
		if (SUCCEEDED(pDialog->GetResult(&pItem)) && pItem) {
			LPWSTR pszPath = nullptr;
			if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath) {
				strResult = pszPath;
				CoTaskMemFree(pszPath);
			}
			pItem->Release();
		}
	}
	pDialog->Release();
	return strResult;
}

HWND GetAppMainWindow() {
	struct EnumCtx { DWORD dwPid; HWND hFound; };
	EnumCtx ctx = { GetCurrentProcessId(), nullptr };

	EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
		EnumCtx* pCtx = reinterpret_cast<EnumCtx*>(lParam);
		DWORD dwPid = 0;
		GetWindowThreadProcessId(hWnd, &dwPid);
		if (dwPid == pCtx->dwPid &&
			IsWindowVisible(hWnd) &&
			GetWindow(hWnd, GW_OWNER) == nullptr) {
			pCtx->hFound = hWnd;
			return FALSE;
		}
		return TRUE;
	}, reinterpret_cast<LPARAM>(&ctx));

	return ctx.hFound;
}
