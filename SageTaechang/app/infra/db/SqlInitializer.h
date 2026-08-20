#pragma once

#include "pch.h"
#include "app/infra/db/SqlContext.h"

class SqlInitializer {
public:
    SqlInitializer(SqlContext* pSqlContext);
    ~SqlInitializer();

public:
    BOOL Initialize(CString& strError);

private:
    BOOL CreateTables(CString& strError);
    BOOL CreateIndexes(CString& strError);

private:
    BOOL CreateSagePriceTable(CString& strError);
    BOOL CreateSageUserTable(CString& strError);
    BOOL CreateSageReceivableCompanyOrderTable(CString& strError);
    BOOL CreateSagePriceCompanyTable(CString& strError);
    BOOL SeedSagePriceCompanies(CString& strError);

private:
    BOOL SeedDefaultAdmin(CString& strError);

private:
    SqlContext* m_pSqlContext;
};
