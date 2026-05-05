#pragma once

#include <afxwin.h>

#include "SqlContext.h"
#include "SQLInitializer.h"

#include "TaechangPriceRepository.h"
#include "TaechangPriceService.h"
#include "TaechangUserRepository.h"
#include "TaechangUserService.h"

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

    TaechangPriceRepository* GetTaechangPriceRepository();
    TaechangPriceService* GetTaechangPriceService();

    TaechangUserRepository* GetUserRepository();
    TaechangUserService* GetUserService();

private:
    BOOL CreateRepositories(CString& strError);
    BOOL CreateServices(CString& strError);

    void DeleteServices();
    void DeleteRepositories();

private:
    BOOL m_bInitialized;

    SqlContext m_sqlContext;

    TaechangPriceRepository* m_pTaechangPriceRepository;
    TaechangPriceService* m_pTaechangPriceService;

    TaechangUserRepository* m_pUserRepository;
    TaechangUserService* m_pUserService;
};