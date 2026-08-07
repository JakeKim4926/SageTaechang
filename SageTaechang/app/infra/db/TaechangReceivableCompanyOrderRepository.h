#pragma once

#include "pch.h"
#include "app/infra/db/SqlContext.h"
#include "app/core/receivable/TaechangReceivableCompanyOrderDto.h"

class TaechangReceivableCompanyOrderRepository {
public:
    TaechangReceivableCompanyOrderRepository(SqlContext* pSqlContext);
    ~TaechangReceivableCompanyOrderRepository();

public:
    BOOL Insert(const TaechangReceivableCompanyOrderDto& dto, int& nNewOrderId, CString& strError);
    BOOL SelectAll(CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&>& arrOrder, CString& strError);
    BOOL SelectByCompanyName(const CString& strCompanyName, TaechangReceivableCompanyOrderDto& dto, BOOL& bFound, CString& strError);
    BOOL Update(const TaechangReceivableCompanyOrderDto& dto, int& nAffectedCount, CString& strError);
    BOOL SwapSortOrder(
        const TaechangReceivableCompanyOrderDto& dtoFirst,
        const TaechangReceivableCompanyOrderDto& dtoSecond,
        CString& strError);
    BOOL DeleteByOrderId(int nOrderId, int& nAffectedCount, CString& strError);
    BOOL DeleteByCompanyName(const CString& strCompanyName, int& nAffectedCount, CString& strError);
    BOOL ExistsByCompanyName(const CString& strCompanyName, int nExceptOrderId, BOOL& bExists, CString& strError);
    BOOL ExistsBySortOrder(int nSortOrder, int nExceptOrderId, BOOL& bExists, CString& strError);

private:
    BOOL FillDto(sqlite3_stmt* pStatement, TaechangReceivableCompanyOrderDto& dto);

private:
    SqlContext* m_pSqlContext;
};
