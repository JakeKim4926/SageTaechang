#include "pch.h"
#include "app/ui/drawing/SageUiResources.h"
#include "SageDefine.h"

namespace {

CFont g_fontControl;
CFont g_fontContent;
CFont g_fontContentSemiBold;
CFont g_fontTitle;
CFont g_fontHeader;
CFont g_fontCaption;
CFont g_fontSummary;
CFont g_fontList;
CFont g_fontListSemiBold;
CFont g_fontListBold;
CFont g_fontLogo;

constexpr int SAGE_BG_COUNT = SAGE_BG_LIST_GRID + 1;

const COLORREF g_backgroundColors[SAGE_BG_COUNT] = {
	SAGE_COLOR_APP_BACKGROUND,
	SAGE_COLOR_PANEL,
	SAGE_COLOR_SIDEBAR,
	SAGE_COLOR_STATUS_BG_SUCCESS,
	SAGE_COLOR_STATUS_BG_WARNING,
	SAGE_COLOR_STATUS_BG_ERROR,
	SAGE_COLOR_ACCENT_SURFACE,
	SAGE_COLOR_LIST_HEADER,
	SAGE_COLOR_LIST_GRID
};

CBrush g_brushes[SAGE_BG_COUNT];

constexpr int SAGE_TEXT_COUNT = SAGE_TEXT_MUTED + 1;

const COLORREF g_textColors[SAGE_TEXT_COUNT] = {
	SAGE_COLOR_TEXT,
	SAGE_COLOR_SECONDARY_TEXT,
	SAGE_COLOR_PRIMARY,
	SAGE_COLOR_SUCCESS,
	SAGE_COLOR_ERROR,
	SAGE_COLOR_SIDEBAR_TEXT,
	SAGE_COLOR_SIDEBAR_CATEGORY,
	SAGE_COLOR_TEXT_MUTED
};

void CreateBoldVariant(CFont& fontBase, CFont& fontBold) {
	LOGFONT lf = {};
	if (fontBase.GetLogFont(&lf) == 0)
		return;
	lf.lfWeight = FW_BOLD;
	fontBold.CreateFontIndirect(&lf);
}

}

namespace SageUiResources {

void Create() {
	g_fontControl.CreatePointFont(SAGE_CONTROL_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
	g_fontContent.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
	g_fontTitle.CreatePointFont(SAGE_TITLE_FONT_POINT_SIZE, SAGE_TITLE_FONT_FACE);
	g_fontHeader.CreatePointFont(SAGE_HEADER_FONT_POINT_SIZE, SAGE_TITLE_FONT_FACE);
	g_fontContentSemiBold.CreatePointFont(SAGE_CONTENT_FONT_POINT_SIZE, SAGE_TITLE_FONT_FACE);
	g_fontCaption.CreatePointFont(SAGE_CAPTION_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
	g_fontSummary.CreatePointFont(SAGE_SUMMARY_FONT_POINT_SIZE, SAGE_TITLE_FONT_FACE);
	g_fontList.CreatePointFont(SAGE_LIST_FONT_POINT_SIZE, SAGE_CONTROL_FONT_FACE);
	g_fontListSemiBold.CreatePointFont(SAGE_LIST_FONT_POINT_SIZE, SAGE_TITLE_FONT_FACE);
	g_fontLogo.CreatePointFont(SAGE_TITLE_FONT_POINT_SIZE, SAGE_LOGO_FONT_FACE);
	CreateBoldVariant(g_fontList, g_fontListBold);

	for (int i = 0; i < SAGE_BG_COUNT; ++i)
		g_brushes[i].CreateSolidBrush(g_backgroundColors[i]);
}

void Destroy() {
	g_fontControl.DeleteObject();
	g_fontContent.DeleteObject();
	g_fontContentSemiBold.DeleteObject();
	g_fontTitle.DeleteObject();
	g_fontHeader.DeleteObject();
	g_fontCaption.DeleteObject();
	g_fontSummary.DeleteObject();
	g_fontList.DeleteObject();
	g_fontListSemiBold.DeleteObject();
	g_fontListBold.DeleteObject();
	g_fontLogo.DeleteObject();

	for (int i = 0; i < SAGE_BG_COUNT; ++i)
		g_brushes[i].DeleteObject();
}

CFont* GetFont(SageFontRole nRole) {
	switch (nRole) {
		case SAGE_FONT_CONTENT: return &g_fontContent;
		case SAGE_FONT_CONTENT_SEMIBOLD: return &g_fontContentSemiBold;
		case SAGE_FONT_TITLE:   return &g_fontTitle;
		case SAGE_FONT_HEADER:  return &g_fontHeader;
		case SAGE_FONT_CAPTION:       return &g_fontCaption;
		case SAGE_FONT_SUMMARY:       return &g_fontSummary;
		case SAGE_FONT_LIST:          return &g_fontList;
		case SAGE_FONT_LIST_SEMIBOLD: return &g_fontListSemiBold;
		case SAGE_FONT_LIST_BOLD:     return &g_fontListBold;
		case SAGE_FONT_LOGO:          return &g_fontLogo;
		default:                return &g_fontControl;
	}
}

HBRUSH GetBrush(SageBackgroundRole nRole) {
	return g_brushes[nRole];
}

COLORREF GetBackgroundColor(SageBackgroundRole nRole) {
	return g_backgroundColors[nRole];
}

COLORREF GetTextColor(SageTextRole nRole) {
	return g_textColors[nRole];
}

}
