#pragma once

#include "pch.h"

struct SageReceivableCompanyOrderDto {
    int nOrderId;
    CString strCompanyName;
    int nSortOrder;

    SageReceivableCompanyOrderDto() {
        nOrderId = 0;
        nSortOrder = 0;
    }
};
