#include "imageviewer.h"
#include "./ui_imageviewer.h"

#include <QFileDialog>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QHBoxLayout>
#include <QFrame>
#include <QGroupBox>
#include <QListView>
#include <QResizeEvent>

ImageViewer::ImageViewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageViewer)
{
    ui->setupUi(this);
    setMinimumSize(1200, 720);
    setWindowTitle("Image Viewer");

    setupLayout();
    setupImageListStyle();
    showPropertiesEmptyState();

    connect(ui->actionOpen_Folder, &QAction::triggered,
            this, &ImageViewer::onOpenFolderClicked);

    connect(ui->folderListWidget, &QListWidget::itemClicked,
            this, &ImageViewer::onImageSelected);
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::clearPropertiesUI()
{
    if (!m_adjustmentsLayout)
        return;

    while (QLayoutItem* item = m_adjustmentsLayout->takeAt(0)) {
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    m_adjustmentsHintLabel = nullptr;
    m_propertyControls.clear();
}

void ImageViewer::rebuildPropertiesUI(ImageItem &item)
{
    clearPropertiesUI();

    if (m_histogramWidget) {
        m_histogramWidget->setImage(item.editedImage());
    }

    const QVector<ImageProperty> &props = item.properties();

    for (const ImageProperty &prop : props) {
        if (prop.id() != PropertyId::Brightness &&
            prop.id() != PropertyId::Contrast)
            continue;

        QFrame *controlCard = new QFrame(ui->propertiesPanel);
        controlCard->setObjectName("propertyCard");

        auto *cardLayout = new QVBoxLayout(controlCard);
        cardLayout->setContentsMargins(12, 12, 12, 12);
        cardLayout->setSpacing(8);

        QWidget *headerRow = new QWidget(controlCard);
        auto *headerLayout = new QHBoxLayout(headerRow);
        headerLayout->setContentsMargins(0, 0, 0, 0);
        headerLayout->setSpacing(8);

        QLabel *nameLabel = new QLabel(prop.name(), headerRow);
        QLabel *valueLabel = new QLabel(QString::number(prop.value()), headerRow);
        valueLabel->setObjectName("propertyValueBadge");

        headerLayout->addWidget(nameLabel);
        headerLayout->addStretch();
        headerLayout->addWidget(valueLabel);

        QSlider *slider = new QSlider(Qt::Horizontal, controlCard);
        slider->setRange(prop.min(), prop.max());
        slider->blockSignals(true);
        slider->setValue(prop.value());
        slider->blockSignals(false);

        cardLayout->addWidget(headerRow);
        cardLayout->addWidget(slider);
        m_adjustmentsLayout->addWidget(controlCard);

        PropertyControl ctrl { prop.id(), slider, valueLabel };
        m_propertyControls.push_back(ctrl);

        connect(slider, &QSlider::valueChanged,
                this, &ImageViewer::onPropertySliderChanged);
    }

    m_adjustmentsLayout->addStretch();
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
    ui->imageLabel->setText("Select an image to preview");
    ui->imageLabel->setPixmap(QPixmap());
    if (m_histogramWidget) {
        m_histogramWidget->clear();
    }
    showPropertiesEmptyState();

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
        item->setIcon(QPixmap::fromImage(img).scaled(56, 56,
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));
        item->setSizeHint(QSize(item->sizeHint().width(), 68));
        ui->folderListWidget->addItem(item);
    }

    if (ui->folderListWidget->count() > 0) {
        ui->folderListWidget->setCurrentRow(0);
        onImageSelected(ui->folderListWidget->currentItem());
    }
}

void ImageViewer::onImageSelected(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

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
        updateDisplayedImage(img);
    } else {
        qDebug() << "Image is null at selected index";
        ui->imageLabel->setText("Unable to preview image");
        ui->imageLabel->setPixmap(QPixmap());
        if (m_histogramWidget) {
            m_histogramWidget->clear();
        }
        showPropertiesEmptyState();
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
            if (ctrl.valueLabel) {
                ctrl.valueLabel->setText(QString::number(slider->value()));
            }
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
        updateDisplayedImage(img);
    }
}

