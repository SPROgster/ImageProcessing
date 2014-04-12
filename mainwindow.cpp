#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    coinDialog = new CoinDialog(this);
    coinDialog->installEventFilter(this);

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

void MainWindow::menuEditCoinMask()
{
    ;
}

//
//  Фильтр событий
//
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == coinDialog)
    {
    }

    return QMainWindow::eventFilter(obj, event);
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

// Маска, выделяющая монетку
void MainWindow::MoneyMask(int size)
{
    QImage *element = disk(size, QColor(Qt::black));

    QImage* moneyMask = closing(image, element, (Qt::GlobalColor)Qt::black);

    ui->imageView->setPixmap(QPixmap::fromImage(*moneyMask));

    repaint();

    delete moneyMask;
    delete element;
}
