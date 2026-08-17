#pragma once

#include "app/ui/dialogs/SageFramelessDialog.h"
#include "app/ui/drawing/SageButton.h"
#include "app/ui/drawing/SageMessageBody.h"

class SageMessageBoxDlg : public SageFramelessDialog
{
	DECLARE_MESSAGE_MAP()

public:
	SageMessageBoxDlg(const CString& strMessage, UINT nType, CWnd* pParent);

	virtual INT_PTR DoModal();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnYesClicked();
	afx_msg void OnNoClicked();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

private:
	void CreateControls();
	int LayoutControls();
	void ApplyStyle();
	BOOL IsConfirm() const;
	BOOL IsDefaultReject() const;
	SageMessageIcon GetMessageIcon() const;
	LPCWSTR GetCaptionTitle() const;

private:
	CString m_strMessage;
	UINT m_nType;
	CSageMessageBody m_wndBody;
	CSageButton m_wndAcceptBtn;
	CSageButton m_wndRejectBtn;
	CBrush m_brushBackground;
};

int ShowSageMessageBox(LPCWSTR pszText, UINT nType = MB_OK, CWnd* pParent = NULL);
