#include "pch.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "TaechangDefine.h"
#include "app/common/TaechangJson.h"

namespace
{
	void SplitJsonObjectArray(const CString& strArrayJson, std::vector<CString>& outObjects) {
		std::string json = WideToUtf8(strArrayJson);
		int nDepth = 0;
		bool bInString = false;
		bool bEscaped = false;
		size_t nObjectStart = std::string::npos;
		for (size_t i = 0; i < json.size(); ++i) {
			char ch = json[i];
			if (bEscaped) {
				bEscaped = false;
				continue;
			}
			if (ch == '\\' && bInString) {
				bEscaped = true;
				continue;
			}
			if (ch == '"') {
				bInString = !bInString;
				continue;
			}
			if (bInString)
				continue;
			if (ch == '{') {
				if (nDepth == 0)
					nObjectStart = i;
				++nDepth;
			} else if (ch == '}') {
				--nDepth;
				if (nDepth == 0 && nObjectStart != std::string::npos) {
					outObjects.push_back(Utf8ToWide(json.substr(nObjectStart, i - nObjectStart + 1)));
					nObjectStart = std::string::npos;
				}
			}
		}
	}

	CString JsonExtractIntText(const CString& strJson, const CString& strKey) {
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

	CString JsonExtractValueText(const CString& strJson, const CString& strKey) {
		CString strValue = JsonExtractString(strJson, strKey);
		if (!strValue.IsEmpty())
			return strValue;

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
		while (nStart < json.size() &&
			(json[nStart] == ' ' || json[nStart] == '\t' || json[nStart] == '\r' || json[nStart] == '\n'))
			++nStart;

		if (nStart >= json.size() || json[nStart] == '"' || json.compare(nStart, 4, "null") == 0)
			return L"";

		size_t nEnd = nStart;
		while (nEnd < json.size() && json[nEnd] != ',' && json[nEnd] != '}')
			++nEnd;

		while (nEnd > nStart &&
			(json[nEnd - 1] == ' ' || json[nEnd - 1] == '\t' || json[nEnd - 1] == '\r' || json[nEnd - 1] == '\n'))
			--nEnd;

		if (nEnd <= nStart)
			return L"";
		return Utf8ToWide(json.substr(nStart, nEnd - nStart));
	}

	CString FormatAmountText(const CString& strText) {
		CString strValue = strText;
		strValue.Trim();
		if (strValue.IsEmpty())
			return strValue;

		BOOL bNegative = FALSE;
		if (strValue[0] == L'-') {
			bNegative = TRUE;
			strValue = strValue.Mid(1);
		}

		CString strDigits;
		for (int i = 0; i < strValue.GetLength(); ++i) {
			wchar_t ch = strValue[i];
			if (ch >= L'0' && ch <= L'9')
				strDigits += ch;
			else if (ch != L',')
				return strText;
		}

		if (strDigits.IsEmpty())
			return strText;

		for (int i = strDigits.GetLength() - 3; i > 0; i -= 3)
			strDigits.Insert(i, L',');
		return bNegative ? CString(L"-") + strDigits : strDigits;
	}

}

TaechangResultRow::TaechangResultRow()
	: m_nSourceRowIndex(0) {}

BOOL TaechangWorkflowResultPresenter::BuildRows(
	int nWorkflowType,
	int nTaskType,
	const CString& strResponseJson,
	std::vector<TaechangResultRow>& outRows) {
	outRows.clear();

	BOOL bSuccess = JsonExtractBool(strResponseJson, L"success");

	if (!bSuccess) {
		AddRow(outRows, TAECHANG_UI_RESULT_STATUS, TAECHANG_UI_FAILED, TAECHANG_RESULT_STATUS_FAILED, L"");
		CString strCode = JsonExtractString(strResponseJson, L"code");
		CString strMessage = JsonExtractString(strResponseJson, L"message");
		AddRow(outRows, TAECHANG_UI_RESULT_ERROR, strCode, TAECHANG_RESULT_STATUS_ERROR, strMessage);
		return FALSE;
	}

	if (nWorkflowType == TAECHANG_WORKFLOW_RECEIVABLES &&
		(nTaskType == TAECHANG_TASK_LOAD || nTaskType == TAECHANG_TASK_GENERATE))
		AddReceivablesResultRows(strResponseJson, outRows);
	else if (nWorkflowType == TAECHANG_WORKFLOW_DELIVERY && nTaskType == TAECHANG_TASK_LOAD)
		AddDeliveryInputRows(strResponseJson, outRows);
	else if (nWorkflowType == TAECHANG_WORKFLOW_ESTIMATE && nTaskType == TAECHANG_TASK_LOAD)
		AddEstimateInputRows(strResponseJson, outRows);
	else {
		AddRow(outRows, TAECHANG_UI_RESULT_STATUS, TAECHANG_UI_COMPLETED, TAECHANG_RESULT_STATUS_SUCCESS, L"");
		AddSummaryRows(strResponseJson, outRows);
	}

	return TRUE;
}

void TaechangWorkflowResultPresenter::AddRow(
	std::vector<TaechangResultRow>& outRows,
	const CString& strField,
	const CString& strValue,
	const CString& strStatus,
	const CString& strReason) const {
	TaechangResultRow row;
	row.m_strField = strField;
	row.m_strValue = strValue;
	row.m_strStatus = strStatus;
	row.m_strReason = strReason;
	outRows.push_back(row);
}

void TaechangWorkflowResultPresenter::AddSummaryRows(
	const CString& strResponseJson,
	std::vector<TaechangResultRow>& outRows) const {
	CString strStatus = JsonExtractString(strResponseJson, L"status");
	CString strFileName = JsonExtractString(strResponseJson, L"fileName");
	CString strFolder = JsonExtractString(strResponseJson, L"outputFolder");
	CString strTotal = JsonExtractIntText(strResponseJson, L"totalFiles");
	CString strPassed = JsonExtractIntText(strResponseJson, L"passedFiles");
	CString strFailed = JsonExtractIntText(strResponseJson, L"failedFiles");

	if (!strStatus.IsEmpty())
		AddRow(outRows, TAECHANG_UI_RESULT_RESULT_LABEL, strStatus, strStatus, L"");
	if (!strTotal.IsEmpty())
		AddRow(outRows, TAECHANG_UI_RESULT_TOTAL_LABEL, strTotal, TAECHANG_RESULT_STATUS_SUMMARY, CString(TAECHANG_UI_RESULT_PASSED_PREFIX) + strPassed + TAECHANG_UI_RESULT_FAILED_SUFFIX + strFailed);
	if (!strFileName.IsEmpty())
		AddRow(outRows, TAECHANG_UI_RESULT_FILE, strFileName, TAECHANG_RESULT_STATUS_OUTPUT, L"");
	if (!strFolder.IsEmpty())
		AddRow(outRows, TAECHANG_UI_RESULT_FOLDER, strFolder, TAECHANG_RESULT_STATUS_OUTPUT, L"");
}

void TaechangWorkflowResultPresenter::AddReceivablesResultRows(
	const CString& strResponseJson,
	std::vector<TaechangResultRow>& outRows) const {
	CString strRowsJson = JsonExtractArray(strResponseJson, TAECHANG_JSON_KEY_ROWS);
	std::vector<CString> arrObjects;
	SplitJsonObjectArray(strRowsJson, arrObjects);
	for (int i = 0; i < static_cast<int>(arrObjects.size()); ++i) {
		CString strCompanyName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_COMPANY_NAME);
		CString strManager = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_MANAGER);
		CString strTotalAmount = JsonExtractValueText(arrObjects[i], TAECHANG_JSON_KEY_TOTAL_AMOUNT);
		CString strIssueDate = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ISSUE_DATE);
		if (strIssueDate.IsEmpty())
			strIssueDate = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ISSUE_DATE_TEXT);
		CString strIssueType = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ISSUE_TYPE);
		CString strItemName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ITEM_NAME);
		TaechangResultRow row;
		row.m_strCompanyName = strCompanyName;
		row.m_strManager = strManager;
		row.m_strIssueDate = strIssueDate;
		row.m_strItemName = strItemName;
		row.m_strIssueType = strIssueType;
		row.m_strTotalAmount = FormatAmountText(strTotalAmount);
		row.m_strDepositAmount = FormatAmountText(JsonExtractValueText(arrObjects[i], TAECHANG_JSON_KEY_DEPOSIT_AMOUNT));
		row.m_strReceivableAmount = FormatAmountText(JsonExtractValueText(arrObjects[i], TAECHANG_JSON_KEY_RECEIVABLE_AMOUNT));
		row.m_strBankName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_BANK_NAME);
		row.m_strNote = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_NOTE);
		outRows.push_back(row);
	}
}

