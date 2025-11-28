#include "imageviewer.h"
#include "./ui_imageviewer.h"

#include <QFileDialog>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QHBoxLayout>

ImageViewer::ImageViewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageViewer)
{
    ui->setupUi(this);

    connect(ui->actionOpen_Folder, &QAction::triggered,
            this, &ImageViewer::onOpenFolderClicked);

    connect(ui->folderListWidget, &QListWidget::itemClicked,
            this, &ImageViewer::onImageSelected);

    m_propertiesLayout = new QVBoxLayout(ui->propertiesPanel);
    m_propertiesLayout->setContentsMargins(4, 4, 4, 4);
    m_propertiesLayout->setSpacing(6);
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::clearPropertiesUI()
{
    if (!m_propertiesLayout)
        return;

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

        // You can keep this filter or remove it if you add more properties later
        if (prop.id() != PropertyId::Brightness &&
            prop.id() != PropertyId::Contrast)
            continue;

        QWidget *row = new QWidget(ui->propertiesPanel);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(4);

        QLabel *label = new QLabel(prop.name(), row);
        QSlider *slider = new QSlider(Qt::Horizontal, row);

        int sliderMin   = prop.min();
        int sliderMax   = prop.max();
        int sliderValue = prop.value();  // directly from backend

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

void ImageViewer::onOpenFolderClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select Folder"),
        QDir::homePath()
        );

    if (folderPath.isEmpty()) {
        return;
    }

    QDir dir(folderPath);
    QStringList filters = {"*.jpg", "*.png", "*.gif", "*.bmp", "*.jpeg"};
    QStringList files = dir.entryList(filters, QDir::Files);

    ui->folderListWidget->clear();
    m_images.clear();
    m_currentImageIndex = -1;

    clearPropertiesUI();
    ui->imageLabel->clear();

    for (const QString &fileName : files) {
        QString fullPath = dir.absoluteFilePath(fileName);

        QImage img(fullPath);
        if (img.isNull()) {
            qDebug() << "Failed to load image:" << fullPath;
            continue;
        }

        // Now ImageItem is purely an in-memory document
        m_images.push_back(ImageItem(img));
        int index = m_images.length() - 1;

        QListWidgetItem *item = new QListWidgetItem(fileName);
        item->setData(Qt::UserRole, index);
        ui->folderListWidget->addItem(item);
    }
}

void ImageViewer::onImageSelected(QListWidgetItem *item)
{
    bool ok = false;
    int imageIndex = item->data(Qt::UserRole).toInt(&ok);

    if (!ok || imageIndex < 0 || imageIndex >= m_images.length()) {
        qDebug() << "Invalid image index";
        return;
    }

    m_currentImageIndex = imageIndex;
    ImageItem &imgAtIndex = m_images[imageIndex];

    const QImage &img = imgAtIndex.editedImage(); // original if no edits
    if (!img.isNull()) {
        QPixmap pix = QPixmap::fromImage(img);
        ui->imageLabel->setPixmap(
            pix.scaled(ui->imageLabel->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation)
            );
    } else {
        qDebug() << "Image is null at selected index";
        ui->imageLabel->clear();
    }

    rebuildPropertiesUI(imgAtIndex);
}

void ImageViewer::onPropertySliderChanged(int value)
{
    Q_UNUSED(value);

    if (m_currentImageIndex < 0 ||
        m_currentImageIndex >= m_images.size()) {
        return;
    }

    QSlider *slider = qobject_cast<QSlider*>(sender());
    if (!slider) {
        return;
    }

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

    // 1) Update the property on the document
    if (!imgItem.setPropertyValue(idToApply, slider->value())) {
        return;
    }

    // 2) Recompute edited image using the processor
    QImage newEdited = ImageProcessor::applyAll(
        imgItem.originalImage(),
        imgItem.properties()
        );
    imgItem.setEditedImage(newEdited);

    // 3) Display the new edited image
    const QImage &img = imgItem.editedImage();
    if (!img.isNull()) {
        QPixmap pix = QPixmap::fromImage(img);
        ui->imageLabel->setPixmap(
            pix.scaled(ui->imageLabel->size(),
                       Qt::KeepAspectRatio,
                       Qt::SmoothTransformation)
            );
    }
}
