#include "pch.h"
#include "app/application/services/TaechangPdfCompareService.h"
#include "app/common/TaechangJson.h"
#include "app/infrastructure/bridge/TaechangBridgeResponse.h"
#include <filesystem>

namespace
{
    constexpr int TAECHANG_PDF_TIMEOUT_MS = 600000;
    constexpr LPCWSTR TAECHANG_PDFTOTEXT_PATH = L"C:\\Program Files\\Git\\mingw64\\bin\\pdftotext.exe";
    constexpr LPCWSTR TAECHANG_PDF_ERR_INPUT_REQUIRED = L"SNX_TAECHANG_PDF_001";
    constexpr LPCWSTR TAECHANG_PDF_ERR_EXTRACT_FAILED = L"SNX_TAECHANG_PDF_002";
    constexpr LPCWSTR TAECHANG_PDF_ERR_PROCESS_FAILED = L"SNX_TAECHANG_PDF_003";

    BOOL FileExists(const CString& strPath)
    {
        DWORD dwAttr = GetFileAttributesW(strPath);
        return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0) ? TRUE : FALSE;
    }

    CString BuildTempTextPath()
    {
        wchar_t szTempPath[MAX_PATH] = {};
        wchar_t szTempFile[MAX_PATH] = {};
        GetTempPathW(MAX_PATH, szTempPath);
        GetTempFileNameW(szTempPath, L"tcp", 0, szTempFile);
        CString strPath = szTempFile;
        int nDot = strPath.ReverseFind(L'.');
        if (nDot >= 0)
            strPath = strPath.Left(nDot);
        strPath += L".txt";
        return strPath;
    }

    CString QuoteArgument(const CString& strValue)
    {
        CString strEscaped = strValue;
        strEscaped.Replace(L"\"", L"\\\"");
        return L"\"" + strEscaped + L"\"";
    }

    BOOL RunProcessAndWait(const CString& strCommandLine, DWORD& outExitCode, CString& strError)
    {
        STARTUPINFOW si = {};
        PROCESS_INFORMATION pi = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        CString strMutableCommandLine = strCommandLine;
        BOOL bCreated = CreateProcessW(NULL, strMutableCommandLine.GetBuffer(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        strMutableCommandLine.ReleaseBuffer();
        if (!bCreated)
        {
            strError.Format(L"CreateProcess failed. error=%lu", GetLastError());
            return FALSE;
        }

        DWORD dwWait = WaitForSingleObject(pi.hProcess, TAECHANG_PDF_TIMEOUT_MS);
        if (dwWait == WAIT_TIMEOUT)
        {
            TerminateProcess(pi.hProcess, 1);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            strError = L"PDF compare timeout.";
            return FALSE;
        }

        if (!GetExitCodeProcess(pi.hProcess, &outExitCode))
            outExitCode = 1;

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return TRUE;
    }

    CString CollapseWhitespace(const CString& strValue)
    {
        CString strResult;
        BOOL bPrevSpace = FALSE;
        for (int i = 0; i < strValue.GetLength(); ++i)
        {
            wchar_t ch = strValue[i];
            if (iswspace(ch) != 0)
            {
                if (!bPrevSpace && !strResult.IsEmpty())
                    strResult += L' ';
                bPrevSpace = TRUE;
                continue;
            }
            strResult += ch;
            bPrevSpace = FALSE;
        }
        strResult.Trim();
        return strResult;
    }

    CString NormalizeVisibleText(const CString& strValue)
    {
        CString strCollapsed = CollapseWhitespace(strValue);
        CString strResult;
        for (int i = 0; i < strCollapsed.GetLength(); ++i)
        {
            if (iswspace(strCollapsed[i]) == 0)
                strResult += strCollapsed[i];
        }
        return strResult;
    }

    BOOL ReadUtf8File(const CString& strPath, CString& outContent, CString& strError)
    {
        std::ifstream file(WideToUtf8(strPath), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            strError = L"Cannot read extracted text.";
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
        return TRUE;
    }

    BOOL ExtractPdfText(const CString& strPdfPath, CString& outText, CString& strError)
    {
        if (!FileExists(TAECHANG_PDFTOTEXT_PATH))
        {
            strError = L"pdftotext.exe was not found.";
            return FALSE;
        }

        CString strOutputPath = BuildTempTextPath();
        CString strCommandLine = QuoteArgument(TAECHANG_PDFTOTEXT_PATH) +
            L" -layout -enc UTF-8 -q " + QuoteArgument(strPdfPath) + L" " + QuoteArgument(strOutputPath);

        DWORD dwExitCode = 0;
        if (!RunProcessAndWait(strCommandLine, dwExitCode, strError))
        {
            DeleteFileW(strOutputPath);
            return FALSE;
        }

        if (dwExitCode != 0)
        {
            DeleteFileW(strOutputPath);
            strError.Format(L"pdftotext failed. exit=%lu", dwExitCode);
            return FALSE;
        }

        BOOL bRead = ReadUtf8File(strOutputPath, outText, strError);
        DeleteFileW(strOutputPath);
        return bRead;
    }

    void SplitJsonStringArray(const CString& strArrayJson, std::vector<CString>& outValues)
    {
        std::string strJson = WideToUtf8(strArrayJson);
        bool bInString = false;
        bool bEscaped = false;
        std::string strCurrent;
        for (size_t i = 0; i < strJson.size(); ++i)
        {
            char ch = strJson[i];
            if (!bInString)
            {
                if (ch == '"')
                {
                    bInString = true;
                    strCurrent.clear();
                }
                continue;
            }

            if (bEscaped)
            {
                if (ch == 'n')
                    strCurrent += '\n';
                else if (ch == 'r')
                    strCurrent += '\r';
                else if (ch == 't')
                    strCurrent += '\t';
                else
                    strCurrent += ch;
                bEscaped = false;
                continue;
            }

            if (ch == '\\')
            {
                bEscaped = true;
                continue;
            }

            if (ch == '"')
            {
                outValues.push_back(Utf8ToWide(strCurrent));
                bInString = false;
                continue;
            }

            strCurrent += ch;
        }
    }

    CString ExtractJsonArray(const CString& strJson, const CString& strKey)
    {
        std::string json = WideToUtf8(strJson);
        std::string key = WideToUtf8(strKey);
        std::string token = "\"" + key + "\"";
        size_t nKeyPos = json.find(token);
        if (nKeyPos == std::string::npos)
            return L"";
        size_t nStart = json.find('[', nKeyPos + token.size());
        if (nStart == std::string::npos)
            return L"";

        int nDepth = 0;
        bool bInString = false;
        bool bEscaped = false;
        for (size_t i = nStart; i < json.size(); ++i)
        {
            char ch = json[i];
            if (bEscaped)
            {
                bEscaped = false;
                continue;
            }
            if (ch == '\\' && bInString)
            {
                bEscaped = true;
                continue;
            }
            if (ch == '"')
            {
                bInString = !bInString;
                continue;
            }
            if (bInString)
                continue;
            if (ch == '[')
                ++nDepth;
            else if (ch == ']')
            {
                --nDepth;
                if (nDepth == 0)
                    return Utf8ToWide(json.substr(nStart, i - nStart + 1));
            }
        }
        return L"";
    }

    void SplitLeftRightText(const CString& strText, CString& outLeft, CString& outRight)
    {
        CString strLine;
        for (int i = 0; i <= strText.GetLength(); ++i)
        {
            wchar_t ch = i < strText.GetLength() ? strText[i] : L'\n';
            if (ch != L'\n' && ch != L'\f')
            {
                if (ch != L'\r')
                    strLine += ch;
                continue;
            }

            int nLength = strLine.GetLength();
            if (nLength > 0)
            {
                int nSplit = nLength / 2;
                outLeft += strLine.Left(nSplit);
                outLeft += L"\n";
                outRight += strLine.Mid(nSplit);
                outRight += L"\n";
            }
            strLine.Empty();
        }
    }

    CString BuildSingleCompareItem(const CString& strPdfPath)
    {
        CString strText;
        CString strError;
        BOOL bSuccess = ExtractPdfText(strPdfPath, strText, strError);
        CString strFileName = std::filesystem::path(static_cast<LPCWSTR>(strPdfPath)).filename().wstring().c_str();
        if (!bSuccess)
        {
            CString strErrorItem;
            strErrorItem.Format(
                L"{\"filePath\":\"%s\",\"fileName\":\"%s\",\"status\":\"error\",\"matched\":false,\"reason\":\"%s\"}",
                (LPCWSTR)JsonEscapeString(strPdfPath),
                (LPCWSTR)JsonEscapeString(strFileName),
                (LPCWSTR)JsonEscapeString(strError));
            return strErrorItem;
        }

        CString strLeft;
        CString strRight;
        SplitLeftRightText(strText, strLeft, strRight);
        CString strLeftNormalized = NormalizeVisibleText(strLeft);
        CString strRightNormalized = NormalizeVisibleText(strRight);
        BOOL bMatched = (!strLeftNormalized.IsEmpty() && strLeftNormalized == strRightNormalized) ? TRUE : FALSE;
        CString strStatus = bMatched ? L"pass" : L"fail";
        CString strReason = bMatched ? L"left/right text matched" : L"left/right text differs";

        CString strItem;
        strItem.Format(
            L"{\"filePath\":\"%s\",\"fileName\":\"%s\",\"status\":\"%s\",\"matched\":%s,\"reason\":\"%s\",\"leftValue\":\"%s\",\"rightValue\":\"%s\"}",
            (LPCWSTR)JsonEscapeString(strPdfPath),
            (LPCWSTR)JsonEscapeString(strFileName),
            (LPCWSTR)JsonEscapeString(strStatus),
            bMatched ? L"true" : L"false",
            (LPCWSTR)JsonEscapeString(strReason),
            (LPCWSTR)JsonEscapeString(strLeftNormalized.Left(120)),
            (LPCWSTR)JsonEscapeString(strRightNormalized.Left(120)));
        return strItem;
    }

    CString BuildBatchCompareResponse(const CString& strRequestId, const std::vector<CString>& arrPdfPaths)
    {
        if (arrPdfPaths.empty())
            return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_INPUT_REQUIRED, L"Select PDF files.");

        int nPass = 0;
        int nFail = 0;
        CString strItems = L"[";
        for (int i = 0; i < static_cast<int>(arrPdfPaths.size()); ++i)
        {
            CString strItem = BuildSingleCompareItem(arrPdfPaths[i]);
            if (strItem.Find(L"\"status\":\"pass\"") >= 0)
                ++nPass;
            else
                ++nFail;
            if (i > 0)
                strItems += L",";
            strItems += strItem;
        }
        strItems += L"]";

        CString strStatus = nFail == 0 ? L"pass" : L"fail";
        CString strPayload;
        strPayload.Format(
            L"{\"status\":\"%s\",\"extractor\":\"pdftotext-layout\",\"totalFiles\":%d,\"passedFiles\":%d,\"failedFiles\":%d,\"files\":%s}",
            (LPCWSTR)JsonEscapeString(strStatus),
            static_cast<int>(arrPdfPaths.size()),
            nPass,
            nFail,
            (LPCWSTR)strItems);
        return BuildSuccessResponse(strRequestId, strPayload);
    }
}

CString TaechangPdfCompareService::BuildOpenFileDialogResponse(const CString& strRequestId, const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_INPUT_REQUIRED, L"Use the MFC file picker.");
}

CString TaechangPdfCompareService::BuildOpenMultiFileDialogResponse(const CString& strRequestId, const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_INPUT_REQUIRED, L"Use the MFC file picker.");
}

CString TaechangPdfCompareService::BuildRunCompareResponse(const CString& strRequestId, const CString& strPayloadJson)
{
    CString strFilePathsJson = ExtractJsonArray(strPayloadJson, L"pdfFilePaths");
    std::vector<CString> arrPdfPaths;
    SplitJsonStringArray(strFilePathsJson, arrPdfPaths);
    return BuildBatchCompareResponse(strRequestId, arrPdfPaths);
}

CString TaechangPdfCompareService::BuildExportCsvResponse(const CString& strRequestId, const CString& strPayloadJson)
{
    UNREFERENCED_PARAMETER(strPayloadJson);
    return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_PROCESS_FAILED, L"CSV export is not implemented in MFC PDF compare yet.");
}
