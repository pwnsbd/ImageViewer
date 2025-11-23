#include "imageviewer.h"
#include "./ui_imageviewer.h"
#include <QFileDialog>
#include <QDir>
#include <QPixmap>

ImageViewer::ImageViewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageViewer)
{
    ui->setupUi(this);

    connect(ui->actionOpen_Folder, &QAction::triggered,
            this, &ImageViewer::onOpenFolderClicked);

    connect(ui->folderListWidget, &QListWidget::itemClicked,
            this, &ImageViewer::onImageSelected);

    // Create a vertical layout for the dynamic property controls
    m_propertiesLayout = new QVBoxLayout(ui->propertiesPanel);
    m_propertiesLayout->setContentsMargins(4, 4, 4, 4);
    m_propertiesLayout->setSpacing(6);
}

void ImageViewer::clearPropertiesUI()
{
    // Remove and delete all widgets in the layout
    while (QLayoutItem* item = m_propertiesLayout->takeAt(0)) {
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }
    m_propertyControls.clear();
}

void ImageViewer::rebuildPropertiesUI(ImageItem &item)
{
    clearPropertiesUI();

    const QVector<ImageProperty> &props = item.properties();

    for (const ImageProperty &prop : props) {

        // For now we only handle Brightness + Contrast
        if (prop.id() != PropertyId::Brightness &&
            prop.id() != PropertyId::Contrast)
            continue;

        QWidget *row = new QWidget(ui->propertiesPanel);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(4);

        QLabel *label = new QLabel(prop.name(), row);
        QSlider *slider = new QSlider(Qt::Horizontal, row);

        int sliderMin = 0;
        int sliderMax = 100;
        int sliderValue = 50;

        if (prop.id() == PropertyId::Brightness) {
            // brightness backend value is 0–255
            sliderMin = 0;
            sliderMax = 100;

            int baseBrightness = item.brightness(); // 0–255 or -1
            if (baseBrightness >= 0) {
                sliderValue = (baseBrightness * 100) / 255;
            } else {
                sliderValue = 50;
            }
        }
        else if (prop.id() == PropertyId::Contrast) {
            // contrast backend value is already 0–100
            sliderMin = 0;
            sliderMax = 100;
            sliderValue = prop.value(); // 0–100, default 50
        }

        slider->setRange(sliderMin, sliderMax);
        slider->blockSignals(true);
        slider->setValue(sliderValue);
        slider->blockSignals(false);

        rowLayout->addWidget(label);
        rowLayout->addWidget(slider);
        m_propertiesLayout->addWidget(row);

        PropertyControl ctrl { prop.id(), slider, label };
        m_propertyControls.push_back(ctrl);

        connect(slider, &QSlider::valueChanged,
                this, &ImageViewer::onPropertySliderChanged);
    }

    m_propertiesLayout->addStretch();
}



ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::onOpenFolderClicked(){
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", QDir::homePath());

    if(folderPath.isEmpty())
    {
        return;
    }

    QDir dir(folderPath);
    QStringList filters = {"*.jpg", "*.png", "*.gif", "*.bmp", "*.jpeg"};
    QStringList files = dir.entryList(filters, QDir::Files);

    ui->folderListWidget->clear();
    m_images.clear();
    m_currentImageIndex = -1;

    for(const QString &fileName : files)
    {
        QString fullPath = dir.absoluteFilePath(fileName);

        m_images.push_back(ImageItem(fullPath));
        int index = m_images.length() - 1;

        QListWidgetItem *item = new QListWidgetItem(fileName);
        item->setData(Qt::UserRole, index);
        ui->folderListWidget->addItem(item);
    }
}

void ImageViewer::onImageSelected(QListWidgetItem *item){
    bool ok = false;
    int imageIndex = item->data(Qt::UserRole).toInt(&ok);

    if(!ok || imageIndex < 0 || imageIndex >= m_images.length())
    {
        qDebug() << "Invalid image index";
        return;
    }

    m_currentImageIndex = imageIndex;
    ImageItem &imgAtIndex = m_images[imageIndex];

    // 1) Load image in backend
    if (!imgAtIndex.loadImage()) {
        qDebug() << "Could not load image in backend:" << imgAtIndex.filepath();
        return;
    }

    // 2) Compute brightness in backend (fills ImageProperty)
    if (imgAtIndex.computeBrightness()) {
        int avg = imgAtIndex.brightness();   // 0–255
        qDebug() << "Average brightness (0-255):" << avg;
    }

    // 3) Show the original image (no adjustment yet)
    const QImage &img = imgAtIndex.originalImage();
    if (!img.isNull())
    {
        QPixmap pix = QPixmap::fromImage(img);
        ui->imageLabel->setPixmap(
            pix.scaled(ui->imageLabel->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation)
            );
    }
    else {
        qDebug() << "Image is null after load:" << imgAtIndex.filepath();
    }

    // 4) Rebuild right panel controls based on this image's properties
    rebuildPropertiesUI(imgAtIndex);
}

void ImageViewer::onPropertySliderChanged(int value)
{
    if (m_currentImageIndex < 0 || m_currentImageIndex >= m_images.size()) {
        return;
    }

    QSlider *slider = qobject_cast<QSlider*>(sender());
    if (!slider) {
        return;
    }

    // Find which property this slider belongs to
    PropertyId idToApply;
    bool found = false;

    for (const PropertyControl &ctrl : m_propertyControls) {
        if (ctrl.slider == slider) {
            idToApply = ctrl.id;
            found = true;
            break;
        }
    }

    if (!found) {
        return;
    }

    ImageItem &imgItem = m_images[m_currentImageIndex];
    QImage adjusted;

    switch (idToApply) {
    case PropertyId::Brightness:
        if (!imgItem.applyBrightnessLevel(value, adjusted)) {
            qDebug() << "Failed to apply brightness level";
            return;
        }
        break;

    case PropertyId::Contrast:
        if (!imgItem.applyContrastLevel(value, adjusted)) {
            qDebug() << "Failed to apply contrast level";
            return;
        }
        break;

    default:
        return;
    }

    QPixmap pix = QPixmap::fromImage(adjusted);
    ui->imageLabel->setPixmap(
        pix.scaled(ui->imageLabel->size(),
                   Qt::KeepAspectRatio,
                   Qt::SmoothTransformation)
        );
}

