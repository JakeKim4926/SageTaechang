#include "pch.h"
#include "app/infra/db/SageDBMgr.h"

SageDBMgr& SageDBMgr::GetInstance() {
    static SageDBMgr instance;
    return instance;
}

SageDBMgr::SageDBMgr() {
    m_bInitialized = FALSE;

    m_pTaechangPriceRepository = NULL;
    m_pTaechangPriceService = NULL;

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

TaechangPriceRepository* SageDBMgr::GetTaechangPriceRepository() {
    return m_pTaechangPriceRepository;
}

TaechangPriceService* SageDBMgr::GetTaechangPriceService() {
    return m_pTaechangPriceService;
}

TaechangReceivableCompanyOrderRepository* SageDBMgr::GetReceivableCompanyOrderRepository() {
    return m_pReceivableCompanyOrderRepository;
}

TaechangReceivableCompanyOrderService* SageDBMgr::GetReceivableCompanyOrderService() {
    return m_pReceivableCompanyOrderService;
}

TaechangUserRepository* SageDBMgr::GetUserRepository() {
    return m_pUserRepository;
}

TaechangUserService* SageDBMgr::GetUserService() {
    return m_pUserService;
}

BOOL SageDBMgr::CreateRepositories(CString& strError) {
    if (m_sqlContext.IsOpened() == FALSE) {
        strError = _T("SQLite DB가 열려 있지 않습니다.");
        return FALSE;
    }

    DeleteRepositories();

    m_pTaechangPriceRepository = new TaechangPriceRepository(&m_sqlContext);

    if (m_pTaechangPriceRepository == NULL) {
        strError = _T("TaechangPriceRepository 생성 실패");
        return FALSE;
    }

    m_pReceivableCompanyOrderRepository = new TaechangReceivableCompanyOrderRepository(&m_sqlContext);

    if (m_pReceivableCompanyOrderRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository 생성 실패");
        return FALSE;
    }

    m_pUserRepository = new TaechangUserRepository(&m_sqlContext);

    if (m_pUserRepository == NULL) {
        strError = _T("TaechangUserRepository 생성 실패");
        return FALSE;
    }

    return TRUE;
}

BOOL SageDBMgr::CreateServices(CString& strError) {
    if (m_pTaechangPriceRepository == NULL) {
        strError = _T("TaechangPriceRepository가 생성되지 않았습니다.");
        return FALSE;
    }

    if (m_pReceivableCompanyOrderRepository == NULL) {
        strError = _T("TaechangReceivableCompanyOrderRepository가 생성되지 않았습니다.");
        return FALSE;
    }

    DeleteServices();

    m_pTaechangPriceService = new TaechangPriceService(m_pTaechangPriceRepository);

    if (m_pTaechangPriceService == NULL) {
        strError = _T("TaechangPriceService 생성 실패");
        return FALSE;
    }

    m_pReceivableCompanyOrderService = new TaechangReceivableCompanyOrderService(m_pReceivableCompanyOrderRepository);

    if (m_pReceivableCompanyOrderService == NULL) {
        strError = _T("TaechangReceivableCompanyOrderService 생성 실패");
        return FALSE;
    }

    m_pUserService = new TaechangUserService(m_pUserRepository);

    if (m_pUserService == NULL) {
        strError = _T("TaechangUserService 생성 실패");
        return FALSE;
    }

    return TRUE;
}

void SageDBMgr::DeleteServices() {
    if (m_pTaechangPriceService != NULL) {
        delete m_pTaechangPriceService;
        m_pTaechangPriceService = NULL;
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
    if (m_pTaechangPriceRepository != NULL) {
        delete m_pTaechangPriceRepository;
        m_pTaechangPriceRepository = NULL;
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
