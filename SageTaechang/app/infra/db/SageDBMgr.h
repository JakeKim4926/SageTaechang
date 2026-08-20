#pragma once

#include <afxwin.h>

#include "app/infra/db/SqlContext.h"
#include "app/infra/db/SqlInitializer.h"

#include "app/infra/db/SagePriceRepository.h"
#include "app/core/price/SagePriceService.h"
#include "app/infra/db/SageReceivableCompanyOrderRepository.h"
#include "app/core/receivable/SageReceivableCompanyOrderService.h"
#include "app/infra/db/SageUserRepository.h"
#include "app/core/auth/SageUserService.h"

#define sageDBMgr SageDBMgr::GetInstance()

class SageDBMgr {
public:
    static SageDBMgr& GetInstance();

private:
    SageDBMgr();
    ~SageDBMgr();

private:
    SageDBMgr(const SageDBMgr& rhs);
    SageDBMgr& operator=(const SageDBMgr& rhs);

public:
    BOOL Initialize(CString& strError);
    void Finalize();

    BOOL IsInitialized() const;

public:
    SqlContext* GetSqlContext();

    SagePriceRepository* GetSagePriceRepository();
    SagePriceService* GetSagePriceService();

    SageReceivableCompanyOrderRepository* GetReceivableCompanyOrderRepository();
    SageReceivableCompanyOrderService* GetReceivableCompanyOrderService();

    SageUserRepository* GetUserRepository();
    SageUserService* GetUserService();

private:
    BOOL CreateRepositories(CString& strError);
    BOOL CreateServices(CString& strError);

    void DeleteServices();
    void DeleteRepositories();

private:
    BOOL m_bInitialized;

    SqlContext m_sqlContext;

    SagePriceRepository* m_pSagePriceRepository;
    SagePriceService* m_pSagePriceService;

    SageReceivableCompanyOrderRepository* m_pReceivableCompanyOrderRepository;
    SageReceivableCompanyOrderService* m_pReceivableCompanyOrderService;

    SageUserRepository* m_pUserRepository;
    SageUserService* m_pUserService;
};
