#include "pch.h"
#include "app/application/services/TaechangDeliveryExcelService.h"
#include "app/common/TaechangJson.h"
#include "app/common/TaechangFileUtils.h"
#include "app/common/TaechangDialogHelper.h"
#include "app/infrastructure/bridge/TaechangBridgeResponse.h"
#include "TaechangDefine.h"

namespace
{
    constexpr const wchar_t* TAECHANG_DELIVERY_LOAD_SCRIPT_PATH = L"tools\\load-delivery-data.ps1";
    constexpr const wchar_t* TAECHANG_DELIVERY_SCRIPT_PATH = L"tools\\generate-delivery-forms.ps1";
    constexpr const wchar_t* TAECHANG_DELIVERY_TEMPLATE_PATH = L"templates\\delivery-template.xls";
    constexpr const wchar_t* TAECHANG_ERROR_INPUT_REQUIRED = L"SNX_TAECHANG_DELIVERY_001";
    constexpr const wchar_t* TAECHANG_ERROR_OUTPUT_REQUIRED = L"SNX_TAECHANG_DELIVERY_002";
    constexpr const wchar_t* TAECHANG_ERROR_TEMPLATE_MISSING = L"SNX_TAECHANG_DELIVERY_003";
    constexpr const wchar_t* TAECHANG_ERROR_SCRIPT_MISSING = L"SNX_TAECHANG_DELIVERY_004";
    constexpr const wchar_t* TAECHANG_ERROR_PROCESS_FAILED = L"SNX_TAECHANG_DELIVERY_005";
    constexpr const wchar_t* TAECHANG_ERROR_RESULT_MISSING = L"SNX_TAECHANG_DELIVERY_006";
    constexpr const wchar_t* TAECHANG_ERROR_NO_SELECTION = L"SNX_TAECHANG_DELIVERY_008";

    BOOL ReadUtf8File(const CString& strPath, CString& outContent, CString& strError)
    {
        std::ifstream file(WideToUtf8(strPath), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            strError = L"寃곌낵 ?뚯씪???쎌? 紐삵뻽?듬땲??";
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

    CString HandleOpenInputDialogResponse(const CString& strRequestId)
    {
        COMDLG_FILTERSPEC aTypes[] = {
            { L"Excel ?뚯씪", L"*.xlsx;*.xls" },
            { L"紐⑤뱺 ?뚯씪",  L"*.*"           }
        };
        std::vector<CString> arrPaths;
        ShowIFileOpenDialog(GetAppMainWindow(), L"?⑺뭹???뚯씪 ?좏깮", L"xlsx",
                            aTypes, 2, FALSE, arrPaths);

        CString strFilePath = arrPaths.empty() ? CString() : arrPaths[0];
        CString strPayload  = L"{\"filePath\":\"" + JsonEscapeString(strFilePath) + L"\"}";
        return BuildSuccessResponse(strRequestId, strPayload);
    }

    CString HandleLoadInputDataResponse(
        const CString& strRequestId,
        const CString& strPayloadJson)
    {
        CString strInputPath = JsonExtractString(strPayloadJson, L"inputPath");

        if (strInputPath.IsEmpty() || !FileExists(strInputPath))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_INPUT_REQUIRED, L"?뚯씪???좏깮?댁＜?몄슂.");

        CString strPluginDirectory;
        if (!GetExecutableDirectory(strPluginDirectory))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_SCRIPT_MISSING, L"?뚮윭洹몄씤 寃쎈줈瑜??뺤씤?섏? 紐삵뻽?듬땲??");

        CString strScriptPath = CombinePath(strPluginDirectory, TAECHANG_DELIVERY_LOAD_SCRIPT_PATH);
        if (!FileExists(strScriptPath))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_SCRIPT_MISSING, L"?곗씠??濡쒕뱶 ?ㅽ겕由쏀듃媛 ?놁뒿?덈떎.");

        CString strResultPath = BuildTempJsonPath(L"tcd");
        CString strCommandLine = QuoteArgument(TAECHANG_POWERSHELL_PATH) +
            L" -NoProfile -ExecutionPolicy Bypass -File " + QuoteArgument(strScriptPath) +
            L" -InputPath " + QuoteArgument(strInputPath) +
            L" -ResultPath " + QuoteArgument(strResultPath);

        DWORD dwExitCode = 0;
        CString strError;
        if (!RunProcessAndWait(strCommandLine, dwExitCode, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_PROCESS_FAILED, strError);
        }

        CString strResultJson;
        if (!ReadUtf8File(strResultPath, strResultJson, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_RESULT_MISSING, strError);
        }
        DeleteFileW(strResultPath);

