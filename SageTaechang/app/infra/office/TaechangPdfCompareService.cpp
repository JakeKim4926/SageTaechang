#include "pch.h"
#include "app/application/services/TaechangPdfCompareService.h"
#include "app/application/services/TaechangAppSettingsService.h"
#include "app/common/TaechangJson.h"
#include "app/common/TaechangFileUtils.h"
#include "app/infrastructure/bridge/TaechangBridgeResponse.h"
#include "TaechangDefine.h"
#include <filesystem>

namespace
{
	constexpr LPCWSTR TAECHANG_PDF_ERR_INPUT_REQUIRED = L"SNX_TAECHANG_PDF_001";
	constexpr LPCWSTR TAECHANG_PDF_ERR_EXTRACT_FAILED = L"SNX_TAECHANG_PDF_002";
	constexpr LPCWSTR TAECHANG_PDF_ERR_PROCESS_FAILED = L"SNX_TAECHANG_PDF_003";


	CString BuildTempTextPath() {
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



	BOOL ReadUtf8File(const CString& strPath, CString& outContent, CString& strError) {
		std::ifstream file(WideToUtf8(strPath), std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			strError = L"Cannot read extracted text.";
			return FALSE;
		}

		std::ostringstream buffer;
		buffer << file.rdbuf();
		std::string strUtf8 = buffer.str();
		if (strUtf8.size() >= 3 &&
			static_cast<unsigned char>(strUtf8[0]) == 0xEF &&
			static_cast<unsigned char>(strUtf8[1]) == 0xBB &&
			static_cast<unsigned char>(strUtf8[2]) == 0xBF) {
			strUtf8.erase(0, 3);
		}
		outContent = Utf8ToWide(strUtf8);
		return TRUE;
	}

	BOOL ExtractPdfText(const CString& strPdfPath, CString& outText, CString& strError) {
		TaechangAppSettingsService settingsService;
		TaechangAppSettings settings;
		settingsService.Load(settings);
		if (!FileExists(settings.m_strPdfToTextPath)) {
			strError = L"PDF 검수를 사용하려면 pdftotext.exe를 앱 실행 파일과 같은 폴더에 넣어주세요.";
			return FALSE;
		}

		CString strOutputPath = BuildTempTextPath();
		CString strCommandLine = QuoteArgument(settings.m_strPdfToTextPath) +
			L" -layout -enc UTF-8 -q " + QuoteArgument(strPdfPath) + L" " + QuoteArgument(strOutputPath);

		DWORD dwExitCode = 0;
		if (!RunProcessAndWait(strCommandLine, dwExitCode, strError)) {
			DeleteFileW(strOutputPath);
			return FALSE;
		}

		if (dwExitCode != 0) {
			DeleteFileW(strOutputPath);
			strError.Format(L"pdftotext failed. exit=%lu", dwExitCode);
			return FALSE;
		}

		BOOL bRead = ReadUtf8File(strOutputPath, outText, strError);
		DeleteFileW(strOutputPath);
		return bRead;
	}

	CString CollapseWhitespace(const CString& strValue) {
		CString strResult;
		BOOL bPrevSpace = FALSE;
		for (int i = 0; i < strValue.GetLength(); ++i) {
			wchar_t ch = strValue[i];
			if (iswspace(ch) != 0) {
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

	CString NormalizeVisibleText(const CString& strValue) {
		CString strCollapsed = CollapseWhitespace(strValue);
		CString strResult;
		for (int i = 0; i < strCollapsed.GetLength(); ++i) {
			if (iswspace(strCollapsed[i]) == 0)
				strResult += strCollapsed[i];
		}
		return strResult;
	}

	void SplitLeftRightText(const CString& strText, CString& outLeft, CString& outRight) {
		CString strLine;
		for (int i = 0; i <= strText.GetLength(); ++i) {
			wchar_t ch = i < strText.GetLength() ? strText[i] : L'\n';
			if (ch != L'\n' && ch != L'\f') {
				if (ch != L'\r')
					strLine += ch;
				continue;
			}

			int nLength = strLine.GetLength();
			if (nLength > 0) {
				int nSplit = nLength / 2;
				outLeft += strLine.Left(nSplit);
				outLeft += L"\n";
				outRight += strLine.Mid(nSplit);
				outRight += L"\n";
			}
			strLine.Empty();
		}
	}

	CString BuildSingleCompareItem(const CString& strPdfPath) {
		CString strText;
		CString strError;
		BOOL bSuccess = ExtractPdfText(strPdfPath, strText, strError);
		CString strFileName = std::filesystem::path(static_cast<LPCWSTR>(strPdfPath)).filename().wstring().c_str();
		if (!bSuccess) {
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

	CString BuildBatchCompareResponse(const CString& strRequestId, const std::vector<CString>& arrPdfPaths) {
		if (arrPdfPaths.empty())
			return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_INPUT_REQUIRED, L"Select PDF files.");

		int nPass = 0;
		int nFail = 0;
		CString strItems = L"[";
		for (int i = 0; i < static_cast<int>(arrPdfPaths.size()); ++i) {
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

CString TaechangPdfCompareService::BuildOpenFileDialogResponse(const CString& strRequestId, const CString& strPayloadJson) {
	UNREFERENCED_PARAMETER(strPayloadJson);
	return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_INPUT_REQUIRED, L"Use the MFC file picker.");
}

CString TaechangPdfCompareService::BuildOpenMultiFileDialogResponse(const CString& strRequestId, const CString& strPayloadJson) {
	UNREFERENCED_PARAMETER(strPayloadJson);
	return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_INPUT_REQUIRED, L"Use the MFC file picker.");
}

CString TaechangPdfCompareService::BuildRunCompareResponse(const CString& strRequestId, const CString& strPayloadJson) {
	CString strFilePathsJson = JsonExtractArray(strPayloadJson, L"pdfFilePaths");
	std::vector<CString> arrPdfPaths;
	JsonSplitStringArray(strFilePathsJson, arrPdfPaths);
	return BuildBatchCompareResponse(strRequestId, arrPdfPaths);
}

CString TaechangPdfCompareService::BuildExportCsvResponse(const CString& strRequestId, const CString& strPayloadJson) {
	UNREFERENCED_PARAMETER(strPayloadJson);
	return BuildErrorResponse(strRequestId, TAECHANG_PDF_ERR_PROCESS_FAILED, L"CSV export is not implemented in MFC PDF compare yet.");
}
