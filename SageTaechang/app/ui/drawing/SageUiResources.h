#pragma once

enum SageFontRole {
	SAGE_FONT_CONTROL,
	SAGE_FONT_CONTENT,
	SAGE_FONT_TITLE,
	SAGE_FONT_HEADER
};

enum SageBackgroundRole {
	SAGE_BG_APP,
	SAGE_BG_PANEL,
	SAGE_BG_SIDEBAR,
	SAGE_BG_STATUS_SUCCESS,
	SAGE_BG_STATUS_WARNING,
	SAGE_BG_STATUS_ERROR
};

namespace SageUiResources {
	void Create();
	void Destroy();
	CFont* GetFont(SageFontRole nRole);
	HBRUSH GetBrush(SageBackgroundRole nRole);
	COLORREF GetBackgroundColor(SageBackgroundRole nRole);
}
