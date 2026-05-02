#include "pch.h"
#include "app/application/services/TaechangReceivablesExcelService.h"
#include "app/common/TaechangJson.h"
#include "app/infrastructure/bridge/TaechangBridgeResponse.h"
#include <shobjidl.h>

namespace
{
    constexpr int TAECHANG_RECEIVABLES_TIMEOUT_MS = 600000;
    constexpr const wchar_t* TAECHANG_RECEIVABLES_LOAD_SCRIPT_PATH = L"tools\\load-receivables-data.ps1";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_GEN_SCRIPT_PATH = L"tools\\generate-receivables-form.ps1";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_TEMPLATE_PATH = L"templates\\receivables-template.xls";
    constexpr const wchar_t* TAECHANG_POWERSHELL_PATH = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_ERR_INPUT_REQUIRED = L"SNX_TAECHANG_RECEIVABLES_001";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_ERR_OUTPUT_REQUIRED = L"SNX_TAECHANG_RECEIVABLES_002";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_ERR_TEMPLATE_MISSING = L"SNX_TAECHANG_RECEIVABLES_003";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_ERR_SCRIPT_MISSING = L"SNX_TAECHANG_RECEIVABLES_004";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_ERR_PROCESS_FAILED = L"SNX_TAECHANG_RECEIVABLES_005";
    constexpr const wchar_t* TAECHANG_RECEIVABLES_ERR_RESULT_MISSING = L"SNX_TAECHANG_RECEIVABLES_006";

