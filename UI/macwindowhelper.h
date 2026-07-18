#ifndef MACWINDOWHELPER_H
#define MACWINDOWHELPER_H

class QWidget;

namespace MacWindowHelper
{
// 为无边框窗口应用 macOS 原生的圆角与阴影效果。
// radius 为圆角半径（像素）。
void applyRoundedCorners(QWidget *window, double radius = 10.0);
}

#endif // MACWINDOWHELPER_H