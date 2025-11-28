#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QImage>
#include <QVector>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

#include "ImageItem.h"
#include "ImageProcessor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ImageViewer;
}
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr);
    ~ImageViewer();

private:
    Ui::ImageViewer *ui;
    QVector<ImageItem> m_images;
    int m_currentImageIndex = -1;

    QVBoxLayout *m_propertiesLayout = nullptr;

    struct PropertyControl {
        PropertyId id;
        QSlider*   slider;
        QLabel*    label;
    };
    QVector<PropertyControl> m_propertyControls;

private slots:
    void onOpenFolderClicked();
    void onImageSelected(QListWidgetItem *item);
    void onPropertySliderChanged(int value);

private:
    void rebuildPropertiesUI(ImageItem &item);
    void clearPropertiesUI();
};

#endif // IMAGEVIEWER_H
