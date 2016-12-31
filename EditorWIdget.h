//
// Created by Colin Edwards on 12/31/16.
//

#ifndef OBS_STUDIO_EDITORDIALOG_H
#define OBS_STUDIO_EDITORDIALOG_H

#include <QWidget>
#include "aeffectx.h"

class EditorWidget : public QWidget {

public:
    EditorWidget(QWidget *parent);
    void buildEffectContainer(AEffect *effect);
};


#endif //OBS_STUDIO_EDITORDIALOG_H
