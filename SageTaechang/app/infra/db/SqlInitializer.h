#pragma once

#include "pch.h"
#include "app/infra/db/SqlContext.h"

class SQLInitializer {
public:
    SQLInitializer(SqlContext* pSqlContext);
    ~SQLInitializer();

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