void TaechangWorkflowResultPresenter::AddDeliveryInputRows(
	const CString& strResponseJson,
	std::vector<TaechangResultRow>& outRows) const {
	CString strRowsJson = JsonExtractArray(strResponseJson, TAECHANG_JSON_KEY_ROWS);
	std::vector<CString> arrObjects;
	SplitJsonObjectArray(strRowsJson, arrObjects);
	for (int i = 0; i < static_cast<int>(arrObjects.size()); ++i) {
		CString strRowIndex = JsonExtractIntText(arrObjects[i], TAECHANG_JSON_KEY_ROW_INDEX);
		CString strCompanyName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_COMPANY_NAME);
		CString strItemName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ITEM_NAME);
		CString strDepartment = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_DEPARTMENT);
		CString strOrderDate = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ORDER_DATE);
		CString strDeliveryDate = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_DELIVERY_DATE);
		CString strDeliveryTime = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_DELIVERY_TIME);
		CString strProductType = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_PRODUCT_TYPE);
		CString strCompanyCopies = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_COMPANY_COPIES);
		CString strCorporationCopies = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_CORPORATION_COPIES);
		CString strTotalCopies = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_TOTAL_COPIES);

		TaechangResultRow row;
		row.m_nSourceRowIndex = _wtoi(strRowIndex);
		row.m_strField = CString(TAECHANG_UI_DELIVERY_ROW_FIELD_PREFIX) + strRowIndex;
		row.m_strValue = strCompanyName;
		row.m_strStatus = strItemName;
		row.m_strCompanyName = strCompanyName;
		row.m_strDepartment = strDepartment;
		row.m_strOrderDate = strOrderDate;
		row.m_strDeliveryDate = strDeliveryDate;
		row.m_strDeliveryTime = strDeliveryTime;
		row.m_strItemName = strItemName;
		row.m_strProductType = strProductType;
		row.m_strCompanyCopies = strCompanyCopies;
		row.m_strCorporationCopies = strCorporationCopies;
		row.m_strTotalCopies = strTotalCopies;
		outRows.push_back(row);
	}
}

