#pragma once

#include "pch.h"

class SqlContext {
public:
    SqlContext();
    ~SqlContext();

public:
    BOOL OpenDefault(CString& strError);
    BOOL Open(const CString& strDbPath, CString& strError);
    void Close();

    BOOL IsOpened() const;
    sqlite3* GetDb() const;

public:
    BOOL Execute(const CString& strSql, CString& strError);

    BOOL BeginTransaction(CString& strError);
    BOOL Commit(CString& strError);
    BOOL Rollback(CString& strError);

private:
    CString GetDefaultDbPath() const;
    CString GetExeDirectory() const;

    BOOL EnsureDirectoryExists(const CString& strDirectory, CString& strError) const;
    CString GetDirectoryPathFromFilePath(const CString& strFilePath) const;

private:
    sqlite3* m_pDb;
};