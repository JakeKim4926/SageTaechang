#pragma once

#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageEdit.h"
#include "app/ui/drawing/SageInlineError.h"
#include "app/ui/drawing/SageLabel.h"
#include "app/ui/drawing/SageListBox.h"
#include "app/ui/dialogs/SageFramelessDialog.h"

#include "pch.h"

class TaechangCalcCompanyPickerDlg : public SageFramelessDialog {
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
    afx_msg void OnSearchChanged();
    afx_msg void OnListDblClick();

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void LayoutControls();
    void ApplyFont();
    void ApplySearchEditTextRect();
    void FilterList(const CString& strKeyword);

private:
    CStringArray m_arrAllNames;
    CString m_strInitialName;
    CString m_strSelectedName;

    CSageEdit m_wndSearchEdit;
    CSageListBox m_wndNameList;
    CSageInlineError m_wndError;
    CSageLabel m_wndMatchCount;
    CSageButton m_wndOkBtn;
    CSageButton m_wndCancelBtn;

    CFont m_font;
    CBrush m_brushBackground;
    CBrush m_brushPanel;
};