    BOOL FileExists(const CString& strPath)
    {
        DWORD dwAttr = GetFileAttributesW(strPath);
        return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0) ? TRUE : FALSE;
    }

    BOOL FolderExists(const CString& strPath)
    {
        DWORD dwAttr = GetFileAttributesW(strPath);
        return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0) ? TRUE : FALSE;
    }

    BOOL GetPluginDirectory(CString& outDirectory)
    {
        HMODULE hModule = NULL;
        BOOL bResult = GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&GetPluginDirectory),
            &hModule);
        if (!bResult)
            return FALSE;

        wchar_t szPath[MAX_PATH] = {};
        DWORD dwLen = GetModuleFileNameW(hModule, szPath, MAX_PATH);
        if (dwLen == 0 || dwLen >= MAX_PATH)
            return FALSE;

        CString strPath = szPath;
        int nSlash = strPath.ReverseFind(L'\\');
        if (nSlash < 0)
            return FALSE;

        outDirectory = strPath.Left(nSlash);
        return TRUE;
    }

    CString CombinePath(const CString& strDirectory, const CString& strRelativePath)
    {
        CString strResult = strDirectory;
        if (!strResult.IsEmpty() && strResult[strResult.GetLength() - 1] != L'\\')
            strResult += L"\\";
        strResult += strRelativePath;
        return strResult;
    }

    CString BuildTempJsonPath()
    {
        wchar_t szTempPath[MAX_PATH] = {};
        wchar_t szTempFile[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, szTempPath);
        GetTempFileNameW(szTempPath, L"tcr", 0, szTempFile);

        CString strPath = szTempFile;
        int nDot = strPath.ReverseFind(L'.');
        if (nDot >= 0)
            strPath = strPath.Left(nDot);
        strPath += L".json";
        return strPath;
    }

    CString QuoteArgument(const CString& strValue)
    {
        CString strEscaped = strValue;
        strEscaped.Replace(L"\"", L"\\\"");
        return L"\"" + strEscaped + L"\"";
    }

    BOOL RunProcessAndWait(
        const CString& strCommandLine,
        DWORD& outExitCode,
        CString& strError)
    {
        STARTUPINFOW si = {};
        PROCESS_INFORMATION pi = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        CString strMutableCommandLine = strCommandLine;
        BOOL bCreated = CreateProcessW(
            NULL,
            strMutableCommandLine.GetBuffer(),
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi);
        strMutableCommandLine.ReleaseBuffer();

        if (!bCreated)
        {
            strError.Format(L"PowerShell 실행에 실패했습니다. error=%lu", GetLastError());
            return FALSE;
        }

        DWORD dwWait = WaitForSingleObject(pi.hProcess, TAECHANG_RECEIVABLES_TIMEOUT_MS);
        if (dwWait == WAIT_TIMEOUT)
        {
            TerminateProcess(pi.hProcess, 1);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            strError = L"미수금 내역서 처리 시간이 초과되었습니다.";
            return FALSE;
        }

        if (!GetExitCodeProcess(pi.hProcess, &outExitCode))
            outExitCode = 1;

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return TRUE;
    }

    BOOL ReadUtf8File(const CString& strPath, CString& outContent, CString& strError)
    {
        std::ifstream file(WideToUtf8(strPath), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            strError = L"결과 파일을 읽지 못했습니다.";
            return FALSE;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string strUtf8 = buffer.str();
        if (strUtf8.size() >= 3 &&
            static_cast<unsigned char>(strUtf8[0]) == 0xEF &&
            static_cast<unsigned char>(strUtf8[1]) == 0xBB &&
            static_cast<unsigned char>(strUtf8[2]) == 0xBF)
        {
            strUtf8.erase(0, 3);
        }

        outContent = Utf8ToWide(strUtf8);
        outContent.Trim();
        return TRUE;
    }

    CString HandleOpenInputDialog(const CString& strRequestId)
    {
        COMDLG_FILTERSPEC aTypes[] = {
            { L"Excel 파일", L"*.xls;*.xlsx" },
            { L"모든 파일",  L"*.*"           }
        };
        std::vector<CString> arrPaths;
        ShowIFileOpenDialog(GetAppMainWindow(), L"미수금 내역서 파일 선택", L"xls",
                            aTypes, 2, FALSE, arrPaths);

        CString strFilePath = arrPaths.empty() ? CString() : arrPaths[0];
        CString strPayload  = L"{\"filePath\":\"" + JsonEscapeString(strFilePath) + L"\"}";
        return BuildSuccessResponse(strRequestId, strPayload);
    }

    CString HandleLoadInputData(
        const CString& strRequestId,
        const CString& strPayloadJson)
    {
        CString strInputPath = JsonExtractString(strPayloadJson, L"inputPath");

        if (strInputPath.IsEmpty() || !FileExists(strInputPath))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_INPUT_REQUIRED, L"파일을 선택해주세요.");

        CString strPluginDirectory;
        if (!GetPluginDirectory(strPluginDirectory))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_SCRIPT_MISSING, L"플러그인 경로를 확인하지 못했습니다.");

        CString strScriptPath = CombinePath(strPluginDirectory, TAECHANG_RECEIVABLES_LOAD_SCRIPT_PATH);
        if (!FileExists(strScriptPath))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_SCRIPT_MISSING, L"미수금 데이터 로드 스크립트가 없습니다.");

        CString strResultPath = BuildTempJsonPath();
        CString strCommandLine = QuoteArgument(TAECHANG_POWERSHELL_PATH) +
            L" -NoProfile -ExecutionPolicy Bypass -File " + QuoteArgument(strScriptPath) +
            L" -InputPath " + QuoteArgument(strInputPath) +
            L" -ResultPath " + QuoteArgument(strResultPath);

        DWORD dwExitCode = 0;
        CString strError;
        if (!RunProcessAndWait(strCommandLine, dwExitCode, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_PROCESS_FAILED, strError);
        }

        CString strResultJson;
        if (!ReadUtf8File(strResultPath, strResultJson, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_RESULT_MISSING, strError);
        }
        DeleteFileW(strResultPath);

        if (dwExitCode != 0)
        {
            CString strMessage = JsonExtractString(strResultJson, L"message");
            if (strMessage.IsEmpty())
                strMessage = L"미수금 데이터를 불러오는 중 오류가 발생했습니다.";
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_PROCESS_FAILED, strMessage);
        }

        return BuildSuccessResponse(strRequestId, strResultJson);
    }

    CString HandleSelectOutputFolder(const CString& strRequestId)
    {
        HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        BOOL bNeedUninitialize = (hrInit == S_OK || hrInit == S_FALSE) ? TRUE : FALSE;

        IFileOpenDialog* pDialog = NULL;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
        if (FAILED(hr) || pDialog == NULL)
        {
            if (bNeedUninitialize)
                CoUninitialize();
            return BuildErrorResponse(strRequestId, L"SNX_TAECHANG_RECEIVABLES_007", L"폴더 선택 창을 열지 못했습니다.");
        }

        DWORD dwOptions = 0;
        pDialog->GetOptions(&dwOptions);
        pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        pDialog->SetTitle(L"미수금 내역서 저장 폴더 선택");

        CString strFolderPath;
        hr = SafeShowDialog(pDialog, GetAppMainWindow());
        if (SUCCEEDED(hr))
        {
            IShellItem* pItem = NULL;
            hr = pDialog->GetResult(&pItem);
            if (SUCCEEDED(hr) && pItem != NULL)
            {
                PWSTR pszPath = NULL;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr) && pszPath != NULL)
                {
                    strFolderPath = pszPath;
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }

        pDialog->Release();
        if (bNeedUninitialize)
            CoUninitialize();

        CString strPayload = L"{\"folderPath\":\"" + JsonEscapeString(strFolderPath) + L"\"}";
        return BuildSuccessResponse(strRequestId, strPayload);
    }

    CString HandleGenerateForm(
        const CString& strRequestId,
        const CString& strPayloadJson)
    {
        CString strInputPath = JsonExtractString(strPayloadJson, L"inputPath");
        CString strOutputFolder = JsonExtractString(strPayloadJson, L"outputFolder");

        if (strInputPath.IsEmpty() || !FileExists(strInputPath))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_INPUT_REQUIRED, L"파일을 선택해주세요.");

        if (strOutputFolder.IsEmpty() || !FolderExists(strOutputFolder))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_OUTPUT_REQUIRED, L"저장 폴더를 선택해주세요.");

        CString strPluginDirectory;
        if (!GetPluginDirectory(strPluginDirectory))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_SCRIPT_MISSING, L"플러그인 경로를 확인하지 못했습니다.");

        CString strTemplatePath = CombinePath(strPluginDirectory, TAECHANG_RECEIVABLES_TEMPLATE_PATH);
        CString strScriptPath = CombinePath(strPluginDirectory, TAECHANG_RECEIVABLES_GEN_SCRIPT_PATH);

        if (!FileExists(strTemplatePath))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_TEMPLATE_MISSING, L"미수금 내역서 템플릿 파일이 없습니다.");

        if (!FileExists(strScriptPath))
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_SCRIPT_MISSING, L"미수금 내역서 생성 스크립트가 없습니다.");

        CString strResultPath = BuildTempJsonPath();
        CString strCommandLine = QuoteArgument(TAECHANG_POWERSHELL_PATH) +
            L" -NoProfile -ExecutionPolicy Bypass -File " + QuoteArgument(strScriptPath) +
            L" -InputPath " + QuoteArgument(strInputPath) +
            L" -TemplatePath " + QuoteArgument(strTemplatePath) +
            L" -OutputFolder " + QuoteArgument(strOutputFolder) +
            L" -ResultPath " + QuoteArgument(strResultPath);

        DWORD dwExitCode = 0;
        CString strError;
        if (!RunProcessAndWait(strCommandLine, dwExitCode, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_PROCESS_FAILED, strError);
        }

        CString strResultJson;
        if (!ReadUtf8File(strResultPath, strResultJson, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_RESULT_MISSING, strError);
        }
        DeleteFileW(strResultPath);

        if (dwExitCode != 0)
        {
            CString strMessage = JsonExtractString(strResultJson, L"message");
            if (strMessage.IsEmpty())
                strMessage = L"미수금 내역서 생성 중 오류가 발생했습니다.";
            return BuildErrorResponse(strRequestId, TAECHANG_RECEIVABLES_ERR_PROCESS_FAILED, strMessage);
        }

        return BuildSuccessResponse(strRequestId, strResultJson);
    }
}

CString TaechangReceivablesExcelService::BuildOpenInputDialogResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return HandleOpenInputDialog(strRequestId);
}

CString TaechangReceivablesExcelService::BuildLoadInputDataResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    return HandleLoadInputData(strRequestId, strPayloadJson);
}

CString TaechangReceivablesExcelService::BuildSelectOutputFolderResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return HandleSelectOutputFolder(strRequestId);
}

CString TaechangReceivablesExcelService::BuildGenerateResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    return HandleGenerateForm(strRequestId, strPayloadJson);
}
