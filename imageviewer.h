#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QImage>
#include <QVector>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>

#include "HistogramWidget.h"
#include "ImageItem.h"
#include "ImageProcessor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class ImageViewer;
}
QT_END_NAMESPACE

class QResizeEvent;

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
    QVBoxLayout *m_adjustmentsLayout = nullptr;
    HistogramWidget *m_histogramWidget = nullptr;
    QLabel *m_adjustmentsHintLabel = nullptr;

    struct PropertyControl {
        PropertyId id;
        QSlider*   slider;
        QLabel*    valueLabel;
    };
    QVector<PropertyControl> m_propertyControls;

private slots:
    void onOpenFolderClicked();
    void onImageSelected(QListWidgetItem *item);
    void onPropertySliderChanged(int value);

private:
    void rebuildPropertiesUI(ImageItem &item);
    void clearPropertiesUI();
    void updateDisplayedImage(const QImage &image);
    void setupLayout();
    void setupImageListStyle();
    void showPropertiesEmptyState();

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // IMAGEVIEWER_H