void TaechangWorkflowResultPresenter::AddEstimateInputRows(
	const CString& strResponseJson,
	std::vector<TaechangResultRow>& outRows) const {
	CString strRowsJson = JsonExtractArray(strResponseJson, TAECHANG_JSON_KEY_ROWS);
	std::vector<CString> arrObjects;
	SplitJsonObjectArray(strRowsJson, arrObjects);
	for (int i = 0; i < static_cast<int>(arrObjects.size()); ++i) {
		CString strRowNum = JsonExtractIntText(arrObjects[i], TAECHANG_JSON_KEY_ROW_NUM);
		CString strCompanyName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_COMPANY_NAME);
		CString strDateText = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_DATE_TEXT);
		CString strItemName = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_ITEM_NAME);
		CString strCopies = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_COPIES);
		CString strPages = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_PAGES);
		CString strUnitPrice = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_UNIT_PRICE);
		CString strCoverCost = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_COVER_COST);
		CString strFreight = JsonExtractString(arrObjects[i], TAECHANG_JSON_KEY_FREIGHT);

		TaechangResultRow row;
		row.m_nSourceRowIndex = _wtoi(strRowNum);
		row.m_strField = strRowNum;
		row.m_strCompanyName = strCompanyName;
		row.m_strIssueDate = strDateText;
		row.m_strItemName = strItemName;
		row.m_strCompanyCopies = strCopies;
		row.m_strCorporationCopies = strPages;
		row.m_strTotalCopies = FormatAmountText(strUnitPrice);
		row.m_strValue = FormatAmountText(strCoverCost);
		row.m_strReason = FormatAmountText(strFreight);
		outRows.push_back(row);
	}
}
