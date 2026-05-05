#include "pch.h"
#include "ReportTypeRepository.h"
#include "RepositoryHelper.h"

ReportTypeRepository::ReportTypeRepository(SqlContext* pSqlContext) {
	m_pSqlContext = pSqlContext;
}

ReportTypeRepository::~ReportTypeRepository() {}

BOOL ReportTypeRepository::SelectAll(CArray<ReportTypeDto, ReportTypeDto&>& arrReportType, CString& strError) {
	sqlite3* pDb;
	sqlite3_stmt* pStatement;
	CStringA strSqlA;
	int nResult;

	arrReportType.RemoveAll();

	if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
		strError = _T("SQLite DB가 열려 있지 않습니다.");
		return FALSE;
	}

	pDb = m_pSqlContext->GetDb();
	pStatement = NULL;

	strSqlA =
		"SELECT "
		"    report_type_id, "
		"    report_code, "
		"    report_name, "
		"    memo "
		"FROM report_types "
		"ORDER BY report_type_id ASC;";

	nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

	if (nResult != SQLITE_OK) {
		strError = RepositoryHelper::GetLastError(pDb);
		return FALSE;
	}

	while ((nResult = sqlite3_step(pStatement)) == SQLITE_ROW) {
		ReportTypeDto dto;

		dto.nReportTypeId = sqlite3_column_int(pStatement, 0);
		dto.strReportCode = RepositoryHelper::GetColumnText(pStatement, 1);
		dto.strReportName = RepositoryHelper::GetColumnText(pStatement, 2);
		dto.strMemo = RepositoryHelper::GetColumnText(pStatement, 3);

		arrReportType.Add(dto);
	}

	if (nResult != SQLITE_DONE) {
		strError = RepositoryHelper::GetLastError(pDb);
		sqlite3_finalize(pStatement);
		return FALSE;
	}

	sqlite3_finalize(pStatement);

	return TRUE;
}

BOOL ReportTypeRepository::SelectByCode(const CString& strReportCode, ReportTypeDto& dto, BOOL& bFound, CString& strError) {
	sqlite3* pDb;
	sqlite3_stmt* pStatement;
	CStringA strSqlA;
	int nResult;

	bFound = FALSE;
	dto = ReportTypeDto();

	if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
		strError = _T("SQLite DB가 열려 있지 않습니다.");
		return FALSE;
	}

	pDb = m_pSqlContext->GetDb();
	pStatement = NULL;

	strSqlA =
		"SELECT "
		"    report_type_id, "
		"    report_code, "
		"    report_name, "
		"    memo "
		"FROM report_types "
		"WHERE report_code = ?;";

	nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

	if (nResult != SQLITE_OK) {
		strError = RepositoryHelper::GetLastError(pDb);
		return FALSE;
	}

	if (RepositoryHelper::BindText(pStatement, 1, strReportCode, strError) == FALSE) {
		sqlite3_finalize(pStatement);
		return FALSE;
	}

	nResult = sqlite3_step(pStatement);

	if (nResult == SQLITE_ROW) {
		dto.nReportTypeId = sqlite3_column_int(pStatement, 0);
		dto.strReportCode = RepositoryHelper::GetColumnText(pStatement, 1);
		dto.strReportName = RepositoryHelper::GetColumnText(pStatement, 2);
		dto.strMemo = RepositoryHelper::GetColumnText(pStatement, 3);

		bFound = TRUE;
	} else if (nResult != SQLITE_DONE) {
		strError = RepositoryHelper::GetLastError(pDb);
		sqlite3_finalize(pStatement);
		return FALSE;
	}

	sqlite3_finalize(pStatement);

	return TRUE;
}

BOOL ReportTypeRepository::SelectById(int nReportTypeId, ReportTypeDto& dto, BOOL& bFound, CString& strError) {
	sqlite3* pDb;
	sqlite3_stmt* pStatement;
	CStringA strSqlA;
	int nResult;

	bFound = FALSE;
	dto = ReportTypeDto();

	if (m_pSqlContext == NULL || m_pSqlContext->IsOpened() == FALSE) {
		strError = _T("SQLite DB가 열려 있지 않습니다.");
		return FALSE;
	}

	pDb = m_pSqlContext->GetDb();
	pStatement = NULL;

	strSqlA =
		"SELECT "
		"    report_type_id, "
		"    report_code, "
		"    report_name, "
		"    memo "
		"FROM report_types "
		"WHERE report_type_id = ?;";

	nResult = sqlite3_prepare_v2(pDb, strSqlA.GetString(), -1, &pStatement, NULL);

	if (nResult != SQLITE_OK) {
		strError = RepositoryHelper::GetLastError(pDb);
		return FALSE;
	}

	if (RepositoryHelper::BindInt(pStatement, 1, nReportTypeId, strError) == FALSE) {
		sqlite3_finalize(pStatement);
		return FALSE;
	}

	nResult = sqlite3_step(pStatement);

	if (nResult == SQLITE_ROW) {
		dto.nReportTypeId = sqlite3_column_int(pStatement, 0);
		dto.strReportCode = RepositoryHelper::GetColumnText(pStatement, 1);
		dto.strReportName = RepositoryHelper::GetColumnText(pStatement, 2);
		dto.strMemo = RepositoryHelper::GetColumnText(pStatement, 3);

		bFound = TRUE;
	} else if (nResult != SQLITE_DONE) {
		strError = RepositoryHelper::GetLastError(pDb);
		sqlite3_finalize(pStatement);
		return FALSE;
	}

	sqlite3_finalize(pStatement);

	return TRUE;
}