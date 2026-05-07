#include "pch.h"
#include "TaechangPriceService.h"
#include "TaechangDefine.h"

TaechangPriceService::TaechangPriceService(TaechangPriceRepository* pRepository) {
    m_pRepository = pRepository;
}

TaechangPriceService::~TaechangPriceService() {}

BOOL TaechangPriceService::AddPrice(const TaechangPriceDto& dto, int& nNewPriceId, CString& strError) {
    BOOL bExists;

    nNewPriceId = 0;
    bExists = FALSE;

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateForInsert(dto, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->ExistsOverlap(
        dto.strCompanyName,
        dto.nReportType,
        dto.nMinCopies,
        dto.bHasMaxCopies,
        dto.nMaxCopies,
        0,
        bExists,
        strError
    ) == FALSE) {
        return FALSE;
    }

    if (bExists == TRUE) {
        strError = _T("이미 겹치는 부수 구간이 존재합니다.");
        return FALSE;
    }

    return m_pRepository->Insert(dto, nNewPriceId, strError);
}

BOOL TaechangPriceService::LoadByCompany(
    const CString& strCompanyName,
    CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
    CString& strError
) {
    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    return m_pRepository->SelectByCompany(
        strCompanyName,
        REPORT_TYPE_AUDIT_REPORT,
        arrPrice,
        strError
    );
}

BOOL TaechangPriceService::LoadByCompanyAndCopies(
    const CString& strCompanyName,
    int nCopies,
    TaechangPriceDto& dto,
    BOOL& bFound,
    CString& strError
) {
    bFound = FALSE;
    dto = TaechangPriceDto();

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (ValidateCopies(nCopies, strError) == FALSE) {
        return FALSE;
    }

    return m_pRepository->SelectByCompanyAndCopies(
        strCompanyName,
        REPORT_TYPE_AUDIT_REPORT,
        nCopies,
        dto,
        bFound,
        strError
    );
}

BOOL TaechangPriceService::LoadByPrice(
    int nPrice,
    CArray<TaechangPriceDto, TaechangPriceDto&>& arrPrice,
    CString& strError
) {
    arrPrice.RemoveAll();

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (nPrice < 0) {
        strError = _T("조회할 가격은 0원 이상이어야 합니다.");
        return FALSE;
    }

    return m_pRepository->SelectByPrice(
        nPrice,
        REPORT_TYPE_AUDIT_REPORT,
        arrPrice,
        strError
    );
}

