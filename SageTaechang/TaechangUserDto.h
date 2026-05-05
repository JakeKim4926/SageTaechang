#pragma once

#include "pch.h"

enum UserRole {
    USER_ROLE_USER = 0,
    USER_ROLE_ADMIN = 1
};

struct TaechangUserDto {
    int nUserId;
    CString strLoginId;
    CString strPwHash;
    int nRole;

    TaechangUserDto() {
        nUserId = 0;
        nRole = USER_ROLE_USER;
    }
};
