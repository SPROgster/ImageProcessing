#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QMouseEvent>

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

    //Маска
    connect(ui->maskButton, SIGNAL(clicked(bool)), this, SLOT(maskButtonClicked(bool)));
    connect(ui->maskMergeButton, SIGNAL(clicked()), this, SLOT(maskMergeButtonClicked()));
    connect(ui->maskCancel, SIGNAL(clicked()), this, SLOT(maskCancelButtonClicked()));
    connect(ui->maskSlider, SIGNAL(sliderMoved(int)), this, SLOT(maskSliderChanged(int)));
    connect(ui->maskSpin, SIGNAL(valueChanged(int)), this, SLOT(maskSpinChanged(int)));

    //Обработка рисования маски
    ui->imageView->installEventFilter(this);

    masking = false;
    maskIsEmpty = true;

    maskValue = 15;

    maskCursor = 0;
    maskImage = 0;
    maskImageAlpha = 0;

    ui->maskButton->setEnabled(false);
    ui->maskMergeButton->setEnabled(false);
    ui->maskCancel->setEnabled(false);
    ui->maskSlider->setEnabled(false);
    ui->maskSpin->setEnabled(false);

    selection = 0;
    selectionBuffer = 0;
    selectionAlpha = 0;

    // История
    historyLayout = ui->historyAreaContents->layout();

    historySpacer = new QSpacerItem(10, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    historyLayout->addItem(historySpacer);

    historyList.clear();
}

