#include "pch.h"
#include "app/application/services/TaechangCompareCsvExportService.h"
#include "app/common/TaechangJson.h"

namespace
{
	CString ExtractJsonArray(const CString& strJson, const CString& strKey) {
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
			if (ch == '[')
				++nDepth;
			else if (ch == ']') {
				--nDepth;
				if (nDepth == 0)
					return Utf8ToWide(json.substr(nStart, i - nStart + 1));
			}
		}
		return L"";
	}

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

	CString EscapeCsvField(const CString& strValue) {
		CString strEscaped = strValue;
		strEscaped.Replace(L"\"", L"\"\"");
		if (strEscaped.Find(L',') >= 0 || strEscaped.Find(L'\"') >= 0 || strEscaped.Find(L'\n') >= 0 || strEscaped.Find(L'\r') >= 0)
			return L"\"" + strEscaped + L"\"";
		return strEscaped;
	}

	CString BuildCsvLine(
		const CString& strFilePath,
		const CString& strFileName,
		const CString& strStatus,
		const CString& strReason,
		const CString& strLeftValue,
		const CString& strRightValue) {
		return EscapeCsvField(strFilePath) + L"," +
			EscapeCsvField(strFileName) + L"," +
			EscapeCsvField(strStatus) + L"," +
			EscapeCsvField(strReason) + L"," +
			EscapeCsvField(strLeftValue) + L"," +
			EscapeCsvField(strRightValue) + L"\r\n";
	}

	BOOL WriteUtf8CsvFile(const CString& strOutputPath, const CString& strContent, CString& strError) {
		std::ofstream file(WideToUtf8(strOutputPath), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file.is_open()) {
			strError = L"CSV 파일을 열 수 없습니다.";
			return FALSE;
		}

		const unsigned char arrBom[3] = { 0xEF, 0xBB, 0xBF };
		file.write(reinterpret_cast<const char*>(arrBom), 3);
		std::string strUtf8 = WideToUtf8(strContent);
		file.write(strUtf8.c_str(), static_cast<std::streamsize>(strUtf8.size()));
		if (!file.good()) {
			strError = L"CSV 파일 저장 중 오류가 발생했습니다.";
			return FALSE;
		}
		return TRUE;
	}
}

BOOL TaechangCompareCsvExportService::ExportCompareResult(
	const CString& strResponseJson,
	const CString& strOutputPath,
	CString& strError) {
	CString strFilesJson = ExtractJsonArray(strResponseJson, L"files");
	std::vector<CString> arrObjects;
	SplitJsonObjectArray(strFilesJson, arrObjects);
	if (arrObjects.empty()) {
		strError = L"저장할 검수 결과가 없습니다.";
		return FALSE;
	}

	CString strContent = L"filePath,fileName,status,reason,leftValue,rightValue\r\n";
	for (int i = 0; i < static_cast<int>(arrObjects.size()); ++i) {
		CString strFilePath = JsonExtractString(arrObjects[i], L"filePath");
		CString strFileName = JsonExtractString(arrObjects[i], L"fileName");
		CString strStatus = JsonExtractString(arrObjects[i], L"status");
		CString strReason = JsonExtractString(arrObjects[i], L"reason");
		CString strLeftValue = JsonExtractString(arrObjects[i], L"leftValue");
		CString strRightValue = JsonExtractString(arrObjects[i], L"rightValue");
		strContent += BuildCsvLine(strFilePath, strFileName, strStatus, strReason, strLeftValue, strRightValue);
	}

	return WriteUtf8CsvFile(strOutputPath, strContent, strError);
}
