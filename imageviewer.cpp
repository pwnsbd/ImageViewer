#include "imageviewer.h"
#include "./ui_imageviewer.h"
#include "QFileDialog"
#include "QDir"
#include "QPixmap"

ImageViewer::ImageViewer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ImageViewer)
{
    ui->setupUi(this);
    connect(ui->actionOpen_Folder, &QAction::triggered, this, &ImageViewer::onOpenFolderClicked);
    connect(ui->folderListWidget, &QListWidget::itemClicked, this, &ImageViewer::onImageSelected);
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
    QStringList filters = {"*.jpg", "*.png", "*.gif", "*.bmp", "*.jpeg",};
    QStringList files = dir.entryList(filters, QDir::Files);

    ui->folderListWidget->clear();
    for(const QString &fileName : files)
    {
        QString fullPath = dir.absoluteFilePath(fileName);
        QListWidgetItem *item = new QListWidgetItem(fileName);
        qDebug()<<fullPath;
        item->setData(Qt::UserRole, fullPath);
        ui->folderListWidget->addItem(item);
    }
}

void ImageViewer::onImageSelected(QListWidgetItem *item){
    QString imagePath = item->data(Qt::UserRole).toString();

    QPixmap pix(imagePath);
    if(!pix.isNull())
    {
        ui->imageLabel->setPixmap(pix.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else{
        qDebug() << "image could not be loaded";
    }
}

