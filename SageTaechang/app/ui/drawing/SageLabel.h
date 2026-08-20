#pragma once

#include "app/ui/drawing/SageUiResources.h"

class CSageLabel : public CStatic
{
	DECLARE_DYNAMIC(CSageLabel)
	DECLARE_MESSAGE_MAP()

public:
	CSageLabel();

	void SetTextColorRole(SageTextRole nRole);
	void SetBackgroundRole(SageBackgroundRole nRole);
	void SetFontRole(SageFontRole nRole);

protected:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

private:
	SageTextRole m_nTextRole;
	SageBackgroundRole m_nBackgroundRole;
};
