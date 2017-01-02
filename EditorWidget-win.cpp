#include "EditorWidget.h"

#include <QWindow>
#include <Windows.h>

void EditorWidget::buildEffectContainer(AEffect *effect) {
	WNDCLASSEX wcex{ sizeof(wcex) };
	wcex.lpfnWndProc = DefWindowProc;
	wcex.hInstance = GetModuleHandle(0);
	wcex.lpszClassName = L"Minimal VST host - Guest VST Window Frame";
	RegisterClassEx(&wcex);

	const auto style = WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPEDWINDOW;
	HWND hwnd = CreateWindow(
		wcex.lpszClassName, TEXT(""), style
		, 0, 0, 0, 0, 0, 0, 0, 0
	);

	QWidget *widget = QWidget::createWindowContainer(QWindow::fromWinId((WId)hwnd), this);
	widget->move(0, 0);
	widget->resize(300, 300);

	effect->dispatcher(effect, effEditOpen, 0, 0, hwnd, 0);

	ERect* eRect = 0;
	effect->dispatcher(effect, effEditGetRect, 0, 0, &eRect, 0);
	if (eRect)
	{
		widget->resize(eRect->right - eRect->left, eRect->bottom - eRect->top);
	}
}

void EditorWidget::handleResizeRequest(int width, int height) {
	// We don't have to do anything here as far as I can tell. The widget will resize the HWIND itself and then this widget will automatically size depending on that.
}