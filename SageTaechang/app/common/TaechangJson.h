#pragma once

std::string WideToUtf8(const CString& strWide);
CString Utf8ToWide(const std::string& strUtf8);
CString JsonExtractString(const CString& strJson, const CString& strKey);
BOOL JsonExtractBool(const CString& strJson, const CString& strKey);
CString JsonEscapeString(const CString& strValue);
CString JsonExtractArray(const CString& strJson, const CString& strKey);
void JsonSplitStringArray(const CString& strArrayJson, std::vector<CString>& outValues);
