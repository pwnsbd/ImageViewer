#include "HistogramWidget.h"

#include <algorithm>

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QSizePolicy>
#include <QtMath>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent)
    , m_red(256, 0)
    , m_green(256, 0)
    , m_blue(256, 0)
{
    setMinimumHeight(180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void HistogramWidget::setImage(const QImage &image)
{
    if (image.isNull()) {
        clear();
        return;
    }

    rebuildHistogram(image);
    m_hasImage = true;
    update();
}

void HistogramWidget::clear()
{
    std::fill(m_red.begin(), m_red.end(), 0);
    std::fill(m_green.begin(), m_green.end(), 0);
    std::fill(m_blue.begin(), m_blue.end(), 0);
    m_maxCount = 0;
    m_hasImage = false;
    update();
}

void HistogramWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#f8fafc"));

    QRectF graphRect = rect().adjusted(10, 10, -10, -28);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#ffffff"));
    painter.drawRoundedRect(graphRect.adjusted(-2, -2, 2, 2), 10, 10);

    painter.setPen(QPen(QColor("#e2e8f0"), 1));
    painter.drawRoundedRect(graphRect, 10, 10);

    for (int i = 1; i < 4; ++i) {
        const qreal y = graphRect.top() + (graphRect.height() * i / 4.0);
        painter.setPen(QPen(QColor("#e2e8f0"), 1, Qt::DashLine));
        painter.drawLine(QPointF(graphRect.left(), y), QPointF(graphRect.right(), y));
    }

    if (!m_hasImage || m_maxCount <= 0 || graphRect.width() <= 0 || graphRect.height() <= 0) {
        painter.setPen(QColor("#94a3b8"));
        painter.drawText(rect(), Qt::AlignCenter, "No histogram");
        return;
    }

    auto drawChannel = [&](const QVector<int> &channel, const QColor &color) {
        QPainterPath path;

        for (int i = 0; i < channel.size(); ++i) {
            const qreal x = graphRect.left() + (graphRect.width() * i / 255.0);
            const qreal normalized = channel[i] / static_cast<qreal>(m_maxCount);
            const qreal y = graphRect.bottom() - normalized * graphRect.height();

            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }

        painter.setPen(QPen(color, 2.0));
        painter.drawPath(path);
    };

    drawChannel(m_red, QColor(239, 68, 68, 200));
    drawChannel(m_green, QColor(34, 197, 94, 200));
    drawChannel(m_blue, QColor(59, 130, 246, 200));

    painter.setPen(QColor("#64748b"));
    painter.drawText(QRectF(rect().left(), rect().bottom() - 18, rect().width(), 16),
                     Qt::AlignHCenter | Qt::AlignVCenter,
                     "0                                              255");
}

void HistogramWidget::rebuildHistogram(const QImage &image)
{
    std::fill(m_red.begin(), m_red.end(), 0);
    std::fill(m_green.begin(), m_green.end(), 0);
    std::fill(m_blue.begin(), m_blue.end(), 0);
    m_maxCount = 0;

    QImage src = image;
    if (src.format() != QImage::Format_ARGB32) {
        src = src.convertToFormat(QImage::Format_ARGB32);
    }

    for (int y = 0; y < src.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        for (int x = 0; x < src.width(); ++x) {
            const QRgb pixel = line[x];
            ++m_red[qRed(pixel)];
            ++m_green[qGreen(pixel)];
            ++m_blue[qBlue(pixel)];
        }
    }

    for (int i = 0; i < 256; ++i) {
        m_maxCount = qMax(m_maxCount, m_red[i]);
        m_maxCount = qMax(m_maxCount, m_green[i]);
        m_maxCount = qMax(m_maxCount, m_blue[i]);
    }
}
