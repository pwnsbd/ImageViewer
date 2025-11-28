#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QImage>
#include <QVector>

#include "ImageProperty.h"

class ImageItem
{
public:
    // Construct from an already-loaded image (no filepath here)
    explicit ImageItem(const QImage& originalImage);

    const QImage& originalImage() const { return m_originalImage; }

    // Edited image: if no edits yet, returns original
    const QImage& editedImage() const {
        return m_hasEdits ? m_editedImage : m_originalImage;
    }

    bool hasEdits() const { return m_hasEdits; }

    void resetEdits();

    const QVector<ImageProperty>& properties() const { return m_properties; }

    // Generic property access
    int  propertyValue(PropertyId id) const;
    bool setPropertyValue(PropertyId id, int value);

    // For the processor to push new image data
    void setEditedImage(const QImage& img);

private:
    QImage m_originalImage;
    QImage m_editedImage;
    bool   m_hasEdits = false;

    QVector<ImageProperty> m_properties;

    ImageProperty*       findProperty(PropertyId id);
    const ImageProperty* findProperty(PropertyId id) const;
};

#endif // IMAGEITEM_H
