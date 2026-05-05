#pragma once

#include "pch.h"
#include "SqlContext.h"
#include "EstimateDto.h"

class CompanyRepository {
public:
    CompanyRepository(SqlContext* pSqlContext);
    ~CompanyRepository();

public:
    BOOL Insert(const CompanyDto& dto, int& nNewCompanyId, CString& strError);
    BOOL Update(const CompanyDto& dto, CString& strError);
    BOOL Delete(int nCompanyId, CString& strError);

    BOOL SelectAll(CArray<CompanyDto, CompanyDto&>& arrCompany, CString& strError);
    BOOL SelectById(int nCompanyId, CompanyDto& dto, BOOL& bFound, CString& strError);
    BOOL ExistsByName(const CString& strCompanyName, int nExceptCompanyId, BOOL& bExists, CString& strError);

private:
    SqlContext* m_pSqlContext;
};