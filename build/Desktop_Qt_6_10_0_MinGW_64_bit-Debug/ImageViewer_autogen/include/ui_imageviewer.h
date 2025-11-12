/********************************************************************************
** Form generated from reading UI file 'imageviewer.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IMAGEVIEWER_H
#define UI_IMAGEVIEWER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ImageViewer
{
public:
    QAction *btnOpenFolder;
    QAction *actionOpen_Folder;
    QWidget *centralwidget;
    QListWidget *folderListWidget;
    QLabel *imageLabel;
    QMenuBar *menuBar;
    QMenu *menuOpen;

    void setupUi(QMainWindow *ImageViewer)
    {
        if (ImageViewer->objectName().isEmpty())
            ImageViewer->setObjectName("ImageViewer");
        ImageViewer->resize(800, 600);
        btnOpenFolder = new QAction(ImageViewer);
        btnOpenFolder->setObjectName("btnOpenFolder");
        btnOpenFolder->setCheckable(false);
        btnOpenFolder->setEnabled(true);
        actionOpen_Folder = new QAction(ImageViewer);
        actionOpen_Folder->setObjectName("actionOpen_Folder");
        centralwidget = new QWidget(ImageViewer);
        centralwidget->setObjectName("centralwidget");
        folderListWidget = new QListWidget(centralwidget);
        folderListWidget->setObjectName("folderListWidget");
        folderListWidget->setGeometry(QRect(0, 0, 181, 571));
        imageLabel = new QLabel(centralwidget);
        imageLabel->setObjectName("imageLabel");
        imageLabel->setGeometry(QRect(198, 15, 551, 511));
        ImageViewer->setCentralWidget(centralwidget);
        menuBar = new QMenuBar(ImageViewer);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 800, 21));
        menuOpen = new QMenu(menuBar);
        menuOpen->setObjectName("menuOpen");
        ImageViewer->setMenuBar(menuBar);

        menuBar->addAction(menuOpen->menuAction());
        menuOpen->addAction(actionOpen_Folder);

        retranslateUi(ImageViewer);

        QMetaObject::connectSlotsByName(ImageViewer);
    } // setupUi

    void retranslateUi(QMainWindow *ImageViewer)
    {
        ImageViewer->setWindowTitle(QCoreApplication::translate("ImageViewer", "ImageViewer", nullptr));
        btnOpenFolder->setText(QCoreApplication::translate("ImageViewer", "Open folder", nullptr));
        actionOpen_Folder->setText(QCoreApplication::translate("ImageViewer", "Open Folder", nullptr));
        imageLabel->setText(QCoreApplication::translate("ImageViewer", "TextLabel", nullptr));
        menuOpen->setTitle(QCoreApplication::translate("ImageViewer", "Image Viewer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ImageViewer: public Ui_ImageViewer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IMAGEVIEWER_H
