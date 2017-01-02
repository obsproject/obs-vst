//
// Created by Colin Edwards on 12/31/16.
//

#ifndef OBS_STUDIO_EDITORDIALOG_H
#define OBS_STUDIO_EDITORDIALOG_H

#include <QWidget>
#include "aeffectx.h"
#include "VSTPlugin.h"

class VSTPlugin;

class EditorWidget : public QWidget {

    VSTPlugin *plugin;

public:
    EditorWidget(QWidget *parent, VSTPlugin *plugin);
    void buildEffectContainer(AEffect *effect);
    void closeEvent(QCloseEvent *event) override;
};


#endif //OBS_STUDIO_EDITORDIALOG_H
