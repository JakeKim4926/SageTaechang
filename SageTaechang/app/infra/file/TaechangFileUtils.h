#pragma once

BOOL FileExists(const CString& strPath);
BOOL FolderExists(const CString& strPath);
BOOL GetExecutableDirectory(CString& outDirectory);
CString CombinePath(const CString& strDirectory, const CString& strRelativePath);
CString BuildTempJsonPath(LPCWSTR pszPrefix);
CString QuoteArgument(const CString& strValue);
BOOL RunProcessAndWait(const CString& strCommandLine, DWORD& outExitCode, CString& strError);
