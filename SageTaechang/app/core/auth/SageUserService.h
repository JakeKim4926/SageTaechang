#pragma once

#include "pch.h"
#include "app/core/auth/SageUserDto.h"
#include "app/infra/db/SageUserRepository.h"

class SageUserService {
public:
    SageUserService(SageUserRepository* pRepository);
    ~SageUserService();

public:
    BOOL Login(const CString& strLoginId, const CString& strPassword,
               SageUserDto& outDto, BOOL& bSuccess, CString& strError);

    BOOL AddUser(const SageUserDto& dto, const CString& strPlainPassword,
                 int& nNewUserId, CString& strError);

    BOOL ChangePassword(int nUserId, const CString& strNewPassword, CString& strError);

    BOOL LoadAll(CArray<SageUserDto, SageUserDto&>& arrUsers, CString& strError);

    BOOL RemoveUser(int nUserId, CString& strError);

    static CString HashPassword(const CString& strPassword);

private:
    BOOL ValidateLoginId(const CString& strLoginId, CString& strError);
    BOOL ValidatePassword(const CString& strPassword, CString& strError);

private:
    SageUserRepository* m_pRepository;
};
