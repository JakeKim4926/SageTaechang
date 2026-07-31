#pragma once

enum SageFontRole {
	SAGE_FONT_CONTROL,
	SAGE_FONT_CONTENT,
	SAGE_FONT_TITLE,
	SAGE_FONT_HEADER
};

namespace SageUiResources {
	void Create();
	void Destroy();
	CFont* GetFont(SageFontRole nRole);
}
