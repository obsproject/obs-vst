#include "EditorWidget.h"

EditorWidget::EditorWidget(QWidget *parent, VSTPlugin *plugin) : QWidget(parent), plugin(plugin) {
	setWindowFlags(this->windowFlags() |= Qt::MSWindowsFixedSizeDialogHint);
}

void EditorWidget::closeEvent(QCloseEvent *event)
{
    plugin->closeEditor();
}