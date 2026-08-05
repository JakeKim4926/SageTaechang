#include "pch.h"
#include "app/common/SageNumberFormat.h"
#include "TaechangDefine.h"

CString FormatPrice(LONGLONG nPrice) {
    CString str;
    str.Format(TAECHANG_UI_NUMBER_FORMAT, nPrice);
    int nLen = str.GetLength();
    for (int i = nLen - TAECHANG_THOUSAND_SEPARATOR_STEP; i > 0; i -= TAECHANG_THOUSAND_SEPARATOR_STEP)
        str.Insert(i, TAECHANG_THOUSAND_SEPARATOR);
    return str;
}

CString RemovePriceSeparators(const CString& strText) {
    CString strResult = strText;
    strResult.Remove(TAECHANG_THOUSAND_SEPARATOR);
    strResult.Trim();
    return strResult;
}

int PriceTextToInt(const CString& strText) {
    CString strValue = RemovePriceSeparators(strText);
    return strValue.IsEmpty() ? 0 : _wtoi(strValue);
}

void FormatPriceEditText(CEdit& edit, BOOL& bFormatting) {
    if (bFormatting)
        return;

    CString strText;
    edit.GetWindowTextW(strText);
    CString strDigits = RemovePriceSeparators(strText);
    if (strDigits.IsEmpty())
        return;

    for (int i = 0; i < strDigits.GetLength(); ++i) {
        wchar_t ch = strDigits[i];
        if (ch < L'0' || ch > L'9')
            return;
    }

    CString strFormatted = FormatPrice(_wtoi(strDigits));
    if (strFormatted == strText)
        return;

    bFormatting = TRUE;
    edit.SetWindowTextW(strFormatted);
    edit.SetSel(strFormatted.GetLength(), strFormatted.GetLength());
    bFormatting = FALSE;
}
