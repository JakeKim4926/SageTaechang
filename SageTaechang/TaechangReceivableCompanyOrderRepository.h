#pragma once

#include "pch.h"
#include "SqlContext.h"
#include "TaechangReceivableCompanyOrderDto.h"

class TaechangReceivableCompanyOrderRepository {
public:
    TaechangReceivableCompanyOrderRepository(SqlContext* pSqlContext);
    ~TaechangReceivableCompanyOrderRepository();

public:
    BOOL Insert(const TaechangReceivableCompanyOrderDto& dto, int& nNewOrderId, CString& strError);
    BOOL SelectAll(CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&>& arrOrder, CString& strError);
    BOOL SelectByCompanyName(const CString& strCompanyName, TaechangReceivableCompanyOrderDto& dto, BOOL& bFound, CString& strError);
    BOOL Update(const TaechangReceivableCompanyOrderDto& dto, int& nAffectedCount, CString& strError);
    BOOL DeleteByOrderId(int nOrderId, int& nAffectedCount, CString& strError);
    BOOL DeleteByCompanyName(const CString& strCompanyName, int& nAffectedCount, CString& strError);
    BOOL ExistsByCompanyName(const CString& strCompanyName, int nExceptOrderId, BOOL& bExists, CString& strError);

private:
    BOOL FillDto(sqlite3_stmt* pStatement, TaechangReceivableCompanyOrderDto& dto);

private:
    SqlContext* m_pSqlContext;
};
