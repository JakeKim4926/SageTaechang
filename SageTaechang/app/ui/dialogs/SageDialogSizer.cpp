#include "pch.h"
#include "app/ui/dialogs/SageDialogSizer.h"

namespace SageDialogSizer {

void SizeToClient(CWnd& dlg, int nClientWidth, int nClientHeight) {
	CRect rectClient;
	dlg.GetClientRect(&rectClient);
	if (rectClient.Width() == nClientWidth && rectClient.Height() == nClientHeight)
		return;

	CRect rectWindow;
	dlg.GetWindowRect(&rectWindow);
	int nFrameWidth = rectWindow.Width() - rectClient.Width();
	int nFrameHeight = rectWindow.Height() - rectClient.Height();

	dlg.SetWindowPos(NULL, 0, 0,
		nClientWidth + nFrameWidth, nClientHeight + nFrameHeight,
		SWP_NOMOVE | SWP_NOZORDER);
}

}
