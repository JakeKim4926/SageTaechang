#pragma once

#include "app/ui/drawing/SageUiResources.h"

struct SageSummaryBarItem
{
	SageSummaryBarItem() {
		bHighlight = FALSE;
	}

	CString strLabel;
	CString strValue;
	CString strUnit;
	BOOL bHighlight;
};

class CSageSummaryBar : public CStatic
{
public:
	void SetItems(const std::vector<SageSummaryBarItem>& arrItems);
	BOOL HasItems() const;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	int MeasureTextWidth(CDC* pDC, const CString& strText, SageFontRole nRole) const;
	int MeasureItemWidth(CDC* pDC, const SageSummaryBarItem& item) const;
	int DrawTextSegment(CDC* pDC, int nLeft, const CRect& rectItem, const CString& strText, SageFontRole nRole, COLORREF color);
	void DrawDivider(CDC* pDC, int nLeft, const CRect& rectClient);

private:
	std::vector<SageSummaryBarItem> m_arrItems;
};
