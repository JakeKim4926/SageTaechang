#pragma once

#include "pch.h"

CString FormatPrice(LONGLONG nPrice);
CString RemovePriceSeparators(const CString& strText);
int PriceTextToInt(const CString& strText);
void FormatPriceEditText(CEdit& edit, BOOL& bFormatting);
