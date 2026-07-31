#pragma once

#include "pch.h"

struct TaechangReceivableCompanyOrderDto {
    int nOrderId;
    CString strCompanyName;
    int nSortOrder;

    TaechangReceivableCompanyOrderDto() {
        nOrderId = 0;
        nSortOrder = 0;
    }
};
