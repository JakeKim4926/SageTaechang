#include "pch.h"
#include "TaechangReceivableCompanyOrderService.h"

TaechangReceivableCompanyOrderService::TaechangReceivableCompanyOrderService(TaechangReceivableCompanyOrderRepository* pRepository) {
    m_pRepository = pRepository;
}

TaechangReceivableCompanyOrderService::~TaechangReceivableCompanyOrderService() {}

BOOL TaechangReceivableCompanyOrderService::AddCompanyOrder(
    const TaechangReceivableCompanyOrderDto& dto,
    int& nNewOrderId,
    CString& strError
) {
    BOOL bExists;

    nNewOrderId = 0;
    bExists = FALSE;

    if (m_pRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateForSave(dto, strError) == FALSE) {
        return FALSE;
    }

    if (m_pRepository->ExistsByCompanyName(dto.strCompanyName, 0, bExists, strError) == FALSE) {
        return FALSE;
    }

    if (bExists == TRUE) {
        strError = _T("이미 등록된 법인명입니다.");
        return FALSE;
    }

    return m_pRepository->Insert(dto, nNewOrderId, strError);
}

BOOL TaechangReceivableCompanyOrderService::LoadAllCompanyOrders(
    CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&>& arrOrder,
    CString& strError
) {
    arrOrder.RemoveAll();

    if (m_pRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    return m_pRepository->SelectAll(arrOrder, strError);
}

BOOL TaechangReceivableCompanyOrderService::LoadCompanyOrder(
    const CString& strCompanyName,
    TaechangReceivableCompanyOrderDto& dto,
    BOOL& bFound,
    CString& strError
) {
    dto = TaechangReceivableCompanyOrderDto();
    bFound = FALSE;

    if (m_pRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 NULL입니다.");
        return FALSE;
    }

    if (ValidateCompanyName(strCompanyName, strError) == FALSE) {
        return FALSE;
    }

    return m_pRepository->SelectByCompanyName(strCompanyName, dto, bFound, strError);
}

BOOL TaechangReceivableCompanyOrderService::ChangeCompanyOrder(
    const TaechangReceivableCompanyOrderDto& dto,
    CString& strError
) {
    BOOL bExists;
    int nAffectedCount;

    bExists = FALSE;
    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 NULL입니다.");
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
        strError = _T("이미 등록된 법인명입니다.");
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

BOOL TaechangReceivableCompanyOrderService::RemoveCompanyOrder(int nOrderId, CString& strError) {
    int nAffectedCount;

    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 NULL입니다.");
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

BOOL TaechangReceivableCompanyOrderService::RemoveCompanyOrderByName(const CString& strCompanyName, CString& strError) {
    int nAffectedCount;

    nAffectedCount = 0;

    if (m_pRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 NULL입니다.");
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

BOOL TaechangReceivableCompanyOrderService::ValidateForSave(
    const TaechangReceivableCompanyOrderDto& dto,
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

BOOL TaechangReceivableCompanyOrderService::ValidateCompanyName(const CString& strCompanyName, CString& strError) {
    CString strTrimCompanyName;

    strTrimCompanyName = strCompanyName;
    strTrimCompanyName.Trim();

    if (strTrimCompanyName.IsEmpty() == TRUE) {
        strError = _T("법인명을 입력해야 합니다.");
        return FALSE;
    }

    return TRUE;
}

BOOL TaechangReceivableCompanyOrderService::ValidateSortOrder(int nSortOrder, CString& strError) {
    if (nSortOrder < 0) {
        strError = _T("정렬 순서는 0 이상이어야 합니다.");
        return FALSE;
    }

    return TRUE;
}
