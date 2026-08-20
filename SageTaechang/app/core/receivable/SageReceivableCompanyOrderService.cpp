#include "pch.h"
#include "app/core/receivable/SageReceivableCompanyOrderService.h"
#include "SageDefine.h"

SageReceivableCompanyOrderService::SageReceivableCompanyOrderService(SageReceivableCompanyOrderRepository* pRepository) {
    m_pRepository = pRepository;
}

SageReceivableCompanyOrderService::~SageReceivableCompanyOrderService() {}

BOOL SageReceivableCompanyOrderService::AddCompanyOrder(
    const SageReceivableCompanyOrderDto& dto,
    int& nNewOrderId,
    CString& strError
) {
    BOOL bExists;
    BOOL bSortOrderExists;

    nNewOrderId = 0;
    bExists = FALSE;
    bSortOrderExists = FALSE;

    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateForSave(dto, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->ExistsByCompanyName(dto.strCompanyName, 0, bExists, strError) == FALSE) {
        return FALSE;
    }

    if (bExists == TRUE) {
        strError = SAGE_UI_CO_COMPANY_DUPLICATE;
        return FALSE;
    }

    if (m_pRepository->ExistsBySortOrder(dto.nSortOrder, 0, bSortOrderExists, strError) == FALSE) {
        return FALSE;
    }

    if (bSortOrderExists == TRUE) {
        strError = SAGE_UI_CO_ORDER_DUPLICATE;
        return FALSE;
    }

    return m_pRepository->Insert(dto, nNewOrderId, strError);
}

BOOL SageReceivableCompanyOrderService::LoadAllCompanyOrders(
    CArray<SageReceivableCompanyOrderDto, SageReceivableCompanyOrderDto&>& arrOrder,
    CString& strError
) {
    arrOrder.RemoveAll();

    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    return m_pRepository->SelectAll(arrOrder, strError);
}

BOOL SageReceivableCompanyOrderService::LoadCompanyOrder(
    const CString& strCompanyName,
    SageReceivableCompanyOrderDto& dto,
    BOOL& bFound,
    CString& strError
) {
    dto = SageReceivableCompanyOrderDto();
    bFound = FALSE;

    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    return m_pRepository->SelectByCompanyName(strCompanyName, dto, bFound, strError);
}

BOOL SageReceivableCompanyOrderService::ChangeCompanyOrder(
    const SageReceivableCompanyOrderDto& dto,
    CString& strError
) {
    BOOL bExists;
    BOOL bSortOrderExists;
    int nAffectedCount;

    bExists = FALSE;
    bSortOrderExists = FALSE;
    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (dto.nOrderId <= 0) {
        strError = _T("유효하지 않은 정렬 ID입니다.");
        return FALSE;
    }

    if (ValidateForSave(dto, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->ExistsByCompanyName(dto.strCompanyName, dto.nOrderId, bExists, strError) == FALSE) {
        return FALSE;
    }

    if (bExists == TRUE) {
        strError = SAGE_UI_CO_COMPANY_DUPLICATE;
        return FALSE;
    }

    if (m_pRepository->ExistsBySortOrder(dto.nSortOrder, dto.nOrderId, bSortOrderExists, strError) == FALSE) {
        return FALSE;
    }

    if (bSortOrderExists == TRUE) {
        strError = SAGE_UI_CO_ORDER_DUPLICATE;
        return FALSE;
    }

    if (m_pRepository->Update(dto, nAffectedCount, strError) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("변경할 법인 정렬 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL SageReceivableCompanyOrderService::SwapCompanyOrder(
    const SageReceivableCompanyOrderDto& dtoFirst,
    const SageReceivableCompanyOrderDto& dtoSecond,
    CString& strError
) {
    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (dtoFirst.nOrderId <= 0 || dtoSecond.nOrderId <= 0) {
        strError = _T("유효하지 않은 정렬 ID입니다.");
        return FALSE;
    }

    return m_pRepository->SwapSortOrder(dtoFirst, dtoSecond, strError);
}

BOOL SageReceivableCompanyOrderService::RemoveCompanyOrder(int nOrderId, CString& strError) {
    int nAffectedCount;

    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (nOrderId <= 0) {
        strError = _T("유효하지 않은 정렬 ID입니다.");
        return FALSE;
    }

    if (m_pRepository->DeleteByOrderId(nOrderId, nAffectedCount, strError) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("삭제할 법인 정렬 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL SageReceivableCompanyOrderService::RemoveCompanyOrderByName(const CString& strCompanyName, CString& strError) {
    int nAffectedCount;

    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->DeleteByCompanyName(strCompanyName, nAffectedCount, strError) == FALSE) {
        return FALSE;
    }

    if (nAffectedCount <= 0) {
        strError = _T("삭제할 법인 정렬 데이터가 없습니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL SageReceivableCompanyOrderService::ValidateForSave(
    const SageReceivableCompanyOrderDto& dto,
    CString& strError
) {
    if (ValidateCompanyName(dto.strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    if (ValidateSortOrder(dto.nSortOrder, strError) == FALSE) {
        return FALSE;
    }

    return TRUE;
}

BOOL SageReceivableCompanyOrderService::ValidateCompanyName(const CString& strCompanyName, CString& strError) {
    CString strTrimCompanyName;

    strTrimCompanyName = strCompanyName;
    strTrimCompanyName.Trim();

    if (strTrimCompanyName.IsEmpty() == TRUE) {
        strError = _T("법인명을 입력해야 합니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL SageReceivableCompanyOrderService::ValidateSortOrder(int nSortOrder, CString& strError) {
    if (nSortOrder < 0) {
        strError = _T("정렬 순서는 0 이상이어야 합니다.");
        return FALSE;
    }

    return TRUE;
}
