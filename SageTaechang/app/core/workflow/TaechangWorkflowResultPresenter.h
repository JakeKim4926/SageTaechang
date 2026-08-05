#pragma once

struct TaechangResultRow
{
    TaechangResultRow();

    int m_nSourceRowIndex;
    CString m_strField;
    CString m_strValue;
    CString m_strStatus;
    CString m_strReason;
    CString m_strCompanyName;
    CString m_strDepartment;
    CString m_strOrderDate;
    CString m_strDeliveryDate;
    CString m_strDeliveryTime;
    CString m_strManager;
    CString m_strIssueDate;
    CString m_strItemName;
    CString m_strProductType;
    CString m_strCompanyCopies;
    CString m_strCorporationCopies;
    CString m_strTotalCopies;
    CString m_strIssueType;
    CString m_strTotalAmount;
    CString m_strDepositAmount;
    CString m_strReceivableAmount;
    CString m_strBankName;
    CString m_strNote;

    __int64 m_nTotalAmount;
    __int64 m_nDepositAmount;
    __int64 m_nReceivableAmount;
};

class TaechangWorkflowResultPresenter
{
public:
    BOOL BuildRows(
        int nWorkflowType,
        int nTaskType,
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows);

private:
    void AddRow(
        std::vector<TaechangResultRow>& outRows,
        const CString& strField,
        const CString& strValue,
        const CString& strStatus,
        const CString& strReason) const;

    void AddSummaryRows(
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows) const;


    void AddReceivablesResultRows(
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows) const;

    void AddDeliveryInputRows(
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows) const;

    void AddEstimateInputRows(
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows) const;
};