void ImageViewer::setupLayout()
{
    ui->centralwidget->setStyleSheet(
        "QWidget#centralwidget { background: #0f172a; }"
        "QFrame#panelCard, QWidget#propertiesPanel {"
        "  background: #ffffff;"
        "  border: 1px solid #dbe4f0;"
        "  border-radius: 18px;"
        "}"
        "QLabel#sectionTitle { color: #0f172a; font-size: 18px; font-weight: 700; }"
        "QLabel#sectionSubtitle { color: #64748b; font-size: 12px; }"
        "QGroupBox {"
        "  border: 1px solid #dbe4f0;"
        "  border-radius: 14px;"
        "  margin-top: 12px;"
        "  padding-top: 10px;"
        "  font-weight: 600;"
        "  color: #0f172a;"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }"
        "QFrame#propertyCard {"
        "  background: #f8fafc;"
        "  border: 1px solid #e2e8f0;"
        "  border-radius: 12px;"
        "}"
        "QLabel#propertyValueBadge {"
        "  background: #dbeafe;"
        "  color: #1d4ed8;"
        "  border-radius: 10px;"
        "  padding: 3px 10px;"
        "  font-weight: 600;"
        "}"
        "QSlider::groove:horizontal { height: 6px; background: #cbd5e1; border-radius: 3px; }"
        "QSlider::sub-page:horizontal { background: #2563eb; border-radius: 3px; }"
        "QSlider::handle:horizontal {"
        "  background: #ffffff;"
        "  border: 2px solid #2563eb;"
        "  width: 16px;"
        "  margin: -6px 0;"
        "  border-radius: 8px;"
        "}"
    );

    auto *mainLayout = new QHBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    QFrame *listCard = new QFrame(ui->centralwidget);
    listCard->setObjectName("panelCard");
    auto *listLayout = new QVBoxLayout(listCard);
    listLayout->setContentsMargins(16, 16, 16, 16);
    listLayout->setSpacing(10);

    QLabel *listTitle = new QLabel("Images", listCard);
    listTitle->setObjectName("sectionTitle");
    QLabel *listSubtitle = new QLabel("Browse the images loaded from a folder.", listCard);
    listSubtitle->setObjectName("sectionSubtitle");
    listLayout->addWidget(listTitle);
    listLayout->addWidget(listSubtitle);
    listLayout->addWidget(ui->folderListWidget, 1);

    QFrame *previewCard = new QFrame(ui->centralwidget);
    previewCard->setObjectName("panelCard");
    auto *previewLayout = new QVBoxLayout(previewCard);
    previewLayout->setContentsMargins(20, 20, 20, 20);
    previewLayout->setSpacing(12);

    QLabel *previewTitle = new QLabel("Image Preview", previewCard);
    previewTitle->setObjectName("sectionTitle");
    QLabel *previewSubtitle = new QLabel("Selected image stays centered while you edit it.", previewCard);
    previewSubtitle->setObjectName("sectionSubtitle");

    QFrame *imageSurface = new QFrame(previewCard);
    imageSurface->setObjectName("panelCard");
    auto *imageLayout = new QVBoxLayout(imageSurface);
    imageLayout->setContentsMargins(18, 18, 18, 18);
    imageLayout->addWidget(ui->imageLabel, 1, Qt::AlignCenter);

    ui->imageLabel->setAlignment(Qt::AlignCenter);
    ui->imageLabel->setMinimumSize(420, 420);
    ui->imageLabel->setText("Open a folder to start");
    ui->imageLabel->setStyleSheet("QLabel { color: #64748b; font-size: 16px; border: none; background: transparent; }");

    previewLayout->addWidget(previewTitle);
    previewLayout->addWidget(previewSubtitle);
    previewLayout->addWidget(imageSurface, 1);

    ui->propertiesPanel->setObjectName("propertiesPanel");
    m_propertiesLayout = new QVBoxLayout(ui->propertiesPanel);
    m_propertiesLayout->setContentsMargins(16, 16, 16, 16);
    m_propertiesLayout->setSpacing(14);

    QLabel *propertiesTitle = new QLabel("Edit Controls", ui->propertiesPanel);
    propertiesTitle->setObjectName("sectionTitle");
    QLabel *propertiesSubtitle = new QLabel("Histogram and image adjustments are grouped here.", ui->propertiesPanel);
    propertiesSubtitle->setObjectName("sectionSubtitle");

    auto *histogramGroup = new QGroupBox("Histogram", ui->propertiesPanel);
    auto *histogramLayout = new QVBoxLayout(histogramGroup);
    histogramLayout->setContentsMargins(12, 12, 12, 12);
    m_histogramWidget = new HistogramWidget(histogramGroup);
    histogramLayout->addWidget(m_histogramWidget);

    auto *adjustmentsGroup = new QGroupBox("Brightness & Contrast", ui->propertiesPanel);
    m_adjustmentsLayout = new QVBoxLayout(adjustmentsGroup);
    m_adjustmentsLayout->setContentsMargins(12, 12, 12, 12);
    m_adjustmentsLayout->setSpacing(10);

    m_propertiesLayout->addWidget(propertiesTitle);
    m_propertiesLayout->addWidget(propertiesSubtitle);
    m_propertiesLayout->addWidget(histogramGroup);
    m_propertiesLayout->addWidget(adjustmentsGroup);
    m_propertiesLayout->addStretch();

    mainLayout->addWidget(listCard, 2);
    mainLayout->addWidget(previewCard, 5);
    mainLayout->addWidget(ui->propertiesPanel, 3);
}

