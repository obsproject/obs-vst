/*****************************************************************************
Copyright (C) 2016-2017 by Colin Edwards.
Additional Code Copyright (C) 2016-2017 by c3r1c3 <c3r1c3@nevermindonline.com>

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

void EditorWidget::buildEffectContainer(AEffect *effect)
{
	/* First we open a connection to the X Server. */
	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(connection) >= 1) {
		blog(LOG_ERROR, "VST: Couldn't connect to XCB.");
		return;
	}

	/* Now get the default/first screen. */
	const xcb_setup_t *   setup  = xcb_get_setup(connection);
	xcb_screen_iterator_t iter   = xcb_setup_roots_iterator(setup);
	xcb_screen_t *        screen = iter.data;
	if (!screen) {
		blog(LOG_WARNING, "VST: Couldn't setup screen.");
		return;
	}

	/* Create the window */
	xcb_window_t window_id = xcb_generate_id(connection);
	blog(LOG_ERROR, "VST: window_id is %d", window_id);
	if (window_id == 0) {
		blog(LOG_ERROR, "VST: Couldn't create XCB window.");
		return;
	}

	xcb_create_window(connection,                    // Connection
	                  XCB_COPY_FROM_PARENT,          // depth (same as root)
	                  window_id,                     // window Id
	                  screen->root,                  // parent window
	                  0,                             // x
	                  0,                             // y
	                  150,                           // width
	                  150,                           // height
	                  10,                            // border_width
	                  XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
	                  screen->root_visual,           // visual
	                  0,
	                  NULL                           // masks, not used yet
	                  );

	/* Map the window on the screen */
	xcb_map_window(connection, window_id);

	/* Make sure to flush so the Window shows. */
	xcb_flush(connection);

	/* Open the plugin. */
	effect->dispatcher(effect, effEditOpen, 0, 0, (void *)window_id, 0);

	/* Now render the plugin window. */
	VstRect *vstRect = nullptr;
	effect->dispatcher(effect, effEditGetRect, 0, 0, &vstRect, 0);

	/* Now we re-parent the Window to Qt so we can interact with it. */
	QWindow *vw = QWindow::fromWinId(window_id);
	vw->setFlags(Qt::FramelessWindowHint);
	QWidget *container = QWidget::createWindowContainer(vw, this);

	//container->move(0, 0);
	//container->resize(300, 300);

	if (vstRect) {
		container->resize(vstRect->right - vstRect->left, vstRect->bottom - vstRect->top);
	}

	this->repaint();
}

void EditorWidget::handleResizeRequest(int width, int height)
{
	// We don't have to do anything here as far as I can tell.
	// The widget will resize the HWIND itself and then
	// this widget will automatically size depending on that.
}
