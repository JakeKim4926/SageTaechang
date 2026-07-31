#include "pch.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

namespace {

CFont g_fontControl;
CFont g_fontContent;
CFont g_fontTitle;
CFont g_fontHeader;

constexpr int SAGE_BG_COUNT = SAGE_BG_STATUS_ERROR + 1;

const COLORREF g_backgroundColors[SAGE_BG_COUNT] = {
	TAECHANG_COLOR_APP_BACKGROUND,
	TAECHANG_COLOR_PANEL,
	TAECHANG_COLOR_SIDEBAR,
	TAECHANG_COLOR_STATUS_BG_SUCCESS,
	TAECHANG_COLOR_STATUS_BG_WARNING,
	TAECHANG_COLOR_STATUS_BG_ERROR
};

CBrush g_brushes[SAGE_BG_COUNT];

constexpr int SAGE_TEXT_COUNT = SAGE_TEXT_SIDEBAR_CATEGORY + 1;

const COLORREF g_textColors[SAGE_TEXT_COUNT] = {
	TAECHANG_COLOR_TEXT,
	TAECHANG_COLOR_SECONDARY_TEXT,
	TAECHANG_COLOR_PRIMARY,
	TAECHANG_COLOR_SUCCESS,
	TAECHANG_COLOR_ERROR,
	TAECHANG_COLOR_SIDEBAR_TEXT,
	TAECHANG_COLOR_SIDEBAR_CATEGORY
};

}

namespace SageUiResources {

void Create() {
	g_fontControl.CreatePointFont(TAECHANG_CONTROL_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontContent.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontTitle.CreatePointFont(TAECHANG_TITLE_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontHeader.CreatePointFont(TAECHANG_HEADER_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);

	for (int i = 0; i < SAGE_BG_COUNT; ++i)
		g_brushes[i].CreateSolidBrush(g_backgroundColors[i]);
}

void Destroy() {
	g_fontControl.DeleteObject();
	g_fontContent.DeleteObject();
	g_fontTitle.DeleteObject();
	g_fontHeader.DeleteObject();

	for (int i = 0; i < SAGE_BG_COUNT; ++i)
		g_brushes[i].DeleteObject();
}

CFont* GetFont(SageFontRole nRole) {
	switch (nRole) {
		case SAGE_FONT_CONTENT: return &g_fontContent;
		case SAGE_FONT_TITLE:   return &g_fontTitle;
		case SAGE_FONT_HEADER:  return &g_fontHeader;
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