void ImageViewer::setupImageListStyle()
{
    ui->folderListWidget->setViewMode(QListView::ListMode);
    ui->folderListWidget->setIconSize(QSize(56, 56));
    ui->folderListWidget->setSpacing(8);
    ui->folderListWidget->setUniformItemSizes(false);
    ui->folderListWidget->setStyleSheet(
        "QListWidget {"
        "  border: none;"
        "  outline: none;"
        "  background: transparent;"
        "  color: #0f172a;"
        "}"
        "QListWidget::item {"
        "  background: #f8fafc;"
        "  border: 1px solid #e2e8f0;"
        "  border-radius: 12px;"
        "  padding: 10px;"
        "  margin-bottom: 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background: #dbeafe;"
        "  border: 1px solid #93c5fd;"
        "  color: #1e3a8a;"
        "}"
        "QListWidget::item:hover {"
        "  background: #eff6ff;"
        "}"
    );
}

void ImageViewer::showPropertiesEmptyState()
{
    clearPropertiesUI();

    if (m_histogramWidget) {
        m_histogramWidget->clear();
    }

    m_adjustmentsHintLabel = new QLabel("Select an image to enable brightness and contrast controls.", ui->propertiesPanel);
    m_adjustmentsHintLabel->setWordWrap(true);
    m_adjustmentsHintLabel->setStyleSheet("QLabel { color: #64748b; background: transparent; }");
    m_adjustmentsLayout->addWidget(m_adjustmentsHintLabel);
    m_adjustmentsLayout->addStretch();
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_currentImageIndex >= 0 && m_currentImageIndex < m_images.size()) {
        const QImage &img = m_images[m_currentImageIndex].editedImage();
        if (!img.isNull()) {
            updateDisplayedImage(img);
        }
    }
}

void ImageViewer::updateDisplayedImage(const QImage &image)
{
    QPixmap pix = QPixmap::fromImage(image);
    ui->imageLabel->setText(QString());
    ui->imageLabel->setPixmap(
        pix.scaled(ui->imageLabel->size(),
                   Qt::KeepAspectRatio,
                   Qt::SmoothTransformation)
        );

    if (m_histogramWidget) {
        m_histogramWidget->setImage(image);
    }
}
