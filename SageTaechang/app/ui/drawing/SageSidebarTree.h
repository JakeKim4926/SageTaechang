#pragma once

class CSageSidebarTree : public CTreeCtrl
{
	DECLARE_MESSAGE_MAP()

protected:
	afx_msg void OnNMCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

private:
	void DrawTreeItem(HTREEITEM hItem, BOOL bSelected, NMTVCUSTOMDRAW* pCD);
};
