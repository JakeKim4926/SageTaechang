#include "pch.h"
#include "TaechangDefine.h"
#include "app/infra/file/TaechangFileUtils.h"

BOOL FileExists(const CString& strPath) {
	DWORD dwAttr = GetFileAttributesW(strPath);
	return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0) ? TRUE : FALSE;
}

BOOL FolderExists(const CString& strPath) {
	DWORD dwAttr = GetFileAttributesW(strPath);
	return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0) ? TRUE : FALSE;
}

BOOL GetExecutableDirectory(CString& outDirectory) {
	wchar_t szPath[MAX_PATH] = {};
	DWORD dwLen = GetModuleFileNameW(NULL, szPath, MAX_PATH);
	if (dwLen == 0 || dwLen >= MAX_PATH)
		return FALSE;

	CString strPath = szPath;
	int nSlash = strPath.ReverseFind(L'\\');
	if (nSlash < 0)
		return FALSE;

	outDirectory = strPath.Left(nSlash);
	return TRUE;
}

CString CombinePath(const CString& strDirectory, const CString& strRelativePath) {
	CString strResult = strDirectory;
	if (!strResult.IsEmpty() && strResult[strResult.GetLength() - 1] != L'\\')
		strResult += L"\\";
	strResult += strRelativePath;
	return strResult;
}

CString BuildTempJsonPath(LPCWSTR pszPrefix) {
	wchar_t szTempPath[MAX_PATH] = {};
	wchar_t szTempFile[MAX_PATH] = {};
	GetTempPathW(MAX_PATH, szTempPath);
	GetTempFileNameW(szTempPath, pszPrefix, 0, szTempFile);

	CString strPath = szTempFile;
	int nDot = strPath.ReverseFind(L'.');
	if (nDot >= 0)
		strPath = strPath.Left(nDot);
	strPath += L".json";
	return strPath;
}

CString QuoteArgument(const CString& strValue) {
	CString strEscaped = strValue;
	strEscaped.Replace(L"\"", L"\\\"");
	return L"\"" + strEscaped + L"\"";
}

BOOL RunProcessAndWait(const CString& strCommandLine, DWORD& outExitCode, CString& strError) {
	STARTUPINFOW si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	CString strMutableCommandLine = strCommandLine;
	BOOL bCreated = CreateProcessW(NULL, strMutableCommandLine.GetBuffer(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	strMutableCommandLine.ReleaseBuffer();

	if (!bCreated) {
		strError.Format(L"프로세스 실행에 실패했습니다. error=%lu", GetLastError());
		return FALSE;
	}

	DWORD dwWait = WaitForSingleObject(pi.hProcess, TAECHANG_PROCESS_TIMEOUT_MS);
	if (dwWait == WAIT_TIMEOUT) {
		TerminateProcess(pi.hProcess, 1);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		strError = L"처리 시간이 초과되었습니다.";
		return FALSE;
	}

	if (!GetExitCodeProcess(pi.hProcess, &outExitCode))
		outExitCode = 1;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return TRUE;
}
