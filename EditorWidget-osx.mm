#import "EditorWidget.h"
#import <Cocoa/Cocoa.h>

#import "VSTPlugin.h"

void EditorWidget::buildEffectContainer(AEffect *effect) {
    cocoaViewContainer = new QMacCocoaViewContainer(0, this);
    cocoaViewContainer->move(0, 0);
    cocoaViewContainer->resize(300, 300);
    NSView *view = [[NSView alloc] initWithFrame:  NSMakeRect(0, 0, 300, 300)];
    cocoaViewContainer->setCocoaView(view);

    VstRect* vstRect = 0;
    effect->dispatcher (effect, effEditGetRect, 0, 0, &vstRect, 0);
    if (vstRect)
    {
        NSRect frame = NSMakeRect(vstRect->left, vstRect->top, vstRect->right, vstRect->bottom);

        [view setFrame:frame];

        cocoaViewContainer->resize(vstRect->right - vstRect->left, vstRect->bottom- vstRect->top);
    }

    effect->dispatcher (effect, effEditOpen, 0, 0, view, 0);
}

void EditorWidget::handleResizeRequest(int width, int height) {
    resize(width, height);
    cocoaViewContainer->resize(width, height);
    NSView *view = cocoaViewContainer->cocoaView();
    NSRect frame = NSMakeRect(0, 0, width, height);

    [view setFrame:frame];
}
