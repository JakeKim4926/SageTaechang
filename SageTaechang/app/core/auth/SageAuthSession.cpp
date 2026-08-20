#include "pch.h"
#include "app/core/auth/SageAuthSession.h"

SageAuthSession& SageAuthSession::GetInstance() {
    static SageAuthSession instance;
    return instance;
}

SageAuthSession::SageAuthSession() {
    m_bLoggedIn = FALSE;
}

SageAuthSession::~SageAuthSession() {}

BOOL SageAuthSession::IsLoggedIn() const {
    return m_bLoggedIn;
}

BOOL SageAuthSession::IsAdmin() const {
    if (m_bLoggedIn == FALSE)
        return FALSE;

    return m_currentUser.nRole == USER_ROLE_ADMIN;
}

const SageUserDto& SageAuthSession::GetCurrentUser() const {
    return m_currentUser;
}

void SageAuthSession::SetLogin(const SageUserDto& dto) {
    m_currentUser = dto;
    m_bLoggedIn = TRUE;
}

void SageAuthSession::Logout() {
    m_currentUser = SageUserDto();
    m_bLoggedIn = FALSE;
}
