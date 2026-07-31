#include "pch.h"
#include "app/ui/drawing/SageLabel.h"

IMPLEMENT_DYNAMIC(CSageLabel, CStatic)

BEGIN_MESSAGE_MAP(CSageLabel, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

CSageLabel::CSageLabel()
	: m_nTextRole(SAGE_TEXT_DEFAULT)
	, m_nBackgroundRole(SAGE_BG_APP) {
}

void CSageLabel::SetTextColorRole(SageTextRole nRole) {
	m_nTextRole = nRole;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageLabel::SetBackgroundRole(SageBackgroundRole nRole) {
	m_nBackgroundRole = nRole;
	if (::IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CSageLabel::SetFontRole(SageFontRole nRole) {
	SetFont(SageUiResources::GetFont(nRole));
}

HBRUSH CSageLabel::CtlColor(CDC* pDC, UINT nCtlColor) {
	pDC->SetTextColor(SageUiResources::GetTextColor(m_nTextRole));
	pDC->SetBkColor(SageUiResources::GetBackgroundColor(m_nBackgroundRole));
	return SageUiResources::GetBrush(m_nBackgroundRole);
}
