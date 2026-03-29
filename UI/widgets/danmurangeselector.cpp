#include "danmurangeselector.h"
#include "Play/Danmu/common.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

DanmuRangeSelector::DanmuRangeSelector(QWidget *parent) : QWidget{parent}
{
    setMouseTracking(true);
}

void DanmuRangeSelector::setData(const QVector<SimpleDanmuInfo> *data, int maxTimeMs)
{
    m_danmuData = data;
    m_maxTimeMs = qMax(maxTimeMs, 1);

    m_startTimeMs = 0;
    m_endTimeMs = m_maxTimeMs;

    recalculateDensity();
    update();
}

void DanmuRangeSelector::setStartTime(int startTimeMs)
{
    m_startTimeMs = qMin(qMax(0, startTimeMs), m_maxTimeMs);
    update();
    emit rangeChanged(m_startTimeMs, m_endTimeMs);
}

void DanmuRangeSelector::setEndTime(int endTimeMs)
{
    m_endTimeMs = qMax(qMin(endTimeMs, m_maxTimeMs), 0);
    update();
    emit rangeChanged(m_startTimeMs, m_endTimeMs);
}

void DanmuRangeSelector::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    recalculateDensity();
}

void DanmuRangeSelector::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    const int radius = 8;


    QPainterPath clipPath;
    clipPath.addRoundedRect(0, 0, w, h, radius, radius);
    painter.setClipPath(clipPath);

    painter.fillPath(clipPath, QColor(0, 0, 0, 220));

    if (!m_densityBuckets.isEmpty() && m_maxDensity > 0)
    {
        QPainterPath path;
        path.moveTo(0, h);

        for (int x = 0; x < m_densityBuckets.size(); ++x)
        {
            double ratio = (double)m_densityBuckets[x] / m_maxDensity;
            int drawY = h - (int)(ratio * (h - 4));
            path.lineTo(x, drawY);
        }
        path.lineTo(w, h);
        path.closeSubpath();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(51, 168, 255, 200));
        painter.drawPath(path);
    }

    int startX = timeToX(m_startTimeMs);
    int endX = timeToX(m_endTimeMs);
    painter.fillRect(0, 0, startX, h, QColor(0, 0, 0, 140));
    painter.fillRect(endX, 0, w - endX, h, QColor(0, 0, 0, 140));


    painter.setClipping(false);

    painter.setPen(QPen(QColor(0x409EFF), 2));
    painter.drawLine(startX, 0, startX, h);
    painter.drawLine(endX, 0, endX, h);
    painter.drawLine(startX, 0, endX, 0);
    painter.drawLine(startX, h, endX, h);

    int handleWidth = 6;
    int handleHeight = 20;
    int handleY = (h - handleHeight) / 2;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(startX - handleWidth/2, handleY, handleWidth, handleHeight, 2, 2);
    painter.drawRoundedRect(endX - handleWidth/2, handleY, handleWidth, handleHeight, 2, 2);
}

void DanmuRangeSelector::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    int x = event->pos().x();
    int startX = timeToX(m_startTimeMs);
    int endX = timeToX(m_endTimeMs);


    if (qAbs(x - startX) <= mouseDragOffset)
    {
        m_dragState = DragLeft;
    }
    else if (qAbs(x - endX) <= mouseDragOffset)
    {
        m_dragState = DragRight;
    }
    else if (x > startX && x < endX)
    {
        m_dragState = DragCenter;
        m_dragOffsetX = x - startX;
    }
    else
    {
        m_dragState = None;
    }
}

void DanmuRangeSelector::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->pos().x();
    int startX = timeToX(m_startTimeMs);
    int endX = timeToX(m_endTimeMs);

    if (m_dragState == None)
    {
        if (qAbs(x - startX) <= mouseDragOffset || qAbs(x - endX) <= mouseDragOffset)
        {
            setCursor(Qt::SizeHorCursor);
        }
        else if (x > startX && x < endX)
        {
            setCursor(Qt::SizeAllCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
        return;
    }

    if (m_dragState == DragLeft)
    {
        int newTime = xToTime(x);
        if (newTime < m_endTimeMs)
        {
            m_startTimeMs = qMax(0, newTime);
        }
    }
    else if (m_dragState == DragRight)
    {
        int newTime = xToTime(x);
        if (newTime > m_startTimeMs)
        {
            m_endTimeMs = qMin(m_maxTimeMs, newTime);
        }
    }
    else if (m_dragState == DragCenter)
    {
        int duration = m_endTimeMs - m_startTimeMs;
        int newStartX = x - m_dragOffsetX;
        int newStartTime = xToTime(newStartX);

        if (newStartTime >= 0 && newStartTime + duration <= m_maxTimeMs)
        {
            m_startTimeMs = newStartTime;
            m_endTimeMs = newStartTime + duration;
        }
        else if (newStartTime < 0)
        {
            m_startTimeMs = 0;
            m_endTimeMs = duration;
        }
        else
        {
            m_endTimeMs = m_maxTimeMs;
            m_startTimeMs = m_maxTimeMs - duration;
        }
    }

    update();
}

void DanmuRangeSelector::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragState != None)
    {
        m_dragState = None;
        emit rangeChanged(m_startTimeMs, m_endTimeMs);
    }
}

void DanmuRangeSelector::recalculateDensity()
{
    int w = width();
    if (w <= 0 || m_maxTimeMs <= 0) return;

    m_densityBuckets.resize(w);
    m_densityBuckets.fill(0);
    m_maxDensity = 0;

    for (const auto& danmu : *m_danmuData)
    {
        int x = timeToX(danmu.originTime);
        if (x >= 0 && x < w)
        {
            m_densityBuckets[x]++;
            if (m_densityBuckets[x] > m_maxDensity)
            {
                m_maxDensity = m_densityBuckets[x];
            }
        }
    }
}

int DanmuRangeSelector::timeToX(int timeMs) const
{
    if (m_maxTimeMs <= 0) return 0;
    return ((double)timeMs / m_maxTimeMs) * width();
}

int DanmuRangeSelector::xToTime(int x) const
{
    if (width() <= 0) return 0;
    return ((double)x / width()) * m_maxTimeMs;
}
