#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QString>
#include <QImage>
#include <QVector>
#include <QDebug>

#include "ImageProperty.h"

class ImageItem {

public:
    explicit ImageItem(const QString& filepath);

    QString filepath() const { return m_filepath; }

    bool loadImage();

    const QImage& originalImage() const { return m_originalImage; }

    const QVector<ImageProperty>& properties() const { return m_properties; }

    int brightness() const;

    bool computeBrightness();

    bool applyBrightnessLevel(int sliderValue, QImage &outImage);

    bool applyContrastLevel(int sliderValue, QImage &outImage);


private:
    QString m_filepath;
    QImage m_originalImage;          // base image
    QVector<ImageProperty> m_properties;

    ImageProperty* findProperty(PropertyId id);
    const ImageProperty* findProperty(PropertyId id) const;
};

#endif // IMAGEITEM_H
