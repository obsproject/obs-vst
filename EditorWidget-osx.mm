#import "EditorWidget.h"

#import <QMacCocoaViewContainer>
#import <Cocoa/Cocoa.h>

#import "VSTPlugin.h"

void EditorWidget::buildEffectContainer(AEffect *effect) {
    QMacCocoaViewContainer *cocoaViewContainer = new QMacCocoaViewContainer(0, this);
    cocoaViewContainer->move(0, 0);
    cocoaViewContainer->resize(300, 300);
    NSView *view =[[NSView alloc] initWithFrame:  NSMakeRect(0, 0, 300, 300)];
    cocoaViewContainer->setCocoaView(view);

    effect->dispatcher (effect, effEditOpen, 0, 0, view, 0);

    ERect* eRect = 0;
    effect->dispatcher (effect, effEditGetRect, 0, 0, &eRect, 0);
    if (eRect)
    {
        NSRect frame = NSMakeRect(eRect->left, eRect->top, eRect->right, eRect->bottom);

        [view setFrame:frame];

        cocoaViewContainer->resize(eRect->right - eRect->left, eRect->bottom- eRect->top);
    }
}