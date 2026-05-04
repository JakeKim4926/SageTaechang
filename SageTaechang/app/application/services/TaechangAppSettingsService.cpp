#include "pch.h"
#include "app/application/services/TaechangAppSettingsService.h"

namespace
{
    constexpr LPCWSTR TAECHANG_SETTINGS_FILE_NAME = L"settings.ini";
    constexpr LPCWSTR TAECHANG_SETTINGS_TOOLS_SECTION = L"Tools";
    constexpr LPCWSTR TAECHANG_SETTINGS_PDFTOTEXT_KEY = L"pdftotextPath";
    constexpr LPCWSTR TAECHANG_PDFTOTEXT_EXE_NAME = L"pdftotext.exe";

    BOOL PathFileExists(const CString& strPath)
    {
        DWORD dw = GetFileAttributesW(strPath);
        return (dw != INVALID_FILE_ATTRIBUTES && !(dw & FILE_ATTRIBUTE_DIRECTORY)) ? TRUE : FALSE;
    }
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
    // 앱 실행 파일 옆에 pdftotext.exe가 있으면 자동 사용
    CString strBundled = GetDefaultPdfToTextPath();
    if (PathFileExists(strBundled))
    {
        outSettings.m_strPdfToTextPath = strBundled;
        return TRUE;
    }

    // 기존 사용자를 위한 ini 파일 폴백
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
    CString strFolder = GetExecutableFolder();
    if (!strFolder.IsEmpty() && strFolder.Right(1) != L"\\")
        strFolder += L"\\";
    return strFolder + TAECHANG_PDFTOTEXT_EXE_NAME;
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
