#include "ImageProcessor.h"
#include <QtMath>

bool ImageProcessor::tryGetProperty(const QVector<ImageProperty>& properties,
                                    PropertyId id,
                                    int& outValue)
{
    for (const auto& prop : properties) {
        if (prop.id() == id) {
            outValue = prop.value();
            return true;
        }
    }
    return false;
}

QImage ImageProcessor::applyAll(const QImage& original,
                                const QVector<ImageProperty>& properties)
{
    if (original.isNull()) {
        return QImage();
    }

    QImage src = original;
    if (src.format() != QImage::Format_ARGB32) {
        src = src.convertToFormat(QImage::Format_ARGB32);
    }

    QImage dst(src.size(), QImage::Format_ARGB32);

    // Defaults (neutral)
    int brightnessSlider = 50;
    int contrastSlider   = 50;
    int tmp;

    if (tryGetProperty(properties, PropertyId::Brightness, tmp)) {
        brightnessSlider = tmp;
    }
    if (tryGetProperty(properties, PropertyId::Contrast, tmp)) {
        contrastSlider = tmp;
    }

    // Map slider ranges to parameters
    // Brightness: [-127, 127] ish
    double brightnessOffset = (brightnessSlider - 50) * (255.0 / 100.0);

    // Contrast: 50 -> 1.0, 0 -> 0.0, 100 -> 2.0
    double contrastFactor = contrastSlider / 50.0;
    if (contrastFactor < 0.0) contrastFactor = 0.0;

    auto clamp = [](int v) {
        if (v < 0)   return 0;
        if (v > 255) return 255;
        return v;
    };

    int w = src.width();
    int h = src.height();

    for (int y = 0; y < h; ++y) {
        const QRgb* srcLine = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        QRgb*       dstLine = reinterpret_cast<QRgb*>(dst.scanLine(y));

        for (int x = 0; x < w; ++x) {
            QRgb p = srcLine[x];

            int a = qAlpha(p);
            int r = qRed(p);
            int g = qGreen(p);
            int b = qBlue(p);

            r = clamp(int((r - 128) * contrastFactor + 128 + brightnessOffset));
            g = clamp(int((g - 128) * contrastFactor + 128 + brightnessOffset));
            b = clamp(int((b - 128) * contrastFactor + 128 + brightnessOffset));

            dstLine[x] = qRgba(r, g, b, a);
        }
    }

    return dst;
}
