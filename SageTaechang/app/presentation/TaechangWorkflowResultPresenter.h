#pragma once

struct TaechangResultRow
{
    CString m_strFile;
    CString m_strField;
    CString m_strValue;
    CString m_strStatus;
    CString m_strReason;
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

    BOOL IsCompareWorkflow(int nWorkflowType) const;
};
