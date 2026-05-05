#include "pch.h"
#include "SqlContext.h"

#include <atlconv.h>

#define SQL_CONTEXT_DB_FOLDER_NAME        _T("data")
#define SQL_CONTEXT_DB_FILE_NAME          _T("estimate.db")
#define SQL_CONTEXT_BUSY_TIMEOUT_MS       5000

SqlContext::SqlContext() {
    m_pDb = NULL;
}

SqlContext::~SqlContext() {
    Close();
}

BOOL SqlContext::OpenDefault(CString& strError) {
    CString strDbPath;

    strDbPath = GetDefaultDbPath();

    return Open(strDbPath, strError);
}

BOOL SqlContext::Open(const CString& strDbPath, CString& strError) {
    CString strDbDirectory;
    int nResult;

    if (IsOpened() == TRUE) {
        return TRUE;
    }

    strDbDirectory = GetDirectoryPathFromFilePath(strDbPath);

    if (strDbDirectory.IsEmpty() == TRUE) {
        strError.Format(_T("DB 폴더 경로를 찾을 수 없습니다. DBPath=%s"), strDbPath.GetString());
        return FALSE;
    }

    if (EnsureDirectoryExists(strDbDirectory, strError) == FALSE) {
        return FALSE;
    }

    nResult = sqlite3_open16(strDbPath.GetString(), &m_pDb);

    if (nResult != SQLITE_OK) {
        CStringA strMessageA;

        if (m_pDb != NULL) {
            strMessageA = sqlite3_errmsg(m_pDb);

            strError.Format(
                _T("SQLite DB 열기 실패. DBPath=%s, Error=%s"),
                strDbPath.GetString(),
                CString(CA2T(strMessageA.GetString(), CP_UTF8)).GetString()
            );

            sqlite3_close(m_pDb);
            m_pDb = NULL;
        } else {
            strError.Format(_T("SQLite DB 열기 실패. DBPath=%s"), strDbPath.GetString());
        }

        return FALSE;
    }

    sqlite3_busy_timeout(m_pDb, SQL_CONTEXT_BUSY_TIMEOUT_MS);

    if (Execute(_T("PRAGMA foreign_keys = ON;"), strError) == FALSE) {
        Close();
        return FALSE;
    }

    return TRUE;
}

void SqlContext::Close() {
    if (m_pDb != NULL) {
        sqlite3_close(m_pDb);
        m_pDb = NULL;
    }
}

BOOL SqlContext::IsOpened() const {
    return m_pDb != NULL;
}

sqlite3* SqlContext::GetDb() const {
    return m_pDb;
}

BOOL SqlContext::Execute(const CString& strSql, CString& strError) {
    CStringA strSqlA;
    char* pszErrorMessage;
    int nResult;

    if (m_pDb == NULL) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    pszErrorMessage = NULL;
    strSqlA = CT2A(strSql.GetString(), CP_UTF8);

    nResult = sqlite3_exec(
        m_pDb,
        strSqlA.GetString(),
        NULL,
        NULL,
        &pszErrorMessage
    );

    if (nResult != SQLITE_OK) {
        if (pszErrorMessage != NULL) {
            strError.Format(
                _T("SQLite SQL 실행 실패. Error=%s"),
                CString(CA2T(pszErrorMessage, CP_UTF8)).GetString()
            );

            sqlite3_free(pszErrorMessage);
        } else {
            strError.Format(_T("SQLite SQL 실행 실패. ErrorCode=%d"), nResult);
        }

        return FALSE;
    }

    return TRUE;
}

BOOL SqlContext::BeginTransaction(CString& strError) {
    return Execute(_T("BEGIN TRANSACTION;"), strError);
}

BOOL SqlContext::Commit(CString& strError) {
    return Execute(_T("COMMIT;"), strError);
}

BOOL SqlContext::Rollback(CString& strError) {
    return Execute(_T("ROLLBACK;"), strError);
}

CString SqlContext::GetDefaultDbPath() const {
    CString strExeDirectory;
    CString strDbPath;

    strExeDirectory = GetExeDirectory();

    strDbPath.Format(
        _T("%s\\%s\\%s"),
        strExeDirectory.GetString(),
        SQL_CONTEXT_DB_FOLDER_NAME,
        SQL_CONTEXT_DB_FILE_NAME
    );

    return strDbPath;
}

CString SqlContext::GetExeDirectory() const {
    TCHAR szModulePath[MAX_PATH];
    CString strModulePath;
    int nFindPos;

    ZeroMemory(szModulePath, sizeof(szModulePath));

    GetModuleFileName(NULL, szModulePath, MAX_PATH);

    strModulePath = szModulePath;
    nFindPos = strModulePath.ReverseFind(_T('\\'));

    if (nFindPos >= 0) {
        return strModulePath.Left(nFindPos);
    }

    return strModulePath;
}

CString SqlContext::GetDirectoryPathFromFilePath(const CString& strFilePath) const {
    int nFindPos;

    nFindPos = strFilePath.ReverseFind(_T('\\'));

    if (nFindPos < 0) {
        return _T("");
    }

    return strFilePath.Left(nFindPos);
}

BOOL SqlContext::EnsureDirectoryExists(const CString& strDirectory, CString& strError) const {
    DWORD dwAttribute;
    BOOL bResult;
    DWORD dwError;

    dwAttribute = GetFileAttributes(strDirectory.GetString());

    if (dwAttribute != INVALID_FILE_ATTRIBUTES) {
        if ((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
            return TRUE;
        }

        strError.Format(
            _T("동일한 이름의 파일이 존재하여 폴더를 만들 수 없습니다. Path=%s"),
            strDirectory.GetString()
        );

        return FALSE;
    }

    bResult = CreateDirectory(strDirectory.GetString(), NULL);

    if (bResult == TRUE) {
        return TRUE;
    }

    dwError = GetLastError();

    if (dwError == ERROR_ALREADY_EXISTS) {
        return TRUE;
    }

    strError.Format(
        _T("폴더 생성 실패. Path=%s, ErrorCode=%lu"),
        strDirectory.GetString(),
        dwError
    );

    return FALSE;
}