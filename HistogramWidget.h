#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QImage>
#include <QVector>

class HistogramWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget *parent = nullptr);

    void setImage(const QImage &image);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void rebuildHistogram(const QImage &image);

    QVector<int> m_red;
    QVector<int> m_green;
    QVector<int> m_blue;
    int m_maxCount = 0;
    bool m_hasImage = false;
};

#endif // HISTOGRAMWIDGET_H
