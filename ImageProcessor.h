#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <QVector>

#include "ImageProperty.h"

class ImageProcessor
{
public:
    // Apply all properties to original image and return a new edited image
    static QImage applyAll(const QImage& original,
                           const QVector<ImageProperty>& properties);

private:
    static bool tryGetProperty(const QVector<ImageProperty>& properties,
                               PropertyId id,
                               int& outValue);
};

#endif // IMAGEPROCESSOR_H
