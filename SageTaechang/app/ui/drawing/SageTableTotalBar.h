#pragma once

#include "app/ui/drawing/SageUiResources.h"
#include "app/core/workflow/SageWorkflowResultTable.h"

enum SageTotalBarCellStyle
{
	SAGE_TOTAL_BAR_LABEL,
	SAGE_TOTAL_BAR_COUNT,
	SAGE_TOTAL_BAR_AMOUNT,
	SAGE_TOTAL_BAR_AMOUNT_HIGHLIGHT
};

struct SageTableTotalBarCell
{
	SageTableTotalBarCell() {
		nLeft = 0;
		nWidth = 0;
		nAlign = SAGE_COLUMN_ALIGN_LEFT;
		nStyle = SAGE_TOTAL_BAR_LABEL;
	}

	CString strText;
	int nLeft;
	int nWidth;
	SageColumnAlign nAlign;
	SageTotalBarCellStyle nStyle;
};

class CSageTableTotalBar : public CStatic
{
public:
	void SetCells(const std::vector<SageTableTotalBarCell>& arrCells);

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

private:
	SageFontRole GetCellFont(SageTotalBarCellStyle nStyle) const;
	COLORREF GetCellColor(SageTotalBarCellStyle nStyle) const;

private:
	std::vector<SageTableTotalBarCell> m_arrCells;
};
