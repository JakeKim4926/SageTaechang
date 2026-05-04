#include "pch.h"
#include "app/application/services/TaechangAppSettingsService.h"
#include "app/common/TaechangFileUtils.h"

namespace
{
    constexpr LPCWSTR TAECHANG_SETTINGS_FILE_NAME = L"settings.ini";
    constexpr LPCWSTR TAECHANG_SETTINGS_TOOLS_SECTION = L"Tools";
    constexpr LPCWSTR TAECHANG_SETTINGS_PDFTOTEXT_KEY = L"pdftotextPath";
    constexpr LPCWSTR TAECHANG_PDFTOTEXT_EXE_NAME = L"pdftotext.exe";
}

CString TaechangAppSettingsService::GetSettingsPath() const
{
    CString strFolder;
    GetExecutableDirectory(strFolder);
    if (!strFolder.IsEmpty() && strFolder.Right(1) != L"\\")
        strFolder += L"\\";
    return strFolder + TAECHANG_SETTINGS_FILE_NAME;
}

BOOL TaechangAppSettingsService::Load(TaechangAppSettings& outSettings) const
{
    CString strBundled = GetDefaultPdfToTextPath();
    if (FileExists(strBundled))
    {
        outSettings.m_strPdfToTextPath = strBundled;
        return TRUE;
    }

    wchar_t szValue[MAX_PATH * 4] = {};
    GetPrivateProfileStringW(
        TAECHANG_SETTINGS_TOOLS_SECTION,
        TAECHANG_SETTINGS_PDFTOTEXT_KEY,
        L"",
        szValue,
        static_cast<DWORD>(_countof(szValue)),
        GetSettingsPath());

    outSettings.m_strPdfToTextPath = szValue;
    outSettings.m_strPdfToTextPath.Trim();
    return TRUE;
}

CString TaechangAppSettingsService::GetDefaultPdfToTextPath() const
{
    CString strFolder;
    GetExecutableDirectory(strFolder);
    if (!strFolder.IsEmpty() && strFolder.Right(1) != L"\\")
        strFolder += L"\\";
    return strFolder + TAECHANG_PDFTOTEXT_EXE_NAME;
}
