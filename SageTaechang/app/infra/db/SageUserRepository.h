#pragma once

#include "pch.h"
#include "app/infra/db/SqlContext.h"
#include "app/core/auth/SageUserDto.h"

class SageUserRepository {
public:
    SageUserRepository(SqlContext* pSqlContext);
    ~SageUserRepository();

public:
    BOOL Insert(const SageUserDto& dto, int& nNewUserId, CString& strError);
    BOOL SelectByLoginId(const CString& strLoginId, SageUserDto& dto, BOOL& bFound, CString& strError);
    BOOL SelectAll(CArray<SageUserDto, SageUserDto&>& arrUsers, CString& strError);
    BOOL Delete(int nUserId, CString& strError);
    BOOL UpdatePassword(int nUserId, const CString& strPwHash, CString& strError);
    BOOL UpdateRole(int nUserId, int nRole, CString& strError);
    BOOL ExistsByLoginId(const CString& strLoginId, BOOL& bExists, CString& strError);

private:
    BOOL FillDto(sqlite3_stmt* pStatement, SageUserDto& dto);

private:
    SqlContext* m_pSqlContext;
};
