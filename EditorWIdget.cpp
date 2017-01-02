#include "EditorWidget.h"

EditorWidget::EditorWidget(QWidget *parent, VSTPlugin *plugin) : QWidget(parent), plugin(plugin) {

}

void EditorWidget::closeEvent(QCloseEvent *event)
{
    plugin->closeEditor();
}