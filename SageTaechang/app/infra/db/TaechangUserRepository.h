#pragma once

#include "pch.h"
#include "SqlContext.h"
#include "TaechangUserDto.h"

class TaechangUserRepository {
public:
    TaechangUserRepository(SqlContext* pSqlContext);
    ~TaechangUserRepository();

public:
    BOOL Insert(const TaechangUserDto& dto, int& nNewUserId, CString& strError);
    BOOL SelectByLoginId(const CString& strLoginId, TaechangUserDto& dto, BOOL& bFound, CString& strError);
    BOOL SelectAll(CArray<TaechangUserDto, TaechangUserDto&>& arrUsers, CString& strError);
    BOOL Delete(int nUserId, CString& strError);
    BOOL UpdatePassword(int nUserId, const CString& strPwHash, CString& strError);
    BOOL UpdateRole(int nUserId, int nRole, CString& strError);
    BOOL ExistsByLoginId(const CString& strLoginId, BOOL& bExists, CString& strError);

private:
    BOOL FillDto(sqlite3_stmt* pStatement, TaechangUserDto& dto);

private:
    SqlContext* m_pSqlContext;
};
