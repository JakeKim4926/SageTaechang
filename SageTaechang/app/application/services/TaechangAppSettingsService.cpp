#include "pch.h"
#include "app/application/services/TaechangAppSettingsService.h"

namespace
{
    constexpr LPCWSTR TAECHANG_SETTINGS_FILE_NAME = L"settings.ini";
    constexpr LPCWSTR TAECHANG_SETTINGS_TOOLS_SECTION = L"Tools";
    constexpr LPCWSTR TAECHANG_SETTINGS_PDFTOTEXT_KEY = L"pdftotextPath";
    constexpr LPCWSTR TAECHANG_DEFAULT_PDFTOTEXT_PATH = L"C:\\Program Files\\Git\\mingw64\\bin\\pdftotext.exe";
}

CString TaechangAppSettingsService::GetSettingsPath() const
{
    CString strFolder = GetExecutableFolder();
    if (!strFolder.IsEmpty() && strFolder.Right(1) != L"\\")
        strFolder += L"\\";
    return strFolder + TAECHANG_SETTINGS_FILE_NAME;
}

BOOL TaechangAppSettingsService::Load(TaechangAppSettings& outSettings) const
{
    wchar_t szValue[MAX_PATH * 4] = {};
    GetPrivateProfileStringW(
        TAECHANG_SETTINGS_TOOLS_SECTION,
        TAECHANG_SETTINGS_PDFTOTEXT_KEY,
        TAECHANG_DEFAULT_PDFTOTEXT_PATH,
        szValue,
        static_cast<DWORD>(_countof(szValue)),
        GetSettingsPath());

    outSettings.m_strPdfToTextPath = szValue;
    outSettings.m_strPdfToTextPath.Trim();
    if (outSettings.m_strPdfToTextPath.IsEmpty())
        outSettings.m_strPdfToTextPath = TAECHANG_DEFAULT_PDFTOTEXT_PATH;
    return TRUE;
}

BOOL TaechangAppSettingsService::Save(const TaechangAppSettings& settings, CString& strError) const
{
    if (!WritePrivateProfileStringW(
        TAECHANG_SETTINGS_TOOLS_SECTION,
        TAECHANG_SETTINGS_PDFTOTEXT_KEY,
        settings.m_strPdfToTextPath,
        GetSettingsPath()))
    {
        strError.Format(L"설정 파일을 저장할 수 없습니다. error=%lu", GetLastError());
        return FALSE;
    }
    return TRUE;
}

CString TaechangAppSettingsService::GetDefaultPdfToTextPath() const
{
    return TAECHANG_DEFAULT_PDFTOTEXT_PATH;
}

CString TaechangAppSettingsService::GetExecutableFolder() const
{
    wchar_t szPath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    CString strPath = szPath;
    int nSlash = strPath.ReverseFind(L'\\');
    if (nSlash >= 0)
        return strPath.Left(nSlash);
    return L".";
}
