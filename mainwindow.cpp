#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "imageentry.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // File
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(menuFileOpen()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(menuFileExit()));

    image = new QImage();
}

MainWindow::~MainWindow()
{
    delete image;
    delete ui;
}

//
//  Слоты меню
//
void MainWindow::menuFileOpen()
{
    loadImage();
}

void MainWindow::menuFileExit()
{
    close();
}

//
//  Загрузка изображения
//
void MainWindow::loadImage()
{
    try
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите файл");
        image->load(fileName);

        ui->imageView->setPixmap(QPixmap::fromImage(*image));

        activateMenu();

        QLayout* historyLayout = ui->historyAreaContents->layout();
        historyLayout->addWidget(new imageEntry(ui->historyAreaContents, image, "Файл открыт 1"));
        historyLayout->addWidget(new imageEntry(ui->historyAreaContents, image, "Файл открыт 2"));
        historyLayout->addWidget(new imageEntry(ui->historyAreaContents, image, "Файл открыт 3"));
        historyLayout->setAlignment(historyLayout, Qt::AlignTop);
    }
    catch (...)
    {
        QMessageBox errorMessage;

        errorMessage.setText("Ошибка открытия файла");
        errorMessage.exec();
    }
}

void MainWindow::activateMenu()
{

}
