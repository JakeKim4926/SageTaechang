#include "pch.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

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
	TAECHANG_COLOR_APP_BACKGROUND,
	TAECHANG_COLOR_PANEL,
	TAECHANG_COLOR_SIDEBAR,
	TAECHANG_COLOR_STATUS_BG_SUCCESS,
	TAECHANG_COLOR_STATUS_BG_WARNING,
	TAECHANG_COLOR_STATUS_BG_ERROR,
	TAECHANG_COLOR_ACCENT_SURFACE,
	TAECHANG_COLOR_LIST_HEADER,
	TAECHANG_COLOR_LIST_GRID
};

CBrush g_brushes[SAGE_BG_COUNT];

constexpr int SAGE_TEXT_COUNT = SAGE_TEXT_MUTED + 1;

const COLORREF g_textColors[SAGE_TEXT_COUNT] = {
	TAECHANG_COLOR_TEXT,
	TAECHANG_COLOR_SECONDARY_TEXT,
	TAECHANG_COLOR_PRIMARY,
	TAECHANG_COLOR_SUCCESS,
	TAECHANG_COLOR_ERROR,
	TAECHANG_COLOR_SIDEBAR_TEXT,
	TAECHANG_COLOR_SIDEBAR_CATEGORY,
	TAECHANG_COLOR_TEXT_MUTED
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
	g_fontControl.CreatePointFont(TAECHANG_CONTROL_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontContent.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontTitle.CreatePointFont(TAECHANG_TITLE_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontHeader.CreatePointFont(TAECHANG_HEADER_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontContentSemiBold.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontCaption.CreatePointFont(TAECHANG_CAPTION_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontSummary.CreatePointFont(TAECHANG_SUMMARY_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontList.CreatePointFont(TAECHANG_LIST_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontListSemiBold.CreatePointFont(TAECHANG_LIST_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontLogo.CreatePointFont(TAECHANG_TITLE_FONT_POINT_SIZE, TAECHANG_LOGO_FONT_FACE);
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
