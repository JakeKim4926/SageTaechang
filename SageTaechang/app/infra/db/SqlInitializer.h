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
    BOOL CreateTaechangPriceTable(CString& strError);
    BOOL CreateTaechangUserTable(CString& strError);
    BOOL CreateTaechangReceivableCompanyOrderTable(CString& strError);

private:
    BOOL SeedDefaultAdmin(CString& strError);

private:
    SqlContext* m_pSqlContext;
};
