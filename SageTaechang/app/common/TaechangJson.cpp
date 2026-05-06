#include "pch.h"
#include "app/common/TaechangJson.h"

std::string WideToUtf8(const CString& strWide) {
	int nLen = WideCharToMultiByte(CP_UTF8, 0, strWide, -1, nullptr, 0, nullptr, nullptr);
	if (nLen <= 1)
		return "";

	std::string strUtf8(nLen - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, strWide, -1, &strUtf8[0], nLen, nullptr, nullptr);
	return strUtf8;
}

CString Utf8ToWide(const std::string& strUtf8) {
	int nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
	if (nLen <= 1)
		return L"";

	CString strWide;
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.GetBuffer(nLen), nLen);
	strWide.ReleaseBuffer();
	return strWide;
}

CString JsonExtractString(const CString& strJson, const CString& strKey) {
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
	while (nStart < json.size() && json[nStart] == ' ')
		++nStart;

	if (nStart >= json.size() || json[nStart] != '"')
		return L"";

	size_t nEnd = nStart + 1;
	while (nEnd < json.size()) {
		if (json[nEnd] == '\\') {
			nEnd += 2;
			continue;
		}

		if (json[nEnd] == '"')
			break;

		++nEnd;
	}

	if (nEnd >= json.size())
		return L"";

	std::string raw = json.substr(nStart + 1, nEnd - nStart - 1);
	std::string unescaped;
	unescaped.reserve(raw.size());
	for (size_t i = 0; i < raw.size(); ++i) {
		if (raw[i] == '\\' && i + 1 < raw.size()) {
			++i;
			switch (raw[i]) {
				case '"':  unescaped += '"';  break;
				case '\\': unescaped += '\\'; break;
				case '/':  unescaped += '/';  break;
				case 'n':  unescaped += '\n'; break;
				case 'r':  unescaped += '\r'; break;
				case 't':  unescaped += '\t'; break;
				default:   unescaped += raw[i]; break;
			}
		} else {
			unescaped += raw[i];
		}
	}

	return Utf8ToWide(unescaped);
}

BOOL JsonExtractBool(const CString& strJson, const CString& strKey) {
	std::string json = WideToUtf8(strJson);
	std::string key = WideToUtf8(strKey);
	std::string token = "\"" + key + "\"";

	size_t nKeyPos = json.find(token);
	if (nKeyPos == std::string::npos)
		return FALSE;

	size_t nColon = json.find(':', nKeyPos + token.size());
	if (nColon == std::string::npos)
		return FALSE;

	size_t nStart = nColon + 1;
	while (nStart < json.size() && json[nStart] == ' ')
		++nStart;

	return (json.compare(nStart, 4, "true") == 0) ? TRUE : FALSE;
}

CString JsonExtractArray(const CString& strJson, const CString& strKey) {
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
	for (size_t i = nStart; i < json.size(); ++i) {
		char ch = json[i];
		if (bEscaped) { bEscaped = false; continue; }
		if (ch == '\\' && bInString) { bEscaped = true; continue; }
		if (ch == '"') { bInString = !bInString; continue; }
		if (bInString) continue;
		if (ch == '[') ++nDepth;
		else if (ch == ']') {
			--nDepth;
			if (nDepth == 0)
				return Utf8ToWide(json.substr(nStart, i - nStart + 1));
		}
	}
	return L"";
}

void JsonSplitStringArray(const CString& strArrayJson, std::vector<CString>& outValues) {
	std::string strJson = WideToUtf8(strArrayJson);
	bool bInString = false;
	bool bEscaped = false;
	std::string strCurrent;
	for (size_t i = 0; i < strJson.size(); ++i) {
		char ch = strJson[i];
		if (!bInString) {
			if (ch == '"') { bInString = true; strCurrent.clear(); }
			continue;
		}
		if (bEscaped) {
			if (ch == 'n') strCurrent += '\n';
			else if (ch == 'r') strCurrent += '\r';
			else if (ch == 't') strCurrent += '\t';
			else strCurrent += ch;
			bEscaped = false;
			continue;
		}
		if (ch == '\\') { bEscaped = true; continue; }
		if (ch == '"') { outValues.push_back(Utf8ToWide(strCurrent)); bInString = false; continue; }
		strCurrent += ch;
	}
}

CString JsonEscapeString(const CString& strValue) {
	CString strResult;
	for (int i = 0; i < strValue.GetLength(); ++i) {
		wchar_t ch = strValue[i];
		switch (ch) {
			case L'"':  strResult += L"\\\""; break;
			case L'\\': strResult += L"\\\\"; break;
			case L'\n': strResult += L"\\n";  break;
			case L'\r': strResult += L"\\r";  break;
			case L'\t': strResult += L"\\t";  break;
			default:    strResult += ch;      break;
		}
	}

	return strResult;
}
