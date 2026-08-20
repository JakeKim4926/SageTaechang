#pragma once

#include "pch.h"

class RepositoryHelper {
public:
    static CString GetLastError(sqlite3* pDb) {
        CString strError;
        CStringA strMessageA;

        if (pDb == NULL) {
            return _T("SQLite DB 객체가 NULL입니다.");
        }

        strMessageA = sqlite3_errmsg(pDb);
        strError = CString(CA2T(strMessageA.GetString(), CP_UTF8));

        return strError;
    }

    static CString GetColumnText(sqlite3_stmt* pStatement, int nColumn) {
        const unsigned char* pszText;
        CString strValue;

        pszText = sqlite3_column_text(pStatement, nColumn);

        if (pszText == NULL) {
            return _T("");
        }

        strValue = CString(CA2T((LPCSTR)pszText, CP_UTF8));

        return strValue;
    }

    static BOOL IsColumnNull(sqlite3_stmt* pStatement, int nColumn) {
        return sqlite3_column_type(pStatement, nColumn) == SQLITE_NULL;
    }

    static BOOL BindInt(sqlite3_stmt* pStatement, int nIndex, int nValue, CString& strError) {
        int nResult;

        nResult = sqlite3_bind_int(pStatement, nIndex, nValue);

        if (nResult != SQLITE_OK) {
            strError.Format(_T("SQLite int 바인딩 실패. Index=%d, ErrorCode=%d"), nIndex, nResult);
            return FALSE;
        }

        return TRUE;
    }

    static BOOL BindNull(sqlite3_stmt* pStatement, int nIndex, CString& strError) {
        int nResult;

        nResult = sqlite3_bind_null(pStatement, nIndex);

        if (nResult != SQLITE_OK) {
            strError.Format(_T("SQLite NULL 바인딩 실패. Index=%d, ErrorCode=%d"), nIndex, nResult);
            return FALSE;
        }

        return TRUE;
    }

    static BOOL BindText(sqlite3_stmt* pStatement, int nIndex, const CString& strValue, CString& strError) {
        CStringA strValueA;
        int nResult;

        strValueA = CT2A(strValue.GetString(), CP_UTF8);

        nResult = sqlite3_bind_text(
            pStatement,
            nIndex,
            strValueA.GetString(),
            -1,
            SQLITE_TRANSIENT
        );

        if (nResult != SQLITE_OK) {
            strError.Format(_T("SQLite text 바인딩 실패. Index=%d, ErrorCode=%d"), nIndex, nResult);
            return FALSE;
        }

        return TRUE;
    }
};