#include "ImageItem.h"
#include <QColor>

// Constructor
ImageItem::ImageItem(const QString& filepath)
    : m_filepath(filepath)
{

    m_properties.push_back(
        ImageProperty(
            PropertyId::Brightness,
            "Brightness",
            0,
            255,
            -1
            )
        );

    m_properties.push_back(
        ImageProperty(
            PropertyId::Contrast,
            "Contrast",
            0,
            100,
            50   // neutral / no change
            )
        );
}


bool ImageItem::loadImage()
{
    if (!m_originalImage.isNull()) {
        return true;
    }

    QImage img(m_filepath);
    if (img.isNull()) {
        qDebug() << "Failed to load image:" << m_filepath;
        return false;
    }

    m_originalImage = img.convertToFormat(QImage::Format_ARGB32);
    return true;
}


ImageProperty* ImageItem::findProperty(PropertyId id) {
    for (auto &prop : m_properties) {
        if (prop.id() == id)
            return &prop;
    }
    return nullptr;
}

const ImageProperty* ImageItem::findProperty(PropertyId id) const {
    for (const auto &prop : m_properties) {
        if (prop.id() == id)
            return &prop;
    }
    return nullptr;
}

int ImageItem::brightness() const {
    const ImageProperty* prop = findProperty(PropertyId::Brightness);
    if (!prop) return -1;
    return prop->value();
}

bool ImageItem::computeBrightness()
{
    if (!loadImage()) {
        return false;
    }

    if (m_originalImage.isNull()) {
        return false;
    }

    const QImage &work = m_originalImage;

    int w = work.width();
    int h = work.height();
    if (w == 0 || h == 0) {
        return false;
    }

    long long total = 0;
    const int totalPixels = w * h;

    for (int y = 0; y < h; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(work.constScanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgba(line[x]);
            int b = (c.red() + c.green() + c.blue()) / 3; // simple brightness
            total += b;
        }
    }

    int avgBrightness = static_cast<int>(total / totalPixels);

    ImageProperty* brightnessProp = findProperty(PropertyId::Brightness);
    if (!brightnessProp) {
        qDebug() << "Brightness property not found on ImageItem!";
        return false;
    }

    brightnessProp->setValue(avgBrightness);

    qDebug() << "Computed brightness for" << m_filepath << ":" << avgBrightness;
    return true;
}

bool ImageItem::applyBrightnessLevel(int sliderValue, QImage &outImage)
{


    if (!loadImage()) {
        return false;
    }

    if (m_originalImage.isNull()) {
        return false;
    }

    ImageProperty* brightnessProp = findProperty(PropertyId::Brightness);
    if (!brightnessProp) {
        qDebug() << "Brightness property missing, computing...";
        if (!computeBrightness()) {
            return false;
        }
        brightnessProp = findProperty(PropertyId::Brightness);
        if (!brightnessProp) {
            return false;
        }
    }

    int baseBrightness = brightnessProp->value();
    if (baseBrightness <= 0) {
        baseBrightness = 1;
    }


    double baseNorm   = baseBrightness / 255.0;

    double targetNorm = sliderValue / 100.0;

    double ratio = (baseNorm > 0.0) ? (targetNorm / baseNorm) : 0.0;

    QImage adjusted = m_originalImage;

    int w = adjusted.width();
    int h = adjusted.height();

    for (int y = 0; y < h; ++y) {
        QRgb *scanLine = reinterpret_cast<QRgb*>(adjusted.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgba(scanLine[x]);

            int r = static_cast<int>(c.red()   * ratio);
            int g = static_cast<int>(c.green() * ratio);
            int b = static_cast<int>(c.blue()  * ratio);

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            c.setRed(r);
            c.setGreen(g);
            c.setBlue(b);

            scanLine[x] = c.rgba();
        }
    }

    outImage = adjusted;
    return true;
}

bool ImageItem::applyContrastLevel(int sliderValue, QImage &outImage)
{
    if (!loadImage()) {
        return false;
    }
    if (m_originalImage.isNull()) {
        return false;
    }

    ImageProperty* contrastProp = findProperty(PropertyId::Contrast);
    if (!contrastProp) {
        qDebug() << "Contrast property not found on ImageItem!";
        return false;
    }

    // Store last used value
    contrastProp->setValue(sliderValue);

    // Map slider 0..100 to contrast factor
    // 50 -> factor 1.0 (no change)
    // >50 -> higher contrast, <50 -> lower contrast
    double t = sliderValue / 50.0;   // 50 => 1.0, 100 => 2.0, 0 => 0.0

    QImage adjusted = m_originalImage;
    int w = adjusted.width();
    int h = adjusted.height();

    for (int y = 0; y < h; ++y) {
        QRgb *scanLine = reinterpret_cast<QRgb*>(adjusted.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgba(scanLine[x]);

            auto apply = [t](int ch) {
                // contrast around mid gray (128)
                int v = static_cast<int>((ch - 128) * t + 128);
                return qBound(0, v, 255);
            };

            int r = apply(c.red());
            int g = apply(c.green());
            int b = apply(c.blue());

            c.setRed(r);
            c.setGreen(g);
            c.setBlue(b);

            scanLine[x] = c.rgba();
        }
    }

    outImage = adjusted;
    return true;
}

