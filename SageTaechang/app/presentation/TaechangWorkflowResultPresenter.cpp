#include "pch.h"
#include "app/presentation/TaechangWorkflowResultPresenter.h"
#include "TaechangDefine.h"
#include "app/common/TaechangJson.h"

namespace
{
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

    void SplitJsonObjectArray(const CString& strArrayJson, std::vector<CString>& outObjects)
    {
        std::string json = WideToUtf8(strArrayJson);
        int nDepth = 0;
        bool bInString = false;
        bool bEscaped = false;
        size_t nObjectStart = std::string::npos;
        for (size_t i = 0; i < json.size(); ++i)
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
            if (ch == '{')
            {
                if (nDepth == 0)
                    nObjectStart = i;
                ++nDepth;
            }
            else if (ch == '}')
            {
                --nDepth;
                if (nDepth == 0 && nObjectStart != std::string::npos)
                {
                    outObjects.push_back(Utf8ToWide(json.substr(nObjectStart, i - nObjectStart + 1)));
                    nObjectStart = std::string::npos;
                }
            }
        }
    }

    CString JsonExtractIntText(const CString& strJson, const CString& strKey)
    {
        std::string json = WideToUtf8(strJson);
        std::string key = WideToUtf8(strKey);
        std::string token = "\"" + key + "\"";
        size_t nKeyPos = json.find(token);
        if (nKeyPos == std::string::npos)
            return L"";

        size_t nColon = json.find(':', nKeyPos + token.size());
        if (nColon == std::string::npos)
            return L"";

        size_t nStart = nColon + 1;
        while (nStart < json.size() && (json[nStart] == ' ' || json[nStart] == '\t'))
            ++nStart;

        size_t nEnd = nStart;
        while (nEnd < json.size() && json[nEnd] >= '0' && json[nEnd] <= '9')
            ++nEnd;

        if (nEnd == nStart)
            return L"";
        return Utf8ToWide(json.substr(nStart, nEnd - nStart));
    }

    CString ComposeReason(const CString& strReason, const CString& strRightValue)
    {
        CString strResult = strReason;
        if (!strRightValue.IsEmpty())
        {
            if (!strResult.IsEmpty())
                strResult += L" | ";
            strResult += TAECHANG_UI_RESULT_BASELINE_PREFIX + strRightValue.Left(80);
        }
        return strResult;
    }
}

BOOL TaechangWorkflowResultPresenter::BuildRows(
    int nWorkflowType,
    int nTaskType,
    const CString& strResponseJson,
    std::vector<TaechangResultRow>& outRows,
    CString& outDetailText)
{
    UNREFERENCED_PARAMETER(nTaskType);
    outRows.clear();
    outDetailText = strResponseJson;

    BOOL bSuccess = JsonExtractBool(strResponseJson, L"success");
    AddRow(outRows, TAECHANG_UI_RESULT_STATUS, bSuccess ? TAECHANG_UI_COMPLETED : TAECHANG_UI_FAILED, bSuccess ? L"success" : L"failed", L"");

    if (!bSuccess)
    {
        CString strCode = JsonExtractString(strResponseJson, L"code");
        CString strMessage = JsonExtractString(strResponseJson, L"message");
        AddRow(outRows, TAECHANG_UI_RESULT_ERROR, strCode, L"error", strMessage);
        return FALSE;
    }

    AddSummaryRows(strResponseJson, outRows);
    if (IsCompareWorkflow(nWorkflowType))
        AddCompareFileRows(strResponseJson, outRows);

    return TRUE;
}

void TaechangWorkflowResultPresenter::AddRow(
    std::vector<TaechangResultRow>& outRows,
    const CString& strField,
    const CString& strValue,
    const CString& strStatus,
    const CString& strReason) const
{
    TaechangResultRow row;
    row.m_strField = strField;
    row.m_strValue = strValue;
    row.m_strStatus = strStatus;
    row.m_strReason = strReason;
    outRows.push_back(row);
}

void TaechangWorkflowResultPresenter::AddSummaryRows(
    const CString& strResponseJson,
    std::vector<TaechangResultRow>& outRows) const
{
    CString strStatus = JsonExtractString(strResponseJson, L"status");
    CString strFileName = JsonExtractString(strResponseJson, L"fileName");
    CString strFolder = JsonExtractString(strResponseJson, L"outputFolder");
    CString strTotal = JsonExtractIntText(strResponseJson, L"totalFiles");
    CString strPassed = JsonExtractIntText(strResponseJson, L"passedFiles");
    CString strFailed = JsonExtractIntText(strResponseJson, L"failedFiles");

    if (!strStatus.IsEmpty())
        AddRow(outRows, L"Result", strStatus, strStatus, L"");
    if (!strTotal.IsEmpty())
        AddRow(outRows, L"Total", strTotal, L"summary", L"Passed " + strPassed + L", Failed " + strFailed);
    if (!strFileName.IsEmpty())
        AddRow(outRows, TAECHANG_UI_RESULT_FILE, strFileName, L"output", L"");
    if (!strFolder.IsEmpty())
        AddRow(outRows, TAECHANG_UI_RESULT_FOLDER, strFolder, L"output", L"");
}

void TaechangWorkflowResultPresenter::AddCompareFileRows(
    const CString& strResponseJson,
    std::vector<TaechangResultRow>& outRows) const
{
    CString strFilesJson = ExtractJsonArray(strResponseJson, L"files");
    std::vector<CString> arrObjects;
    SplitJsonObjectArray(strFilesJson, arrObjects);
    for (int i = 0; i < static_cast<int>(arrObjects.size()); ++i)
    {
        CString strFileName = JsonExtractString(arrObjects[i], L"fileName");
        CString strStatus = JsonExtractString(arrObjects[i], L"status");
        CString strReason = JsonExtractString(arrObjects[i], L"reason");
        CString strLeftValue = JsonExtractString(arrObjects[i], L"leftValue");
        CString strRightValue = JsonExtractString(arrObjects[i], L"rightValue");
        CString strItemLabel = JsonExtractString(arrObjects[i], L"itemLabel");
        TaechangResultRow row;
        row.m_strFile = strFileName;
        row.m_strField = strItemLabel;
        row.m_strValue = strLeftValue;
        row.m_strStatus = strStatus;
        row.m_strReason = ComposeReason(strReason, strRightValue);
        outRows.push_back(row);
    }
}

BOOL TaechangWorkflowResultPresenter::IsCompareWorkflow(int nWorkflowType) const
{
    return (nWorkflowType == TAECHANG_WORKFLOW_PDF_COMPARE || nWorkflowType == TAECHANG_WORKFLOW_HWP_COMPARE) ? TRUE : FALSE;
}
