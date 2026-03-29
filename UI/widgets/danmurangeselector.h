#ifndef DANMURANGESELECTOR_H
#define DANMURANGESELECTOR_H

#include <QWidget>

struct SimpleDanmuInfo;

class DanmuRangeSelector : public QWidget
{
    Q_OBJECT
public:
    explicit DanmuRangeSelector(QWidget *parent = nullptr);

    void setData(const QVector<SimpleDanmuInfo> *data, int maxTimeMs);
    void setStartTime(int startTimeMs);
    void setEndTime(int endTimeMs);

    int getStartTime() const { return m_startTimeMs; }
    int getEndTime() const { return m_endTimeMs; }

signals:
    void rangeChanged(int startTimeMs, int endTimeMs);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const QVector<SimpleDanmuInfo> *m_danmuData;
    int m_maxTimeMs = 1;
    int m_startTimeMs = 0;
    int m_endTimeMs = 1;

    QVector<int> m_densityBuckets; // 每个像素列对应的弹幕数量
    int m_maxDensity = 0;          // 弹幕最多的一列的数量（用于计算高度比例）

    enum DragState { None, DragLeft, DragRight, DragCenter };
    DragState m_dragState = None;
    int m_dragOffsetX = 0; // 拖拽中心区域时的鼠标相对偏移
    const int mouseDragOffset = 8;

    void recalculateDensity();
    int timeToX(int timeMs) const;
    int xToTime(int x) const;
};

#endif // DANMURANGESELECTOR_H
