#pragma once

#include "pch.h"
#include "SqlContext.h"
#include "EstimateDto.h"

class ReportTypeRepository {
public:
    ReportTypeRepository(SqlContext* pSqlContext);
    ~ReportTypeRepository();

public:
    BOOL SelectAll(CArray<ReportTypeDto, ReportTypeDto&>& arrReportType, CString& strError);
    BOOL SelectByCode(const CString& strReportCode, ReportTypeDto& dto, BOOL& bFound, CString& strError);
    BOOL SelectById(int nReportTypeId, ReportTypeDto& dto, BOOL& bFound, CString& strError);

private:
    SqlContext* m_pSqlContext;
};