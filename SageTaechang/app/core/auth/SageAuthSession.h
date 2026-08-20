#pragma once

#include "pch.h"
#include "app/core/auth/SageUserDto.h"

#define sageAuth SageAuthSession::GetInstance()

class SageAuthSession {
public:
    static SageAuthSession& GetInstance();

private:
    SageAuthSession();
    ~SageAuthSession();

private:
    SageAuthSession(const SageAuthSession&);
    SageAuthSession& operator=(const SageAuthSession&);

public:
    BOOL IsLoggedIn() const;
    BOOL IsAdmin() const;
    const SageUserDto& GetCurrentUser() const;

    void SetLogin(const SageUserDto& dto);
    void Logout();

private:
    BOOL m_bLoggedIn;
    SageUserDto m_currentUser;
};
