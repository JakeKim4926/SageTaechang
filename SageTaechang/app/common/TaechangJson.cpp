#include "pch.h"
#include "app/common/TaechangJson.h"

// HncShellExt64.dll 등 버그 있는 쉘 익스텐션이 포커스 변경 시 AV를 일으킬 수 있다.
// IFileDialog::Show()를 별도 함수에서 SEH로 감싸 크래시를 방지한다.
// __try/__except는 C++ 소멸자 객체와 같은 스코프에 둘 수 없으므로 분리 필수.
HRESULT SafeShowDialog(IFileDialog* pDialog, HWND hOwner)
{
    __try
    {
        return pDialog->Show(hOwner);
    }
    __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
              ? EXCEPTION_EXECUTE_HANDLER
              : EXCEPTION_CONTINUE_SEARCH)
    {
        return E_FAIL;
    }
}

std::string WideToUtf8(const CString& strWide)
{
    int nLen = WideCharToMultiByte(CP_UTF8, 0, strWide, -1, nullptr, 0, nullptr, nullptr);
    if (nLen <= 1)
        return "";

    std::string strUtf8(nLen - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, strWide, -1, &strUtf8[0], nLen, nullptr, nullptr);
    return strUtf8;
}

CString Utf8ToWide(const std::string& strUtf8)
{
    int nLen = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
    if (nLen <= 1)
        return L"";

    CString strWide;
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strWide.GetBuffer(nLen), nLen);
    strWide.ReleaseBuffer();
    return strWide;
}

CString JsonExtractString(const CString& strJson, const CString& strKey)
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
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
    while (nEnd < json.size())
    {
        if (json[nEnd] == '\\')
        {
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
    for (size_t i = 0; i < raw.size(); ++i)
    {
        if (raw[i] == '\\' && i + 1 < raw.size())
        {
            ++i;
            switch (raw[i])
            {
            case '"':  unescaped += '"';  break;
            case '\\': unescaped += '\\'; break;
            case '/':  unescaped += '/';  break;
            case 'n':  unescaped += '\n'; break;
            case 'r':  unescaped += '\r'; break;
            case 't':  unescaped += '\t'; break;
            default:   unescaped += raw[i]; break;
            }
        }
        else
        {
            unescaped += raw[i];
        }
    }

    return Utf8ToWide(unescaped);
}

BOOL JsonExtractBool(const CString& strJson, const CString& strKey)
{
    std::string json  = WideToUtf8(strJson);
    std::string key   = WideToUtf8(strKey);
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

BOOL ShowIFileOpenDialog(
    HWND hOwner,
    LPCWSTR pszTitle,
    LPCWSTR pszDefExt,
    const COMDLG_FILTERSPEC* paTypes,
    UINT nTypes,
    BOOL bMultiSelect,
    std::vector<CString>& outPaths)
{
    IFileOpenDialog* pDialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&pDialog))))
        return FALSE;

    DWORD dwOptions = 0;
    pDialog->GetOptions(&dwOptions);
    dwOptions |= FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST;
    if (bMultiSelect)
        dwOptions |= FOS_ALLOWMULTISELECT;
    pDialog->SetOptions(dwOptions);

    if (nTypes > 0 && paTypes)
        pDialog->SetFileTypes(nTypes, paTypes);
    if (pszDefExt && *pszDefExt)
        pDialog->SetDefaultExtension(pszDefExt);
    if (pszTitle && *pszTitle)
        pDialog->SetTitle(pszTitle);

    BOOL bResult = FALSE;
    if (SUCCEEDED(SafeShowDialog(pDialog, hOwner)))
    {
        IShellItemArray* pItems = nullptr;
        if (SUCCEEDED(pDialog->GetResults(&pItems)) && pItems)
        {
            DWORD nCount = 0;
            pItems->GetCount(&nCount);
            for (DWORD i = 0; i < nCount; ++i)
            {
                IShellItem* pItem = nullptr;
                if (SUCCEEDED(pItems->GetItemAt(i, &pItem)) && pItem)
                {
                    LPWSTR pszPath = nullptr;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath)
                    {
                        outPaths.push_back(CString(pszPath));
                        CoTaskMemFree(pszPath);
                    }
                    pItem->Release();
                }
            }
            pItems->Release();
            bResult = !outPaths.empty() ? TRUE : FALSE;
        }
    }
    pDialog->Release();
    return bResult;
}

CString ShowIFileSaveDialog(
    HWND hOwner,
    LPCWSTR pszTitle,
    LPCWSTR pszDefExt,
    const COMDLG_FILTERSPEC* paTypes,
    UINT nTypes,
    LPCWSTR pszInitialName)
{
    IFileSaveDialog* pDialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&pDialog))))
        return L"";

    DWORD dwOptions = 0;
    pDialog->GetOptions(&dwOptions);
    pDialog->SetOptions(dwOptions | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT);

    if (nTypes > 0 && paTypes)
        pDialog->SetFileTypes(nTypes, paTypes);
    if (pszDefExt && *pszDefExt)
        pDialog->SetDefaultExtension(pszDefExt);
    if (pszTitle && *pszTitle)
        pDialog->SetTitle(pszTitle);
    if (pszInitialName && *pszInitialName)
        pDialog->SetFileName(pszInitialName);

    CString strResult;
    if (SUCCEEDED(SafeShowDialog(pDialog, hOwner)))
    {
        IShellItem* pItem = nullptr;
        if (SUCCEEDED(pDialog->GetResult(&pItem)) && pItem)
        {
            LPWSTR pszPath = nullptr;
            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath)
            {
                strResult = pszPath;
                CoTaskMemFree(pszPath);
            }
            pItem->Release();
        }
    }
    pDialog->Release();
    return strResult;
}

HWND GetAppMainWindow()
{
    struct EnumCtx { DWORD dwPid; HWND hFound; };
    EnumCtx ctx = { GetCurrentProcessId(), nullptr };

    EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL
    {
        EnumCtx* pCtx = reinterpret_cast<EnumCtx*>(lParam);
        DWORD dwPid = 0;
        GetWindowThreadProcessId(hWnd, &dwPid);
        if (dwPid == pCtx->dwPid &&
            IsWindowVisible(hWnd) &&
            GetWindow(hWnd, GW_OWNER) == nullptr)
        {
            pCtx->hFound = hWnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&ctx));

    return ctx.hFound;
}

CString JsonEscapeString(const CString& strValue)
{
    CString strResult;
    for (int i = 0; i < strValue.GetLength(); ++i)
    {
        wchar_t ch = strValue[i];
        switch (ch)
        {
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
