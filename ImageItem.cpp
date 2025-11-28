#include "ImageItem.h"

ImageItem::ImageItem(const QImage& originalImage)
{
    // Normalize format for later processing
    if (originalImage.format() != QImage::Format_ARGB32) {
        m_originalImage = originalImage.convertToFormat(QImage::Format_ARGB32);
    } else {
        m_originalImage = originalImage;
    }

    m_editedImage = m_originalImage;
    m_hasEdits    = false;

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

void ImageItem::resetEdits()
{
    m_editedImage = m_originalImage;
    m_hasEdits    = false;
    // You can optionally reset property values here if you want full reset
}

ImageProperty* ImageItem::findProperty(PropertyId id)
{
    for (auto &prop : m_properties) {
        if (prop.id() == id)
            return &prop;
    }
    return nullptr;
}

const ImageProperty* ImageItem::findProperty(PropertyId id) const
{
    for (const auto &prop : m_properties) {
        if (prop.id() == id)
            return &prop;
    }
    return nullptr;
}

int ImageItem::propertyValue(PropertyId id) const
{
    const ImageProperty* prop = findProperty(id);
    if (!prop)
        return 0;
    return prop->value();
}

bool ImageItem::setPropertyValue(PropertyId id, int value)
{
    ImageProperty* prop = findProperty(id);
    if (!prop)
        return false;

    prop->setValue(value);
    return true;
}

void ImageItem::setEditedImage(const QImage& img)
{
    if (img.isNull()) {
        m_editedImage = m_originalImage;
        m_hasEdits    = false;
        return;
    }

    if (img.format() != QImage::Format_ARGB32) {
        m_editedImage = img.convertToFormat(QImage::Format_ARGB32);
    } else {
        m_editedImage = img;
    }

    m_hasEdits = true;
}