MainWindow::~MainWindow()
{
    clearHistory();

    DeleteIfNotNull(selectionAlpha);
    DeleteIfNotNull(selectionBuffer);

    DeleteIfNotNull(maskImageAlpha);
    DeleteIfNotNull(maskImage);
    DeleteIfNotNull(maskCursor);

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
    if (masking)
    {
        if (obj == (QObject*)(ui->imageView))
        {
            if(event->type() == QEvent::MouseButtonPress)
            {
                maskingDrawing = true;

                QMouseEvent* mouseEvent = (QMouseEvent*)event;

                int x = mouseEvent->x() - maskValue + 1;
                int y = mouseEvent->y() - maskValue + 1;

                QPainter painterUi(maskedImage);
                painterUi.save();
                painterUi.drawImage(x, y, *maskImage);
                painterUi.restore();

                QPainter painterAlpha(selectionAlpha);
                painterAlpha.save();
                painterAlpha.drawImage(x, y, *maskImageAlpha);
                painterAlpha.restore();

                ui->imageView->setPixmap(QPixmap::fromImage(*maskedImage));
            }
            else if(event->type() == QEvent::MouseMove && maskingDrawing)
            {
                maskIsEmpty = false;

                QMouseEvent* mouseEvent = (QMouseEvent*)event;

                int x = mouseEvent->x() - maskValue + 1;
                int y = mouseEvent->y() - maskValue + 1;

                QPainter painterUi(maskedImage);
                painterUi.save();
                painterUi.drawImage(x, y, *maskImage);
                painterUi.restore();

                QPainter painterAlpha(selectionAlpha);
                painterAlpha.save();
                painterAlpha.drawImage(x, y, *maskImageAlpha);
                painterAlpha.restore();

                ui->imageView->setPixmap(QPixmap::fromImage(*maskedImage));
            }
            else if(event->type() == QEvent::MouseButtonRelease)
            {
                maskingDrawing = false;
            }
        }
    }

    else
    if (event->type() == QEvent::MouseButtonPress)
    {
        int itemCount = historyLayout->count() - 1,
                index = historyLayout->indexOf((QWidget*)obj);

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

void MainWindow::maskButtonClicked(bool checked)
{
    if (checked)
    {   
        maskImage = disk(maskValue, QColor(255, 100, 100, 255));
        maskImageAlpha = disk(maskValue, Qt::white);

        maskCursor = new QCursor(QPixmap::fromImage(*maskImage));

        masking = true;
        maskingDrawing = false;
        ui->imageView->setCursor(*maskCursor);

        ui->maskSlider->setEnabled(true);
        ui->maskSpin->setEnabled(true);
        ui->maskCancel->setEnabled(false);
        ui->maskMergeButton->setEnabled(false);

        if (maskIsEmpty)
        {
            selection = new QImage(*image);
            selectionAlpha = new QImage(image->size(), QImage::Format_RGB32);
            selectionAlpha->fill(Qt::black);

            maskedImage = new QImage(*image);
        }
    }
    else
    {
        ui->imageView->setCursor(QCursor(Qt::ArrowCursor));

        if (maskImage != 0)
        {
            delete maskImage;
            maskImage = 0;

            delete maskImageAlpha;
            maskImageAlpha = 0;
        }
        if (maskCursor != 0)
        {
            delete maskCursor;
            maskCursor = 0;
        }

        if (!maskIsEmpty)
        {
            ui->maskCancel->setEnabled(true);
            ui->maskMergeButton->setEnabled(true);
        }
        else
        {
            delete selection;
            delete selectionAlpha;
            delete maskedImage;
            selection = 0;
            selectionAlpha = 0;
            maskedImage = 0;
        }
        ui->maskSlider->setEnabled(false);
        ui->maskSpin->setEnabled(false);
    }
}

void MainWindow::maskMergeButtonClicked()
{
    ui->maskCancel->setEnabled(false);
    ui->maskMergeButton->setEnabled(false);

    maskIsEmpty = true;

    selectionMerging();

    addEntryToHistory("После маски");

    delete selection;
    delete selectionAlpha;
    delete maskedImage;
    selection = 0;
    selectionAlpha = 0;
    maskedImage = 0;
}

void MainWindow::maskCancelButtonClicked()
{
    ui->maskCancel->setEnabled(false);
    ui->maskMergeButton->setEnabled(false);

    delete selection;
    delete selectionAlpha;
    delete maskedImage;
    selection = 0;
    selectionAlpha = 0;
    maskedImage = 0;

    ui->imageView->setPixmap(QPixmap::fromImage(*image));

    maskIsEmpty = true;
}

void MainWindow::maskSpinChanged(int value)
{
    ui->maskSlider->setValue(value);
    maskValueChanged(value);
}

void MainWindow::maskSliderChanged(int value)
{
    ui->maskSpin->setValue(value);
    maskValueChanged(value);
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
        image->convertToFormat(QImage::Format_ARGB32);

        ui->imageView->setPixmap(QPixmap::fromImage(*image));

        ui->imageView->setMinimumWidth(image->width());
        ui->imageView->setMaximumWidth(image->width());
        ui->imageView->setMinimumHeight(image->height());
        ui->imageView->setMaximumHeight(image->height());

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
    //Включение панели маски
    ui->maskButton->setEnabled(true);
}

void MainWindow::maskValueChanged(int value)
{
    maskValue = value;

    delete maskImage;
    delete maskImageAlpha;
    delete maskCursor;

    maskImage = disk(maskValue, QColor(255, 100, 100, 255));
    maskImageAlpha = disk(maskValue, Qt::white);

    maskCursor = new QCursor(QPixmap::fromImage(*maskImage));

    ui->imageView->setCursor(*maskCursor);
}

void MainWindow::addEntryToHistory(const QString &text, int index)
{
    imageEntry* entry = new imageEntry(ui->historyAreaContents, image, text);
    entry->setSelected(true);

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
        historyList.append(entry);
    }
    for (int i = 0; i < historyList.size() - 1; i++)
        historyList.at(i)->setSelected(false);

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

void MainWindow::selectionMerging()
{
    selection->setAlphaChannel(*selectionAlpha);
    QPainter painter(image);
    painter.save();
    painter.drawImage(0, 0, *selection);
    painter.restore();

    ui->imageView->setPixmap(QPixmap::fromImage(*image));
}

void MainWindow::selectionPreview()
{
    selectionBuffer->setAlphaChannel(*selectionAlpha);
    QImage buffer(*image);
    QPainter painter(&buffer);
    painter.save();
    painter.drawImage(0, 0, *selectionBuffer);
    painter.restore();

    ui->imageView->setPixmap(QPixmap::fromImage(buffer));

    repaint();
}
