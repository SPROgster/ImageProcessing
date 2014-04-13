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

    historyLayout = ui->historyAreaContents->layout();

    historySpacer = new QSpacerItem(10, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    historyLayout->addItem(historySpacer);

    historyList.clear();
}

MainWindow::~MainWindow()
{
    clearHistory();

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
//  Фильтр событий
//
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        int itemCount = historyLayout->count() - 1,
                index = historyLayout->indexOf((QWidget*)obj),
                 size = historyList.size();

        if (index >= 0 && index < itemCount)
        {
            // Назначение активным
            for(int i = 0; i < historyList.size(); i++)
                historyList.at(i)->setSelected(i == index);

            // Возвращение к текущему состоянию
            delete image;

            image = new QImage(historyList.at(index)->getImage());
            ui->imageView->setPixmap(QPixmap::fromImage(*image));

            repaint();
        }
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

        clearHistory();

        addEntryToHistory("Файл открыт");
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

void MainWindow::addEntryToHistory(const QString &text, int index)
{
    imageEntry* entry = new imageEntry(ui->historyAreaContents, image, text);

    historyLayout->removeItem(historySpacer);

    if (index == -1)
        historyList.append(entry);
    else
    {
        for(int i = historyLayout->count() - 1; i >= index; i--)
        {
            historyLayout->removeItem(historyLayout->itemAt(i));
            delete historyList.at(i);
            historyList.removeAt(i);
        }
    }

    historyLayout->addWidget(entry);
    historyLayout->setAlignment(entry, Qt::AlignTop);

    // Для обработки событий
    entry->installEventFilter(this);

    historyLayout->addItem(historySpacer);
}

void MainWindow::clearHistory()
{
    historyLayout->removeItem(historySpacer);

    for (int i = historyLayout->count() - 1; i >=0; i--)
    {
        historyLayout->removeItem(historyLayout->itemAt(i));
        delete historyList.at(i);
    }
    historyList.clear();

    historyLayout->addItem(historySpacer);
}
