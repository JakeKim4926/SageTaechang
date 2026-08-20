#pragma once

#include "pch.h"
#include "app/core/receivable/SageReceivableCompanyOrderDto.h"
#include "app/infra/db/SageReceivableCompanyOrderRepository.h"

class SageReceivableCompanyOrderService {
public:
    SageReceivableCompanyOrderService(SageReceivableCompanyOrderRepository* pRepository);
    ~SageReceivableCompanyOrderService();

public:
    BOOL AddCompanyOrder(const SageReceivableCompanyOrderDto& dto, int& nNewOrderId, CString& strError);
    BOOL LoadAllCompanyOrders(CArray<SageReceivableCompanyOrderDto, SageReceivableCompanyOrderDto&>& arrOrder, CString& strError);
    BOOL LoadCompanyOrder(const CString& strCompanyName, SageReceivableCompanyOrderDto& dto, BOOL& bFound, CString& strError);
    BOOL ChangeCompanyOrder(const SageReceivableCompanyOrderDto& dto, CString& strError);
    BOOL SwapCompanyOrder(
        const SageReceivableCompanyOrderDto& dtoFirst,
        const SageReceivableCompanyOrderDto& dtoSecond,
        CString& strError);
    BOOL RemoveCompanyOrder(int nOrderId, CString& strError);
    BOOL RemoveCompanyOrderByName(const CString& strCompanyName, CString& strError);

private:
    BOOL ValidateForSave(const SageReceivableCompanyOrderDto& dto, CString& strError);
    BOOL ValidateCompanyName(const CString& strCompanyName, CString& strError);
    BOOL ValidateSortOrder(int nSortOrder, CString& strError);

private:
    SageReceivableCompanyOrderRepository* m_pRepository;
};
