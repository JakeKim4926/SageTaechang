#pragma once

#include "app/ui/drawing/SageDialogCaptionBar.h"

class SageFramelessDialog : public CDialog {
public:
    SageFramelessDialog(CWnd* pParent);

protected:
    BYTE* BuildFramelessTemplate(LPCWSTR pszTitle, int nTemplateCx, int nTemplateCy);
    BOOL CreateCaptionBar(LPCWSTR pszTitle);
    void SizeFramelessClient(int nClientWidth, int nContentBottom);
    int  GetContentTop() const;

protected:
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnCaptionClose();
    DECLARE_MESSAGE_MAP()

protected:
    CWnd* m_pDlgParent;

private:
    CSageDialogCaptionBar m_wndCaption;
};
