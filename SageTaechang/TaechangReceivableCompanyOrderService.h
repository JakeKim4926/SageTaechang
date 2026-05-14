#pragma once

#include "pch.h"
#include "TaechangReceivableCompanyOrderDto.h"
#include "TaechangReceivableCompanyOrderRepository.h"

class TaechangReceivableCompanyOrderService {
public:
    TaechangReceivableCompanyOrderService(TaechangReceivableCompanyOrderRepository* pRepository);
    ~TaechangReceivableCompanyOrderService();

public:
    BOOL AddCompanyOrder(const TaechangReceivableCompanyOrderDto& dto, int& nNewOrderId, CString& strError);
    BOOL LoadAllCompanyOrders(CArray<TaechangReceivableCompanyOrderDto, TaechangReceivableCompanyOrderDto&>& arrOrder, CString& strError);
    BOOL LoadCompanyOrder(const CString& strCompanyName, TaechangReceivableCompanyOrderDto& dto, BOOL& bFound, CString& strError);
    BOOL ChangeCompanyOrder(const TaechangReceivableCompanyOrderDto& dto, CString& strError);
    BOOL RemoveCompanyOrder(int nOrderId, CString& strError);
    BOOL RemoveCompanyOrderByName(const CString& strCompanyName, CString& strError);

private:
    BOOL ValidateForSave(const TaechangReceivableCompanyOrderDto& dto, CString& strError);
    BOOL ValidateCompanyName(const CString& strCompanyName, CString& strError);
    BOOL ValidateSortOrder(int nSortOrder, CString& strError);

private:
    TaechangReceivableCompanyOrderRepository* m_pRepository;
};