        if (dwExitCode != 0)
        {
            CString strMessage = JsonExtractString(strResultJson, L"message");
            if (strMessage.IsEmpty())
                strMessage = L"?곗씠??濡쒕뱶 以??ㅻ쪟媛 諛쒖깮?덉뒿?덈떎.";
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_PROCESS_FAILED, strMessage);
        }

        return BuildSuccessResponse(strRequestId, strResultJson);
    }

    CString HandleSelectOutputFolderResponse(const CString& strRequestId)
    {
        HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        BOOL bNeedUninitialize = (hrInit == S_OK || hrInit == S_FALSE) ? TRUE : FALSE;

        IFileOpenDialog* pDialog = NULL;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
        if (FAILED(hr) || pDialog == NULL)
        {
            if (bNeedUninitialize)
                CoUninitialize();
            return BuildErrorResponse(strRequestId, L"SNX_TAECHANG_DELIVERY_007", L"?대뜑 ?좏깮 李쎌쓣 ?댁? 紐삵뻽?듬땲??");
        }

        DWORD dwOptions = 0;
        pDialog->GetOptions(&dwOptions);
        pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        pDialog->SetTitle(L"?⑺뭹??????대뜑 ?좏깮");

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

    CString HandleGenerateResponse(
        const CString& strRequestId,
        const CString& strPayloadJson)
    {
        CString strInputPath = JsonExtractString(strPayloadJson, L"inputPath");
        CString strOutputFolder = JsonExtractString(strPayloadJson, L"outputFolder");
        CString strRowNums = JsonExtractString(strPayloadJson, L"rowNums");

        if (strInputPath.IsEmpty() || !FileExists(strInputPath))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_INPUT_REQUIRED, L"?뚯씪???좏깮?댁＜?몄슂.");

        if (strOutputFolder.IsEmpty() || !FolderExists(strOutputFolder))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_OUTPUT_REQUIRED, L"????대뜑瑜??좏깮?댁＜?몄슂.");

        CString strPluginDirectory;
        if (!GetExecutableDirectory(strPluginDirectory))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_SCRIPT_MISSING, L"?뚮윭洹몄씤 寃쎈줈瑜??뺤씤?섏? 紐삵뻽?듬땲??");

        CString strTemplatePath = CombinePath(strPluginDirectory, TAECHANG_DELIVERY_TEMPLATE_PATH);
        CString strScriptPath = CombinePath(strPluginDirectory, TAECHANG_DELIVERY_SCRIPT_PATH);

        if (!FileExists(strTemplatePath))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_TEMPLATE_MISSING, L"?⑺뭹???쒗뵆由??뚯씪???놁뒿?덈떎.");

        if (!FileExists(strScriptPath))
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_SCRIPT_MISSING, L"?⑺뭹???앹꽦 ?ㅽ겕由쏀듃媛 ?놁뒿?덈떎.");

        CString strResultPath = BuildTempJsonPath(L"tcd");
        CString strCommandLine = QuoteArgument(TAECHANG_POWERSHELL_PATH) +
            L" -NoProfile -ExecutionPolicy Bypass -File " + QuoteArgument(strScriptPath) +
            L" -InputPath " + QuoteArgument(strInputPath) +
            L" -TemplatePath " + QuoteArgument(strTemplatePath) +
            L" -OutputFolder " + QuoteArgument(strOutputFolder) +
            L" -ResultPath " + QuoteArgument(strResultPath) +
            L" -RowNums " + QuoteArgument(strRowNums);

        DWORD dwExitCode = 0;
        CString strError;
        if (!RunProcessAndWait(strCommandLine, dwExitCode, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_PROCESS_FAILED, strError);
        }

        CString strResultJson;
        if (!ReadUtf8File(strResultPath, strResultJson, strError))
        {
            DeleteFileW(strResultPath);
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_RESULT_MISSING, strError);
        }
        DeleteFileW(strResultPath);

        if (dwExitCode != 0)
        {
            CString strMessage = JsonExtractString(strResultJson, L"message");
            if (strMessage.IsEmpty())
                strMessage = L"?⑺뭹???앹꽦 以??ㅻ쪟媛 諛쒖깮?덉뒿?덈떎.";
            return BuildErrorResponse(strRequestId, TAECHANG_ERROR_PROCESS_FAILED, strMessage);
        }

        return BuildSuccessResponse(strRequestId, strResultJson);
    }
}

CString TaechangDeliveryExcelService::BuildOpenInputDialogResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return HandleOpenInputDialogResponse(strRequestId);
}

CString TaechangDeliveryExcelService::BuildLoadInputDataResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    return HandleLoadInputDataResponse(strRequestId, strPayloadJson);
}

CString TaechangDeliveryExcelService::BuildSelectOutputFolderResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return HandleSelectOutputFolderResponse(strRequestId);
}

CString TaechangDeliveryExcelService::BuildGenerateResponse(
    const CString& strRequestId,
    const CString& strPayloadJson)
{
    return HandleGenerateResponse(strRequestId, strPayloadJson);
}

