#pragma once

struct TaechangResultRow
{
    CString m_strFile;
    CString m_strField;
    CString m_strValue;
    CString m_strStatus;
    CString m_strReason;
    CString m_strCompanyName;
    CString m_strManager;
    CString m_strIssueDate;
    CString m_strItemName;
    CString m_strIssueType;
    CString m_strTotalAmount;
    CString m_strDepositAmount;
    CString m_strReceivableAmount;
    CString m_strBankName;
    CString m_strNote;
};

class TaechangWorkflowResultPresenter
{
public:
    BOOL BuildRows(
        int nWorkflowType,
        int nTaskType,
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows,
        CString& outDetailText);

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

    void AddCompareFileRows(
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows) const;

    void AddReceivablesResultRows(
        const CString& strResponseJson,
        std::vector<TaechangResultRow>& outRows) const;

    BOOL IsCompareWorkflow(int nWorkflowType) const;
};
