#include "pch.h"
#include "app/core/auth/TaechangAuthSession.h"

TaechangAuthSession& TaechangAuthSession::GetInstance() {
    static TaechangAuthSession instance;
    return instance;
}

TaechangAuthSession::TaechangAuthSession() {
    m_bLoggedIn = FALSE;
}

TaechangAuthSession::~TaechangAuthSession() {}

BOOL TaechangAuthSession::IsLoggedIn() const {
    return m_bLoggedIn;
}

BOOL TaechangAuthSession::IsAdmin() const {
    if (m_bLoggedIn == FALSE)
        return FALSE;

    return m_currentUser.nRole == USER_ROLE_ADMIN;
}

const TaechangUserDto& TaechangAuthSession::GetCurrentUser() const {
    return m_currentUser;
}

void TaechangAuthSession::SetLogin(const TaechangUserDto& dto) {
    m_currentUser = dto;
    m_bLoggedIn = TRUE;
}

void TaechangAuthSession::Logout() {
    m_currentUser = TaechangUserDto();
    m_bLoggedIn = FALSE;
}
