#include "pch.h"
#include "app/core/workflow/SageWorkflowResultTable.h"
#include "app/core/workflow/TaechangWorkflowResultPresenter.h"
#include "TaechangDefine.h"

namespace {

const SageWorkflowColumn g_genericColumns[] = {
	{ TAECHANG_UI_RESULT_FIELD, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_FIELD_WIDTH, FALSE, SAGE_RESULT_FIELD_FIELD },
	{ TAECHANG_UI_RESULT_VALUE, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_MIN_VALUE_WIDTH, TRUE, SAGE_RESULT_FIELD_VALUE },
	{ TAECHANG_UI_RESULT_STATUS, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_STATUS_WIDTH, FALSE, SAGE_RESULT_FIELD_STATUS },
	{ TAECHANG_UI_RESULT_REASON, SAGE_COLUMN_ALIGN_LEFT, TAECHANG_RESULT_REASON_WIDTH, FALSE, SAGE_RESULT_FIELD_REASON }
};

constexpr int SAGE_GENERIC_COLUMN_COUNT = sizeof(g_genericColumns) / sizeof(g_genericColumns[0]);

}

namespace SageWorkflowResultTable {

int GetGenericColumnCount() {
	return SAGE_GENERIC_COLUMN_COUNT;
}

const SageWorkflowColumn& GetGenericColumn(int nColumnIndex) {
	return g_genericColumns[nColumnIndex];
}

CString GetRowText(const TaechangResultRow& row, SageResultField nField) {
	switch (nField) {
	case SAGE_RESULT_FIELD_VALUE:               return row.m_strValue;
	case SAGE_RESULT_FIELD_STATUS:              return row.m_strStatus;
	case SAGE_RESULT_FIELD_REASON:              return row.m_strReason;
	case SAGE_RESULT_FIELD_COMPANY_NAME:        return row.m_strCompanyName;
	case SAGE_RESULT_FIELD_DEPARTMENT:          return row.m_strDepartment;
	case SAGE_RESULT_FIELD_ORDER_DATE:          return row.m_strOrderDate;
	case SAGE_RESULT_FIELD_DELIVERY_DATE:       return row.m_strDeliveryDate;
	case SAGE_RESULT_FIELD_DELIVERY_TIME:       return row.m_strDeliveryTime;
	case SAGE_RESULT_FIELD_MANAGER:             return row.m_strManager;
	case SAGE_RESULT_FIELD_ISSUE_DATE:          return row.m_strIssueDate;
	case SAGE_RESULT_FIELD_ITEM_NAME:           return row.m_strItemName;
	case SAGE_RESULT_FIELD_PRODUCT_TYPE:        return row.m_strProductType;
	case SAGE_RESULT_FIELD_COMPANY_COPIES:      return row.m_strCompanyCopies;
	case SAGE_RESULT_FIELD_CORPORATION_COPIES:  return row.m_strCorporationCopies;
	case SAGE_RESULT_FIELD_TOTAL_COPIES:        return row.m_strTotalCopies;
	case SAGE_RESULT_FIELD_ISSUE_TYPE:          return row.m_strIssueType;
	case SAGE_RESULT_FIELD_TOTAL_AMOUNT:        return row.m_strTotalAmount;
	case SAGE_RESULT_FIELD_DEPOSIT_AMOUNT:      return row.m_strDepositAmount;
	case SAGE_RESULT_FIELD_RECEIVABLE_AMOUNT:   return row.m_strReceivableAmount;
	case SAGE_RESULT_FIELD_BANK_NAME:           return row.m_strBankName;
	case SAGE_RESULT_FIELD_NOTE:                return row.m_strNote;
	default:                                    return row.m_strField;
	}
}

CString FormatAmountNumber(__int64 nAmount) {
	BOOL bNegative = (nAmount < 0) ? TRUE : FALSE;
	CString strDigits;
	strDigits.Format(TAECHANG_UI_SUMMARY_AMOUNT_FORMAT, bNegative ? -nAmount : nAmount);

	for (int i = strDigits.GetLength() - TAECHANG_AMOUNT_GROUP_DIGITS; i > 0; i -= TAECHANG_AMOUNT_GROUP_DIGITS)
		strDigits.Insert(i, TAECHANG_UI_AMOUNT_GROUP_SEPARATOR);

	return bNegative ? CString(TAECHANG_UI_AMOUNT_NEGATIVE_MARK) + strDigits : strDigits;
}

void DistributeColumnWidths(
	const std::vector<SageColumnWidthSpec>& arrSpecs,
	int nTotalWidth,
	std::vector<int>& outWidths) {
	outWidths.clear();
	int nColumnCount = static_cast<int>(arrSpecs.size());
	if (nColumnCount == 0 || nTotalWidth <= 0)
		return;

	int nFixedWidth = 0;
	int nStretchDefinedWidth = 0;
	int nDefinedWidth = 0;
	int nLastStretchIndex = TAECHANG_LIST_NO_ITEM;
	for (int i = 0; i < nColumnCount; ++i) {
		nDefinedWidth += arrSpecs[i].nWidth;
		if (arrSpecs[i].bStretch) {
			nStretchDefinedWidth += arrSpecs[i].nWidth;
			nLastStretchIndex = i;
			continue;
		}
		nFixedWidth += arrSpecs[i].nWidth;
	}

	outWidths.resize(nColumnCount);
	if (nStretchDefinedWidth > 0) {
		int nStretchWidth = nTotalWidth - nFixedWidth;
		if (nStretchWidth < nStretchDefinedWidth) {
			for (int i = 0; i < nColumnCount; ++i)
				outWidths[i] = arrSpecs[i].nWidth;
			return;
		}

		int nAssignedWidth = 0;
		for (int i = 0; i < nColumnCount; ++i) {
			if (!arrSpecs[i].bStretch) {
				outWidths[i] = arrSpecs[i].nWidth;
				continue;
			}
			outWidths[i] = (i == nLastStretchIndex)
				? nStretchWidth - nAssignedWidth
				: ::MulDiv(arrSpecs[i].nWidth, nStretchWidth, nStretchDefinedWidth);
			nAssignedWidth += outWidths[i];
		}
		return;
	}

	int nAssignedWidth = 0;
	for (int i = 0; i < nColumnCount; ++i) {
		outWidths[i] = arrSpecs[i].nWidth;
		if (nTotalWidth > nDefinedWidth) {
			outWidths[i] = (i == nColumnCount - 1)
				? nTotalWidth - nAssignedWidth
				: ::MulDiv(arrSpecs[i].nWidth, nTotalWidth, nDefinedWidth);
		}
		nAssignedWidth += outWidths[i];
	}
}

}
