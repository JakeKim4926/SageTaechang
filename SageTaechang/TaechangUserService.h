#pragma once

#include "pch.h"
#include "TaechangUserDto.h"
#include "TaechangUserRepository.h"

class TaechangUserService {
public:
    TaechangUserService(TaechangUserRepository* pRepository);
    ~TaechangUserService();

public:
    BOOL Login(const CString& strLoginId, const CString& strPassword,
               TaechangUserDto& outDto, BOOL& bSuccess, CString& strError);

    BOOL AddUser(const TaechangUserDto& dto, const CString& strPlainPassword,
                 int& nNewUserId, CString& strError);

    BOOL ChangePassword(int nUserId, const CString& strNewPassword, CString& strError);

    BOOL LoadAll(CArray<TaechangUserDto, TaechangUserDto&>& arrUsers, CString& strError);

    BOOL RemoveUser(int nUserId, CString& strError);

    static CString HashPassword(const CString& strPassword);

private:
    BOOL ValidateLoginId(const CString& strLoginId, CString& strError);
    BOOL ValidatePassword(const CString& strPassword, CString& strError);

private:
    TaechangUserRepository* m_pRepository;
};
