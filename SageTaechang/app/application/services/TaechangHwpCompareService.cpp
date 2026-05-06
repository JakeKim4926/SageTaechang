#include "pch.h"
#include "app/application/services/TaechangHwpCompareService.h"
#include "app/common/TaechangJson.h"
#include "app/common/TaechangFileUtils.h"
#include "app/infrastructure/bridge/TaechangBridgeResponse.h"
#include <oleauto.h>
#include <cwctype>
#include <filesystem>

namespace
{
	constexpr LPCWSTR TAECHANG_HWP_PROG_ID = L"HWPFrame.HwpObject";
	constexpr LPCWSTR TAECHANG_HWP_ERR_INPUT_REQUIRED = L"SNX_TAECHANG_HWP_001";
	constexpr LPCWSTR TAECHANG_HWP_ERR_PROCESS_FAILED = L"SNX_TAECHANG_HWP_003";

	CString HResultToText(HRESULT hr) {
		CString strText;
		strText.Format(L"HRESULT=0x%08X", static_cast<UINT>(hr));
		return strText;
	}

	BOOL GetDispatchId(IDispatch* pDispatch, LPCOLESTR pszName, DISPID& outDispId, CString& strError) {
		LPOLESTR pszNames[1] = { const_cast<LPOLESTR>(pszName) };
		HRESULT hr = pDispatch->GetIDsOfNames(IID_NULL, pszNames, 1, LOCALE_USER_DEFAULT, &outDispId);
		if (FAILED(hr)) {
			strError = CString(L"Cannot find HWP method: ") + pszName + L". " + HResultToText(hr);
			return FALSE;
		}
		return TRUE;
	}

	BOOL InvokeHwpMethod(IDispatch* pDispatch, LPCOLESTR pszName, VARIANTARG* pArgs, UINT nArgCount, VARIANT* pResult, CString& strError) {
		DISPID dispId = 0;
		if (!GetDispatchId(pDispatch, pszName, dispId, strError))
			return FALSE;

		DISPPARAMS params = {};
		params.rgvarg = pArgs;
		params.cArgs = nArgCount;
		params.rgdispidNamedArgs = NULL;
		params.cNamedArgs = 0;

		EXCEPINFO excepInfo = {};
		UINT nArgError = 0;
		HRESULT hr = pDispatch->Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, pResult, &excepInfo, &nArgError);
		if (FAILED(hr)) {
			strError = CString(L"HWP method failed: ") + pszName + L". " + HResultToText(hr);
			if (excepInfo.bstrDescription != NULL) {
				strError += L" ";
				strError += excepInfo.bstrDescription;
			}
			SysFreeString(excepInfo.bstrSource);
			SysFreeString(excepInfo.bstrDescription);
			SysFreeString(excepInfo.bstrHelpFile);
			return FALSE;
		}

