//
// Created by Colin Edwards on 12/31/16.
//

#ifndef OBS_STUDIO_EDITORDIALOG_H
#define OBS_STUDIO_EDITORDIALOG_H

#include <QWidget>
#ifdef __APPLE__
#include <QMacCocoaViewContainer>
#endif
#include "aeffectx.h"
#include "VSTPlugin.h"

class VSTPlugin;

class EditorWidget : public QWidget {

    VSTPlugin *plugin;


#ifdef __APPLE__
    QMacCocoaViewContainer *cocoaViewContainer = NULL;
#elif WIN32

#endif

public:
    EditorWidget(QWidget *parent, VSTPlugin *plugin);
    void buildEffectContainer(AEffect *effect);
    void closeEvent(QCloseEvent *event) override;
    void handleResizeRequest(int width, int height);
};


#endif //OBS_STUDIO_EDITORDIALOG_H
