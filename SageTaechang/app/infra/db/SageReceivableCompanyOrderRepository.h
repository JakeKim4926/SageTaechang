#pragma once

#include "pch.h"
#include "app/infra/db/SqlContext.h"
#include "app/core/receivable/SageReceivableCompanyOrderDto.h"

class SageReceivableCompanyOrderRepository {
public:
    SageReceivableCompanyOrderRepository(SqlContext* pSqlContext);
    ~SageReceivableCompanyOrderRepository();

public:
    BOOL Insert(const SageReceivableCompanyOrderDto& dto, int& nNewOrderId, CString& strError);
    BOOL SelectAll(CArray<SageReceivableCompanyOrderDto, SageReceivableCompanyOrderDto&>& arrOrder, CString& strError);
    BOOL SelectByCompanyName(const CString& strCompanyName, SageReceivableCompanyOrderDto& dto, BOOL& bFound, CString& strError);
    BOOL Update(const SageReceivableCompanyOrderDto& dto, int& nAffectedCount, CString& strError);
    BOOL SwapSortOrder(
        const SageReceivableCompanyOrderDto& dtoFirst,
        const SageReceivableCompanyOrderDto& dtoSecond,
        CString& strError);
    BOOL DeleteByOrderId(int nOrderId, int& nAffectedCount, CString& strError);
    BOOL DeleteByCompanyName(const CString& strCompanyName, int& nAffectedCount, CString& strError);
    BOOL ExistsByCompanyName(const CString& strCompanyName, int nExceptOrderId, BOOL& bExists, CString& strError);
    BOOL ExistsBySortOrder(int nSortOrder, int nExceptOrderId, BOOL& bExists, CString& strError);

private:
    BOOL FillDto(sqlite3_stmt* pStatement, SageReceivableCompanyOrderDto& dto);

private:
    SqlContext* m_pSqlContext;
};
