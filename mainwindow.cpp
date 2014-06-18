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

    // Progressive cut
    connect(ui->buttonFore, SIGNAL(clicked()), this, SLOT(buttonForegroundClicked()));
    connect(ui->buttonBack, SIGNAL(clicked()), this, SLOT(buttonBackgroundClicked()));
    connect(ui->buttonGraph, SIGNAL(clicked()), this, SLOT(buttonCreateGraphClicked()));

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

    // Progressive cut
    progressiveCut = 0;
    graphCreated = false;
    segmentBackgroundNew = false;
    segmentForegroundNew = false;

    ui->maskMergeButton->setVisible(false);

    ui->buttonBack->setEnabled(false);
    ui->buttonFore->setEnabled(false);
    ui->buttonGraph->setEnabled(false);
}

MainWindow::~MainWindow()
{
    clearHistory();

    DeleteIfNotNull(selectionAlpha);
    DeleteIfNotNull(selectionBuffer);

    DeleteIfNotNull(maskImageAlpha);
    DeleteIfNotNull(maskImage);
    DeleteIfNotNull(maskCursor);

    DeleteIfNotNull(progressiveCut);

    delete image;
    delete ui;
}

//
//  Слоты меню
//
void MainWindow::menuFileOpen()
{
    loadImage();
    DeleteIfNotNull(progressiveCut);
    progressiveCut = new ProgressiveCut(*image);
    graphCreated = false;
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
            QMouseEvent* mouseEvent = (QMouseEvent*)event;

            int x = mouseEvent->x() - maskValue + 1;
            int y = mouseEvent->y() - maskValue + 1;

            if(event->type() == QEvent::MouseButtonPress)
            {
                maskingDrawing = true;
                maskIsEmpty = false;

                if (graphCreated)
                {
                    cursorWay.clear();
                    xy pos;
                    pos.x = x;
                    pos.y = y;
                    cursorWay << pos;
                }
                else
                {
                    QPainter painterAlpha(selectionAlpha);
                    painterAlpha.save();
                    painterAlpha.drawImage(x, y, *maskImageAlpha);
                    painterAlpha.restore();;
                }

                QPainter painterUi(maskedImage);
                painterUi.save();
                painterUi.drawImage(x, y, *maskImage);
                painterUi.restore();

                ui->imageView->setPixmap(QPixmap::fromImage(*maskedImage));
            }
            else if(event->type() == QEvent::MouseMove && maskingDrawing)
            {
                maskIsEmpty = false;

                if (graphCreated)
                {
                    xy pos;
                    pos.x = x;
                    pos.y = y;
                    cursorWay << pos;
                }
                else
                {
                    QPainter painterAlpha(selectionAlpha);
                    painterAlpha.save();
                    painterAlpha.drawImage(x, y, *maskImageAlpha);
                    painterAlpha.restore();
                }

                QPainter painterUi(maskedImage);
                painterUi.save();
                painterUi.drawImage(x, y, *maskImage);
                painterUi.restore();

                ui->imageView->setPixmap(QPixmap::fromImage(*maskedImage));
            }
            else if(event->type() == QEvent::MouseButtonRelease)
            {
                maskingDrawing = false;

                if (graphCreated)
                {
                    progressiveCut->setImageOutput(ui->imageView);
                    xy pos;
                    pos.x = x;
                    pos.y = y;
                    cursorWay << pos;

                    ui->maskButton->setChecked(false);
                    maskButtonClicked(false);
                }
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

void MainWindow::buttonForegroundClicked()
{
    if (graphCreated)
    {
        segmentForegroundNew = progressiveCut->updateForeground(*selectionAlpha);

        maskCancelButtonClicked();

        ui->maskButton->setEnabled(false);

        if (cursorWay.size() > 0)
        {
            progressiveCut->updateGraph(cursorWay, true, maskValue);
            cursorWay.clear();
        }

        ui->maskButton->setEnabled(true);
    }
    else
    {
        segmentForegroundNew = progressiveCut->setForeground(*selectionAlpha);

        maskCancelButtonClicked();
    }
}

void MainWindow::buttonBackgroundClicked()
{
    if (graphCreated)
    {
        segmentBackgroundNew = progressiveCut->updateBackground(*selectionAlpha);

        maskCancelButtonClicked();

        ui->maskButton->setEnabled(false);

        if (cursorWay.size() > 0)
        {
            progressiveCut->updateGraph(cursorWay, false, maskValue);
            cursorWay.clear();
        }

        ui->maskButton->setEnabled(true);
    }
    else
    {
        segmentBackgroundNew = progressiveCut->setBackground(*selectionAlpha);

        maskCancelButtonClicked();
    }
}

void MainWindow::buttonCreateGraphClicked()
{
    if (graphCreated)
    {
        /*if (!segmentBackgroundNew && !segmentForegroundNew)
        {
            QMessageBox(QMessageBox::Critical, "Невозможно обновить граф", "Нет новых мазков").exec();

            return;
        }

        segmentBackgroundNew = false;
        segmentForegroundNew = false;

        //progressiveCut->setImageOutput(ui->imageView);
        progressiveCut->updateGraph(cursorWay);

        delete image;
        image = new QImage(*progressiveCut->selection);
        ui->imageView->setPixmap(QPixmap::fromImage(*image));
        */
        ui->maskButton->setEnabled(true);
    }
    else
    {
        if (!segmentBackgroundNew)
        {
            QMessageBox(QMessageBox::Critical, "Невозможно создать граф", "Не указан фон").exec();

            return;
        }

        if (!segmentForegroundNew)
        {
            QMessageBox(QMessageBox::Critical, "Невозможно создать граф", "Не указан объект").exec();

            return;
        }

        //progressiveCut->setImageOutput(ui->imageView);
        progressiveCut->createGraph();

        delete image;
        image = new QImage(*progressiveCut->selection);
        ui->imageView->setPixmap(QPixmap::fromImage(*image));

        graphCreated = true;
        segmentBackgroundNew = false;
        segmentForegroundNew = false;

        ui->buttonGraph->setEnabled(false);
    }
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

        // Progressive cut
        ui->buttonBack->setEnabled(false);
        ui->buttonFore->setEnabled(false);
        ui->buttonGraph->setEnabled(false);
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

            // Progressive cut
            ui->buttonBack->setEnabled(true);
            ui->buttonFore->setEnabled(true);
            ui->buttonGraph->setEnabled(true);
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

    // Progressive cut
    ui->buttonBack->setEnabled(false);
    ui->buttonFore->setEnabled(false);
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

//
// Вычисление гистограмы
//
long *MainWindow::computeHistogramRGB()
{
    QColor pixelColor;

    // +2 для min и max
    long *histogram = new long [256 + 2],
        &min = histogram[256],
        &max = histogram[257],
        currentValue;

    memset(histogram, 0, (sizeof(long))*(256 + 2));

    for(int x = 0; x < image->width(); x++)
        for(int y = 0; y < image->height(); y++)
        {
            pixelColor.setRgb(image->pixel(x, y));
            currentValue = pixelColor.value();

            histogram[currentValue]++;

            if (currentValue > max) max = currentValue;
            if (currentValue < min) min = currentValue;
        }

    return histogram;
}
