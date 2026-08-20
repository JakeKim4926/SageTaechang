#include "pch.h"
#include "app/infra/file/SageAppSettingsService.h"
#include "app/infra/file/SageFileUtils.h"

namespace
{
	constexpr LPCWSTR SAGE_SETTINGS_FILE_NAME = L"settings.ini";
	constexpr LPCWSTR SAGE_SETTINGS_TOOLS_SECTION = L"Tools";
	constexpr LPCWSTR SAGE_SETTINGS_PDFTOTEXT_KEY = L"pdftotextPath";
	constexpr LPCWSTR SAGE_PDFTOTEXT_EXE_NAME = L"pdftotext.exe";
}

CString SageAppSettingsService::GetSettingsPath() const {
	CString strFolder;
	GetExecutableDirectory(strFolder);
	if (!strFolder.IsEmpty() && strFolder.Right(1) != L"\\")
		strFolder += L"\\";
	return strFolder + SAGE_SETTINGS_FILE_NAME;
}

BOOL SageAppSettingsService::Load(SageAppSettings& outSettings) const {
	CString strBundled = GetDefaultPdfToTextPath();
	if (FileExists(strBundled)) {
		outSettings.m_strPdfToTextPath = strBundled;
		return TRUE;
	}

	wchar_t szValue[MAX_PATH * 4] = {};
	GetPrivateProfileStringW(
		SAGE_SETTINGS_TOOLS_SECTION,
		SAGE_SETTINGS_PDFTOTEXT_KEY,
		L"",
		szValue,
		static_cast<DWORD>(_countof(szValue)),
		GetSettingsPath());

	outSettings.m_strPdfToTextPath = szValue;
	outSettings.m_strPdfToTextPath.Trim();
	return TRUE;
}

CString SageAppSettingsService::GetDefaultPdfToTextPath() const {
	CString strFolder;
	GetExecutableDirectory(strFolder);
	if (!strFolder.IsEmpty() && strFolder.Right(1) != L"\\")
		strFolder += L"\\";
	return strFolder + SAGE_PDFTOTEXT_EXE_NAME;
}
