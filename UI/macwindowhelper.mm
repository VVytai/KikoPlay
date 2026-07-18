#include "macwindowhelper.h"

#include <QWidget>
#include <QWindow>

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

namespace MacWindowHelper
{
void applyRoundedCorners(QWidget *window, double radius)
{
    if (!window) return;

    // 确保原生窗口已创建
    window->winId();
    QWindow *qWindow = window->windowHandle();
    if (!qWindow) return;

    NSView *view = reinterpret_cast<NSView *>(qWindow->winId());
    if (!view) return;

    NSWindow *nsWindow = [view window];
    if (!nsWindow) return;

    // 让窗口背景透明，从而由内容层的圆角决定可见区域
    nsWindow.opaque = NO;
    nsWindow.backgroundColor = [NSColor clearColor];
    nsWindow.hasShadow = YES;

    NSView *contentView = [nsWindow contentView];
    if (!contentView) return;

    contentView.wantsLayer = YES;
    contentView.layer.cornerRadius = radius;
    contentView.layer.masksToBounds = YES;
    if (@available(macOS 10.15, *))
    {
        contentView.layer.cornerCurve = kCACornerCurveContinuous;
    }
}
}