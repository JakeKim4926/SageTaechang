#pragma once

#include "pch.h"

class TaechangCalcCompanyPickerDlg : public CDialog {
public:
    TaechangCalcCompanyPickerDlg(const CStringArray& arrNames, const CString& strInitialName, CWnd* pParent = NULL);
    ~TaechangCalcCompanyPickerDlg();

public:
    virtual INT_PTR DoModal();
    CString GetSelectedName() const;

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnSearchChanged();
    afx_msg void OnListDblClick();

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void LayoutControls();
    void ApplyFont();
    void ApplySearchEditTextRect();
    BYTE* BuildDialogTemplate();
    void FilterList(const CString& strKeyword);

private:
    CWnd* m_pDlgParent;
    CStringArray m_arrAllNames;
    CString m_strInitialName;
    CString m_strSelectedName;

    CEdit m_wndSearchEdit;
    CListBox m_wndNameList;
    CButton m_wndOkBtn;
    CButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
