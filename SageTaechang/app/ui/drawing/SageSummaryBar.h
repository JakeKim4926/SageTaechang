#pragma once

#include "app/ui/drawing/SageUiResources.h"
#include "app/ui/drawing/SageBadge.h"

constexpr int SAGE_SUMMARY_NO_BADGE_ITEM = -1;

struct SageSummaryBarBadge
{
	SageSummaryBarBadge() {
		clrBackground = CLR_NONE;
		clrBorder = CLR_NONE;
		clrText = CLR_NONE;
	}

	BOOL IsVisible() const {
		return (clrBackground == CLR_NONE) ? FALSE : TRUE;
	}

	COLORREF clrBackground;
	COLORREF clrBorder;
	COLORREF clrText;
};

struct SageSummaryBarItem
{
	SageSummaryBarItem() {
		bHighlight = FALSE;
	}

	CString strLabel;
	CString strValue;
	CString strUnit;
	BOOL bHighlight;
	SageSummaryBarBadge badge;
};

class CSageSummaryBar : public CStatic
{
	DECLARE_MESSAGE_MAP()

public:
	void SetItems(const std::vector<SageSummaryBarItem>& arrItems);
	BOOL HasItems() const;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	int MeasureTextWidth(CDC* pDC, const CString& strText, SageFontRole nRole) const;
	int MeasureItemWidth(CDC* pDC, const SageSummaryBarItem& item) const;
	int DrawTextSegment(CDC* pDC, int nLeft, const CRect& rectItem, const CString& strText, SageFontRole nRole, COLORREF color);
	void DrawDivider(CDC* pDC, int nLeft, const CRect& rectClient);
	CString BuildBadgeText(const SageSummaryBarItem& item) const;
	int FindBadgeItemIndex() const;
	void ApplyBadgeItem();
	void LayoutBadge();

private:
	std::vector<SageSummaryBarItem> m_arrItems;
	CSageBadge m_wndBadge;
};
