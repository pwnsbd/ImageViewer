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

    // Edited image: if no edits yet, returns original
    const QImage& editedImage() const {
        return m_hasEdits ? m_editedImage : m_originalImage;
    }

    bool hasEdits() const { return m_hasEdits; }

    void resetEdits() {
        m_editedImage = m_originalImage;
        m_hasEdits = false;
    }

    const QVector<ImageProperty>& properties() const { return m_properties; }

    // Editing operations (no outImage; backend updates its own state)
    bool applyBrightnessLevel(int sliderValue); // 0–100
    bool applyContrastLevel(int sliderValue);   // 0–100

private:
    QString m_filepath;
    QImage  m_originalImage;
    QImage  m_editedImage;
    bool    m_hasEdits = false;

    QVector<ImageProperty> m_properties;

    ImageProperty*       findProperty(PropertyId id);
    const ImageProperty* findProperty(PropertyId id) const;
};

#endif // IMAGEITEM_H
