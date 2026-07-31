#pragma once

#include "pch.h"
#include "app/core/auth/TaechangUserDto.h"

#define taechangAuth TaechangAuthSession::GetInstance()

class TaechangAuthSession {
public:
    static TaechangAuthSession& GetInstance();

private:
    TaechangAuthSession();
    ~TaechangAuthSession();

private:
    TaechangAuthSession(const TaechangAuthSession&);
    TaechangAuthSession& operator=(const TaechangAuthSession&);

public:
    BOOL IsLoggedIn() const;
    BOOL IsAdmin() const;
    const TaechangUserDto& GetCurrentUser() const;

    void SetLogin(const TaechangUserDto& dto);
    void Logout();

private:
    BOOL m_bLoggedIn;
    TaechangUserDto m_currentUser;
};