		SysFreeString(excepInfo.bstrSource);
		SysFreeString(excepInfo.bstrDescription);
		SysFreeString(excepInfo.bstrHelpFile);
		return TRUE;
	}

	void ClearVariantArray(VARIANTARG* pArgs, int nCount) {
		for (int i = 0; i < nCount; ++i)
			VariantClear(&pArgs[i]);
	}

	void TryRegisterFilePathModule(IDispatch* pDispatch) {
		VARIANTARG arrArgs[2];
		VariantInit(&arrArgs[0]);
		VariantInit(&arrArgs[1]);
		arrArgs[0].vt = VT_BSTR;
		arrArgs[0].bstrVal = SysAllocString(L"FilePathCheckerModuleExample");
		arrArgs[1].vt = VT_BSTR;
		arrArgs[1].bstrVal = SysAllocString(L"FilePathCheckDLL");
		VARIANT result;
		VariantInit(&result);
		CString strError;
		InvokeHwpMethod(pDispatch, L"RegisterModule", arrArgs, 2, &result, strError);
		VariantClear(&result);
		ClearVariantArray(arrArgs, 2);
	}

	BOOL OpenHwpDocument(IDispatch* pDispatch, const CString& strPath, CString& strError) {
		VARIANTARG arrArgs[3];
		VariantInit(&arrArgs[0]);
		VariantInit(&arrArgs[1]);
		VariantInit(&arrArgs[2]);
		arrArgs[0].vt = VT_BSTR;
		arrArgs[0].bstrVal = SysAllocString(L"forceopen:true");
		arrArgs[1].vt = VT_BSTR;
		arrArgs[1].bstrVal = SysAllocString(L"HWP");
		arrArgs[2].vt = VT_BSTR;
		arrArgs[2].bstrVal = SysAllocString(strPath);
		VARIANT result;
		VariantInit(&result);
		BOOL bResult = InvokeHwpMethod(pDispatch, L"Open", arrArgs, 3, &result, strError);
		VariantClear(&result);
		ClearVariantArray(arrArgs, 3);
		return bResult;
	}

	BOOL GetHwpXmlText(IDispatch* pDispatch, CString& outXml, CString& strError) {
		VARIANTARG arrArgs[2];
		VariantInit(&arrArgs[0]);
		VariantInit(&arrArgs[1]);
		arrArgs[0].vt = VT_BSTR;
		arrArgs[0].bstrVal = SysAllocString(L"");
		arrArgs[1].vt = VT_BSTR;
		arrArgs[1].bstrVal = SysAllocString(L"HWPML2X");
		VARIANT result;
		VariantInit(&result);
		BOOL bResult = InvokeHwpMethod(pDispatch, L"GetTextFile", arrArgs, 2, &result, strError);
		if (bResult && result.vt == VT_BSTR && result.bstrVal != NULL)
			outXml = result.bstrVal;
		else if (bResult) {
			strError = L"HWP returned empty text.";
			bResult = FALSE;
		}
		VariantClear(&result);
		ClearVariantArray(arrArgs, 2);
		return bResult;
	}

	void ClearHwpDocument(IDispatch* pDispatch) {
		VARIANT result;
		VariantInit(&result);
		CString strError;
		InvokeHwpMethod(pDispatch, L"Clear", NULL, 0, &result, strError);
		VariantClear(&result);
	}

	BOOL ExtractHwpXml(const CString& strPath, CString& outXml, CString& strError) {
		if (!FileExists(strPath)) {
			strError = L"HWP file was not found.";
			return FALSE;
		}

		HRESULT hrCo = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		BOOL bUninitialize = SUCCEEDED(hrCo) ? TRUE : FALSE;
		if (FAILED(hrCo) && hrCo != RPC_E_CHANGED_MODE) {
			strError = L"COM initialization failed. " + HResultToText(hrCo);
			return FALSE;
		}

		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(TAECHANG_HWP_PROG_ID, &clsid);
		if (FAILED(hr)) {
			strError = L"Hancom HWP automation is not installed. " + HResultToText(hr);
			if (bUninitialize)
				CoUninitialize();
			return FALSE;
		}

		IDispatch* pDispatch = NULL;
		hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, reinterpret_cast<void**>(&pDispatch));
		if (FAILED(hr) || pDispatch == NULL) {
			strError = L"Cannot create HWP automation object. " + HResultToText(hr);
			if (bUninitialize)
				CoUninitialize();
			return FALSE;
		}

		TryRegisterFilePathModule(pDispatch);
		BOOL bResult = OpenHwpDocument(pDispatch, strPath, strError);
		if (bResult)
			bResult = GetHwpXmlText(pDispatch, outXml, strError);
		ClearHwpDocument(pDispatch);
		pDispatch->Release();
		if (bUninitialize)
			CoUninitialize();
		return bResult;
	}

	CString DecodeXmlEntities(const CString& strValue) {
		CString strResult = strValue;
		strResult.Replace(L"&lt;", L"<");
		strResult.Replace(L"&gt;", L">");
		strResult.Replace(L"&amp;", L"&");
		strResult.Replace(L"&quot;", L"\"");
		strResult.Replace(L"&apos;", L"'");
		return strResult;
	}

	CString StripXmlTags(const CString& strXml) {
		CString strResult;
		BOOL bInTag = FALSE;
		for (int i = 0; i < strXml.GetLength(); ++i) {
			wchar_t ch = strXml[i];
			if (ch == L'<') {
				bInTag = TRUE;
				strResult += L' ';
				continue;
			}
			if (ch == L'>') {
				bInTag = FALSE;
				continue;
			}
			if (!bInTag)
				strResult += ch;
		}
		return DecodeXmlEntities(strResult);
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

	BOOL CellHasColumn(const CString& strCell, int nColumn) {
		CString strToken1;
		CString strToken2;
		CString strToken3;
		CString strToken4;
		strToken1.Format(L"ColAddr=\"%d\"", nColumn);
		strToken2.Format(L"ColAddr='%d'", nColumn);
		strToken3.Format(L"colAddr=\"%d\"", nColumn);
		strToken4.Format(L"colAddr='%d'", nColumn);
		return (strCell.Find(strToken1) >= 0 || strCell.Find(strToken2) >= 0 || strCell.Find(strToken3) >= 0 || strCell.Find(strToken4) >= 0) ? TRUE : FALSE;
	}

	void ExtractColumnText(const CString& strXml, int nColumn, CString& outText) {
		int nSearch = 0;
		while (TRUE) {
			int nStart = strXml.Find(L"<CELL", nSearch);
			if (nStart < 0)
				nStart = strXml.Find(L"<cell", nSearch);
			if (nStart < 0)
				break;
			int nEnd = strXml.Find(L"</CELL>", nStart);
			if (nEnd < 0)
				nEnd = strXml.Find(L"</cell>", nStart);
			if (nEnd < 0)
				break;
			int nLength = nEnd - nStart + 7;
			CString strCell = strXml.Mid(nStart, nLength);
			if (CellHasColumn(strCell, nColumn)) {
				CString strText = CollapseWhitespace(StripXmlTags(strCell));
				if (!strText.IsEmpty()) {
					if (!outText.IsEmpty())
						outText += L"\n";
					outText += strText;
				}
			}
			nSearch = nEnd + 7;
		}
	}

	void SplitHwpLeftRightText(const CString& strXml, CString& outLeft, CString& outRight) {
		ExtractColumnText(strXml, 0, outLeft);
		ExtractColumnText(strXml, 2, outRight);
		if (outLeft.IsEmpty() || outRight.IsEmpty()) {
			CString strText = CollapseWhitespace(StripXmlTags(strXml));
			int nSplit = strText.GetLength() / 2;
			outLeft = strText.Left(nSplit);
			outRight = strText.Mid(nSplit);
		}
	}

	CString BuildSingleCompareItem(const CString& strHwpPath) {
		CString strXml;
		CString strError;
		BOOL bSuccess = ExtractHwpXml(strHwpPath, strXml, strError);
		CString strFileName = std::filesystem::path(static_cast<LPCWSTR>(strHwpPath)).filename().wstring().c_str();
		if (!bSuccess) {
			CString strErrorItem;
			strErrorItem.Format(L"{\"filePath\":\"%s\",\"fileName\":\"%s\",\"status\":\"error\",\"matched\":false,\"reason\":\"%s\"}", (LPCWSTR)JsonEscapeString(strHwpPath), (LPCWSTR)JsonEscapeString(strFileName), (LPCWSTR)JsonEscapeString(strError));
			return strErrorItem;
		}
		CString strLeft;
		CString strRight;
		SplitHwpLeftRightText(strXml, strLeft, strRight);
		CString strLeftNormalized = NormalizeVisibleText(strLeft);
		CString strRightNormalized = NormalizeVisibleText(strRight);
		BOOL bMatched = (!strLeftNormalized.IsEmpty() && strLeftNormalized == strRightNormalized) ? TRUE : FALSE;
		CString strStatus = bMatched ? L"pass" : L"fail";
		CString strReason = bMatched ? L"left/right HWP table text matched" : L"left/right HWP table text differs";
		CString strItem;
		strItem.Format(L"{\"filePath\":\"%s\",\"fileName\":\"%s\",\"status\":\"%s\",\"matched\":%s,\"reason\":\"%s\",\"leftValue\":\"%s\",\"rightValue\":\"%s\"}", (LPCWSTR)JsonEscapeString(strHwpPath), (LPCWSTR)JsonEscapeString(strFileName), (LPCWSTR)JsonEscapeString(strStatus), bMatched ? L"true" : L"false", (LPCWSTR)JsonEscapeString(strReason), (LPCWSTR)JsonEscapeString(strLeftNormalized.Left(120)), (LPCWSTR)JsonEscapeString(strRightNormalized.Left(120)));
		return strItem;
	}

	CString BuildBatchCompareResponse(const CString& strRequestId, const std::vector<CString>& arrHwpPaths) {
		if (arrHwpPaths.empty())
			return BuildErrorResponse(strRequestId, TAECHANG_HWP_ERR_INPUT_REQUIRED, L"Select HWP files.");
		int nPass = 0;
		int nFail = 0;
		CString strItems = L"[";
		for (int i = 0; i < static_cast<int>(arrHwpPaths.size()); ++i) {
			CString strItem = BuildSingleCompareItem(arrHwpPaths[i]);
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
		strPayload.Format(L"{\"status\":\"%s\",\"extractor\":\"hwp-automation-hwpml2x\",\"totalFiles\":%d,\"passedFiles\":%d,\"failedFiles\":%d,\"files\":%s}", (LPCWSTR)JsonEscapeString(strStatus), static_cast<int>(arrHwpPaths.size()), nPass, nFail, (LPCWSTR)strItems);
		return BuildSuccessResponse(strRequestId, strPayload);
	}
}

CString TaechangHwpCompareService::BuildOpenMultiFileDialogResponse(const CString& strRequestId, const CString& strPayloadJson) {
	UNREFERENCED_PARAMETER(strPayloadJson);
	return BuildErrorResponse(strRequestId, TAECHANG_HWP_ERR_INPUT_REQUIRED, L"Use the MFC file picker.");
}

CString TaechangHwpCompareService::BuildRunCompareResponse(const CString& strRequestId, const CString& strPayloadJson) {
	CString strFilePathsJson = JsonExtractArray(strPayloadJson, L"hwpFilePaths");
	std::vector<CString> arrHwpPaths;
	JsonSplitStringArray(strFilePathsJson, arrHwpPaths);
	return BuildBatchCompareResponse(strRequestId, arrHwpPaths);
}

CString TaechangHwpCompareService::BuildExportCsvResponse(const CString& strRequestId, const CString& strPayloadJson) {
	UNREFERENCED_PARAMETER(strPayloadJson);
	return BuildErrorResponse(strRequestId, TAECHANG_HWP_ERR_PROCESS_FAILED, L"CSV export is not implemented in MFC HWP compare yet.");
}
