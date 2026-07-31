#include "pch.h"
#include "app/ui/drawing/SageUiResources.h"
#include "TaechangDefine.h"

namespace {

CFont g_fontControl;
CFont g_fontContent;
CFont g_fontTitle;
CFont g_fontHeader;

}

namespace SageUiResources {

void Create() {
	g_fontControl.CreatePointFont(TAECHANG_CONTROL_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontContent.CreatePointFont(TAECHANG_CONTENT_FONT_POINT_SIZE, TAECHANG_CONTROL_FONT_FACE);
	g_fontTitle.CreatePointFont(TAECHANG_TITLE_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
	g_fontHeader.CreatePointFont(TAECHANG_HEADER_FONT_POINT_SIZE, TAECHANG_TITLE_FONT_FACE);
}

void Destroy() {
	g_fontControl.DeleteObject();
	g_fontContent.DeleteObject();
	g_fontTitle.DeleteObject();
	g_fontHeader.DeleteObject();
}

CFont* GetFont(SageFontRole nRole) {
	switch (nRole) {
		case SAGE_FONT_CONTENT: return &g_fontContent;
		case SAGE_FONT_TITLE:   return &g_fontTitle;
		case SAGE_FONT_HEADER:  return &g_fontHeader;
		default:                return &g_fontControl;
	}
}

}
