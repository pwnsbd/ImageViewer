#include "ImageItem.h"
#include <QColor>

// Constructor
ImageItem::ImageItem(const QString& filepath)
    : m_filepath(filepath)
{
    // Brightness: 0–100, 50 = neutral
    m_properties.push_back(
        ImageProperty(
            PropertyId::Brightness,
            "Brightness",
            0,
            100,
            50
            )
        );

    // Contrast: 0–100, 50 = neutral
    m_properties.push_back(
        ImageProperty(
            PropertyId::Contrast,
            "Contrast",
            0,
            100,
            50
            )
        );
}

bool ImageItem::loadImage()
{
    if (!m_originalImage.isNull()) {
        return true; // already loaded
    }

    QImage img(m_filepath);
    if (img.isNull()) {
        qDebug() << "Failed to load image:" << m_filepath;
        return false;
    }

    m_originalImage = img.convertToFormat(QImage::Format_ARGB32);
    m_editedImage   = m_originalImage;
    m_hasEdits      = false;
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

// Brightness: 0–100, 50 = neutral, <50 darker, >50 brighter
bool ImageItem::applyBrightnessLevel(int sliderValue)
{
    if (!loadImage()) return false;
    if (m_originalImage.isNull()) return false;

    ImageProperty* brightnessProp = findProperty(PropertyId::Brightness);
    if (!brightnessProp) {
        qDebug() << "Brightness property not found on ImageItem!";
        return false;
    }
    brightnessProp->setValue(sliderValue);

    // Start from original each time for now
    QImage base = m_originalImage;
    QImage adjusted = base;

    double factor = sliderValue / 50.0; // 50 -> 1.0, 25 -> 0.5, 100 -> 2.0

    int w = adjusted.width();
    int h = adjusted.height();

    for (int y = 0; y < h; ++y) {
        QRgb *scanLine = reinterpret_cast<QRgb*>(adjusted.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgba(scanLine[x]);

            int r = static_cast<int>(c.red()   * factor);
            int g = static_cast<int>(c.green() * factor);
            int b = static_cast<int>(c.blue()  * factor);

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            c.setRed(r);
            c.setGreen(g);
            c.setBlue(b);

            scanLine[x] = c.rgba();
        }
    }

    m_editedImage = adjusted;
    m_hasEdits    = true;
    return true;
}

// Contrast: 0–100, 50 = neutral, around mid-grey (128)
bool ImageItem::applyContrastLevel(int sliderValue)
{
    if (!loadImage()) return false;
    if (m_originalImage.isNull()) return false;

    ImageProperty* contrastProp = findProperty(PropertyId::Contrast);
    if (!contrastProp) {
        qDebug() << "Contrast property not found on ImageItem!";
        return false;
    }
    contrastProp->setValue(sliderValue);

    // Start from original each time for now
    QImage base = m_originalImage;
    QImage adjusted = base;

    // 50 -> 1.0, 100 -> 2.0, 0 -> 0.0
    double t = sliderValue / 50.0;

    int w = adjusted.width();
    int h = adjusted.height();

    for (int y = 0; y < h; ++y) {
        QRgb *scanLine = reinterpret_cast<QRgb*>(adjusted.scanLine(y));
        for (int x = 0; x < w; ++x) {
            QColor c = QColor::fromRgba(scanLine[x]);

            auto apply = [t](int ch) {
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

    m_editedImage = adjusted;
    m_hasEdits    = true;
    return true;
}