BOOL TaechangPriceService::ChangePriceByCompany(
    const CString& strCompanyName,
    int nPrintPrice,
    int nCoverPrice,
    int& nAffectedCount,
    CString& strError
) {
    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (ValidatePriceValue(nPrintPrice, nCoverPrice, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->UpdatePriceByCompany(
        strCompanyName,
        REPORT_TYPE_AUDIT_REPORT,
        nPrintPrice,
        nCoverPrice,
        nAffectedCount,
        strError
    ) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("변경된 가격 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::ChangePriceByCompanyAndCopies(
    const CString& strCompanyName,
    int nCopies,
    int nPrintPrice,
    int nCoverPrice,
    int& nAffectedCount,
    CString& strError
) {
    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (ValidateCopies(nCopies, strError) == FALSE) {
        return FALSE;
    }

    if (ValidatePriceValue(nPrintPrice, nCoverPrice, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->UpdatePriceByCompanyAndCopies(
        strCompanyName,
        REPORT_TYPE_AUDIT_REPORT,
        nCopies,
        nPrintPrice,
        nCoverPrice,
        nAffectedCount,
        strError
    ) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("해당 법인과 부수에 맞는 가격 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::RenameCompany(
    const CString& strOldCompanyName,
    const CString& strNewCompanyName,
    int& nAffectedCount,
    CString& strError
) {
    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository媛 NULL?낅땲??");
        return FALSE;
    }

    if (ValidateCompanyName(strOldCompanyName, strError) == FALSE ||
        ValidateCompanyName(strNewCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->UpdateCompanyName(
        strOldCompanyName,
        strNewCompanyName,
        REPORT_TYPE_AUDIT_REPORT,
        nAffectedCount,
        strError
    ) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("변경할 법인 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::ChangeCoverPriceByCompany(
    const CString& strCompanyName,
    int nCoverPrice,
    int& nAffectedCount,
    CString& strError
) {
    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository媛 NULL?낅땲??");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (ValidatePriceValue(0, nCoverPrice, strError) == FALSE) {
        return FALSE;
    }
    if (nCoverPrice > TAECHANG_PRICE_AMOUNT_MAX) {
        strError = TAECHANG_UI_PRICE_AMOUNT_OUT_OF_RANGE;
        return FALSE;
    }

    if (m_pRepository->UpdateCoverPriceByCompany(
        strCompanyName,
        REPORT_TYPE_AUDIT_REPORT,
        nCoverPrice,
        nAffectedCount,
        strError
    ) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("변경할 표지 단가 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::RemovePrice(int nPriceId, CString& strError) {
    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (nPriceId <= 0) {
        strError = _T("유효하지 않은 가격 ID입니다.");
        return FALSE;
    }

    return m_pRepository->DeleteByPriceId(nPriceId, strError);
}

BOOL TaechangPriceService::LoadAllCompanyNames(CStringArray& arrNames, CString& strError) {
    arrNames.RemoveAll();

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    return m_pRepository->SelectAllCompanyNames(REPORT_TYPE_AUDIT_REPORT, arrNames, strError);
}

BOOL TaechangPriceService::ModifyPriceById(const TaechangPriceDto& dto, CString& strError) {
    BOOL bExists;

    bExists = FALSE;

    if (m_pRepository == NULL) {
        strError = _T("TaechangPriceRepository가 NULL입니다.");
        return FALSE;
    }

    if (dto.nPriceId <= 0) {
        strError = _T("유효하지 않은 가격 ID입니다.");
        return FALSE;
    }

    if (ValidateForInsert(dto, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->ExistsOverlap(
        dto.strCompanyName,
        dto.nReportType,
        dto.nMinCopies,
        dto.bHasMaxCopies,
        dto.nMaxCopies,
        dto.nPriceId,
        bExists,
        strError
    ) == FALSE) {
        return FALSE;
    }

    if (bExists == TRUE) {
        strError = _T("이미 겹치는 부수 구간이 존재합니다.");
        return FALSE;
    }

    return m_pRepository->UpdateByPriceId(
        dto.nPriceId,
        dto.nMinCopies,
        dto.bHasMaxCopies,
        dto.nMaxCopies,
        dto.nPrintPrice,
        dto.nCoverPrice,
        strError
    );
}

BOOL TaechangPriceService::ValidateForInsert(const TaechangPriceDto& dto, CString& strError) {
    if (ValidateCompanyName(dto.strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (dto.nReportType <= REPORT_TYPE_NONE) {
        strError = _T("보고서 종류가 올바르지 않습니다.");
        return FALSE;
    }

    if (dto.nMinCopies < 1) {
        strError = _T("최소 부수는 1 이상이어야 합니다.");
        return FALSE;
    }

    if (dto.bHasMaxCopies == TRUE && dto.nMaxCopies < dto.nMinCopies) {
        strError = _T("최대 부수는 최소 부수보다 작을 수 없습니다.");
        return FALSE;
    }

    if (ValidatePriceValue(dto.nPrintPrice, dto.nCoverPrice, strError) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::ValidateCompanyName(const CString& strCompanyName, CString& strError) {
    CString strTrimCompanyName;

    strTrimCompanyName = strCompanyName;
    strTrimCompanyName.Trim();

    if (strTrimCompanyName.IsEmpty() == TRUE) {
        strError = _T("법인명을 입력해야 합니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::ValidateCopies(int nCopies, CString& strError) {
    if (nCopies < 1) {
        strError = _T("부수는 1 이상이어야 합니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangPriceService::ValidatePriceValue(int nPrintPrice, int nCoverPrice, CString& strError) {
    if (nPrintPrice < 0) {
        strError = _T("인쇄 가격은 0원 이상이어야 합니다.");
        return FALSE;
    }

    if (nCoverPrice < 0) {
        strError = _T("표지 가격은 0원 이상이어야 합니다.");
        return FALSE;
    }

    return TRUE;
}
