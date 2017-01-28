/*****************************************************************************
Copyright (C) 2016-2017 by Colin Edwards.
Additional Code Copyright (C) 2016-2017 by c3r1c3 <c3r1c3@nevermindonline.com>

Special thanks to Nik Reiman for sharing his awesome code with the world.
Some of the original code can be found here:
https://github.com/teragonaudio/MrsWatson/blob/master/source/plugin/PluginVst2xLinux.cpp

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "../headers/EditorWidget.h"

#include <X11/Xlib.h>
#include <QWindow>


void EditorWidget::buildEffectContainer(AEffect *effect) {
	Display *display;
	Window window;
	XEvent event;

	int screenNumber;

	blog(LOG_WARNING, "Opening X display");
	display = XOpenDisplay(NULL);
	if (display == NULL)
	{
		blog(LOG_WARNING, "Can't open default display");
		return;
	}

	blog(LOG_WARNING, "Acquiring default screen for X display");
	screenNumber = DefaultScreen(display);
	Screen *screen = DefaultScreenOfDisplay(display);

	int screenWidth = WidthOfScreen(screen);
	int screenHeight = HeightOfScreen(screen);
	blog(LOG_WARNING, "Screen dimensions: %dx%d", screenWidth, screenHeight);

	// Default size is 300x300 pixels
	int windowX = (screenWidth - 300) / 2;
	int windowY = (screenHeight - 300) / 2;

	blog(LOG_WARNING, "Creating window at %dx%d", windowX, windowY);
	window = XCreateSimpleWindow(display, RootWindow(display, screenNumber),
			0, 0, 300, 300, 1, BlackPixel(display, screenNumber),
			BlackPixel(display, screenNumber));

	//XStoreName(display, window, pluginName->data);
/*
	XSelectInput(display, window, ExposureMask | KeyPressMask);
	XMapWindow(display, window);
	XMoveWindow(display, window, windowX, windowY);


	blog(LOG_WARNING, "Opening plugin editor window");
	effect->dispatcher(effect, effEditOpen, 0, 0, (void *) window, 0);

	while (true) {
		XNextEvent(display, &event);

		if (event.type == Expose) {
		}

		if (event.type == KeyPress) {
			break;
		}
	}

	blog(LOG_WARNING, "Closing plugin editor window");
	effect->dispatcher(effect, effEditClose, 0, 0, 0, 0);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
*/
	/*
	QWidget *widget = QWidget::createWindowContainer(QWindow::window);
	widget->move(0, 0);
	widget->resize(300, 300);

	effect->dispatcher(effect, effEditOpen, 0, 0, window, 0);

	VstRect* vstRect = nullptr;
	effect->dispatcher(effect, effEditGetRect, 0, 0, &vstRect, 0);
	if (vstRect)
	{
		widget->resize(vstRect->right - vstRect->left,
			       vstRect->bottom - vstRect->top);
	}
	 */
}

void EditorWidget::handleResizeRequest(int width, int height) {
	// We don't have to do anything here as far as I can tell.
	// The widget will resize the HWIND itself and then
	// this widget will automatically size depending on that.
}