#include "pch.h"
#include "app/infra/db/SageDBMgr.h"

SageDBMgr& SageDBMgr::GetInstance() {
    static SageDBMgr instance;
    return instance;
}

SageDBMgr::SageDBMgr() {
    m_bInitialized = FALSE;

    m_pSagePriceRepository = NULL;
    m_pSagePriceService = NULL;

    m_pReceivableCompanyOrderRepository = NULL;
    m_pReceivableCompanyOrderService = NULL;

    m_pUserRepository = NULL;
    m_pUserService = NULL;
}

SageDBMgr::~SageDBMgr() {
    Finalize();
}

BOOL SageDBMgr::Initialize(CString& strError) {
    SqlInitializer sqlInitializer(&m_sqlContext);

    if (m_bInitialized == TRUE) {
        return TRUE;
    }

    if (m_sqlContext.OpenDefault(strError) == FALSE) {
        return FALSE;
    }

    if (sqlInitializer.Initialize(strError) == FALSE) {
        m_sqlContext.Close();
        return FALSE;
    }

    if (CreateRepositories(strError) == FALSE) {
        DeleteRepositories();
        m_sqlContext.Close();
        return FALSE;
    }

    if (CreateServices(strError) == FALSE) {
        DeleteServices();
        DeleteRepositories();
        m_sqlContext.Close();
        return FALSE;
    }

    m_bInitialized = TRUE;

    return TRUE;
}

void SageDBMgr::Finalize() {
    DeleteServices();
    DeleteRepositories();

    m_sqlContext.Close();

    m_bInitialized = FALSE;
}

BOOL SageDBMgr::IsInitialized() const {
    return m_bInitialized;
}

SqlContext* SageDBMgr::GetSqlContext() {
    return &m_sqlContext;
}

SagePriceRepository* SageDBMgr::GetSagePriceRepository() {
    return m_pSagePriceRepository;
}

SagePriceService* SageDBMgr::GetSagePriceService() {
    return m_pSagePriceService;
}

SageReceivableCompanyOrderRepository* SageDBMgr::GetReceivableCompanyOrderRepository() {
    return m_pReceivableCompanyOrderRepository;
}

SageReceivableCompanyOrderService* SageDBMgr::GetReceivableCompanyOrderService() {
    return m_pReceivableCompanyOrderService;
}

SageUserRepository* SageDBMgr::GetUserRepository() {
    return m_pUserRepository;
}

SageUserService* SageDBMgr::GetUserService() {
    return m_pUserService;
}

BOOL SageDBMgr::CreateRepositories(CString& strError) {
    if (m_sqlContext.IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    DeleteRepositories();

    m_pSagePriceRepository = new SagePriceRepository(&m_sqlContext);

    if (m_pSagePriceRepository == NULL) {
        strError = _T("SagePriceRepository 생성 실패");
        return FALSE;
    }

    m_pReceivableCompanyOrderRepository = new SageReceivableCompanyOrderRepository(&m_sqlContext);

    if (m_pReceivableCompanyOrderRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository 생성 실패");
        return FALSE;
    }

    m_pUserRepository = new SageUserRepository(&m_sqlContext);

    if (m_pUserRepository == NULL) {
        strError = _T("SageUserRepository 생성 실패");
        return FALSE;
    }

    return TRUE;
}

BOOL SageDBMgr::CreateServices(CString& strError) {
    if (m_pSagePriceRepository == NULL) {
        strError = _T("SagePriceRepository가 생성되지 않았습니다.");
        return FALSE;
    }

    if (m_pReceivableCompanyOrderRepository == NULL) {
        strError = _T("SageReceivableCompanyOrderRepository가 생성되지 않았습니다.");
        return FALSE;
    }

    DeleteServices();

    m_pSagePriceService = new SagePriceService(m_pSagePriceRepository);

    if (m_pSagePriceService == NULL) {
        strError = _T("SagePriceService 생성 실패");
        return FALSE;
    }

    m_pReceivableCompanyOrderService = new SageReceivableCompanyOrderService(m_pReceivableCompanyOrderRepository);

    if (m_pReceivableCompanyOrderService == NULL) {
        strError = _T("SageReceivableCompanyOrderService 생성 실패");
        return FALSE;
    }

    m_pUserService = new SageUserService(m_pUserRepository);

    if (m_pUserService == NULL) {
        strError = _T("SageUserService 생성 실패");
        return FALSE;
    }

    return TRUE;
}

void SageDBMgr::DeleteServices() {
    if (m_pSagePriceService != NULL) {
        delete m_pSagePriceService;
        m_pSagePriceService = NULL;
    }

    if (m_pReceivableCompanyOrderService != NULL) {
        delete m_pReceivableCompanyOrderService;
        m_pReceivableCompanyOrderService = NULL;
    }

    if (m_pUserService != NULL) {
        delete m_pUserService;
        m_pUserService = NULL;
    }
}

void SageDBMgr::DeleteRepositories() {
    if (m_pSagePriceRepository != NULL) {
        delete m_pSagePriceRepository;
        m_pSagePriceRepository = NULL;
    }

    if (m_pReceivableCompanyOrderRepository != NULL) {
        delete m_pReceivableCompanyOrderRepository;
        m_pReceivableCompanyOrderRepository = NULL;
    }

    if (m_pUserRepository != NULL) {
        delete m_pUserRepository;
        m_pUserRepository = NULL;
    }
}
