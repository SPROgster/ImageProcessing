#include "coinviewdialog.h"
#include "ui_coinviewdialog.h"

coinViewDialog::coinViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::coinViewDialog)
{
    ui->setupUi(this);

    connect(ui->buttonNext, SIGNAL(clicked()), this, SLOT(nextImage()));
    connect(ui->buttonLast, SIGNAL(clicked()), this, SLOT(prevImage()));
}

coinViewDialog::~coinViewDialog()
{
    delete ui;
}

QImage* coinViewDialog::selectComponents(const QImage* origin, int& colorNumber)
{
    //QImage bitmap(origin->createMaskFromColor(0xFF000000, Qt::MaskOutColor).convertToFormat(QImage::Format_RGB32));

    QImage* componentsMap = new QImage(origin->width(), origin->height(), QImage::Format_RGB32);
    componentsMap->fill(QColor(0, 0, 0, 0));

    QRgb currColor = 0xFF000000;

    // Берем 2ю по счету строку и 2й по счету элемент. Граница изображения игнорируется
    QRgb* y1 = (QRgb*)origin->scanLine(0);
    QRgb* y2 = (QRgb*)componentsMap->scanLine(0);

    // Список активных маркеров. В последствие, когда будем соединять несколько линий, которая правее будет заменяться
    //на ту, что левее и помечаться в списке неактивной
    QList<bool> componentsActive;

    unsigned int  curveNum  =  0xFF000000,
                  curveNum1 =  0xFF000000;   // В curveNum будет хранится активный номер рядомстоящей точки

    if (*y1 & 0x1)
    {
        *y2 = ++currColor | 0xFF000000;
        componentsActive << true;

        curveNum = curveNum1 = currColor | 0xFF000000;
    }
    y1++;
    y2++;

    int width = origin->width();
    // Маркируем первую строку. Смотрим, стоит ли что слева, для этого curveNum пригодилась
    for (int x = 1; x < origin->width(); x++, y1++, y2++)
    {
        if (*y1 & 0x1)
        {
            if (curveNum & 0xFFFFFF)
                *y2 = curveNum | 0xFF000000;
            else
            {
                *y2 =  ++currColor | 0xFF000000;
                componentsActive << true;

                curveNum = currColor | 0xFF000000;
            }
        }
        else
            curveNum = 0;
    }

    curveNum = curveNum1;

    // Маркируем первый столбец
    for (int y = 1; y < origin->height(); y++, y1 += width, y2 += width)
    {
        if (*(y1) & 0x1)
        {
            if (curveNum & 0xFFFFFF)
                *y2 = curveNum  | 0xFF000000;
            else
            {
                *y2 = ++currColor | 0xFF000000;
                componentsActive << true;

                curveNum = currColor;
            }
        }
        else
            curveNum = 0;
    }

    // Теперь идем по внутренней части
    QRgb *bitmapPixel;
    bitmapPixel = (QRgb*)origin->scanLine(1);

    y1 = (QRgb *)componentsMap->scanLine(0);
    y2 = (QRgb *)componentsMap->scanLine(1);

    for (int y = 1; y < origin->height(); y++)
    {
        bitmapPixel++;
        y1++; y2++; // + 1 лишний раз будет делать for по x

        // Смотрим связные элементы по строке
        for (int x = 1; x < width; x++, y1++, y2++, bitmapPixel++)
            // Если пиксель не пустой, проверяем его окружение и относим к какой нибудь из областей
            if ( *bitmapPixel & 0xFF)
            {
                curveNum = 0;

                //Если мы тут, то под нами элемент. Осталось определить, к какой группе его определить

                // _______
                // |*| | |
                // _______
                // | |?| |
                curveNum1 = *(y1 - 1) & 0xFFFFFF;
                if (curveNum1)
                    // Если выбранный пиксель пронумерован, то проверяемый соединяем с ним
                    curveNum = curveNum1;

                // _______
                // | |*| |
                // _______
                // | |?| |
                curveNum1 = *y1 & 0xFFFFFF;
                if (curveNum1)
                {
                    if (curveNum != curveNum1 && curveNum)
                    {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                        componentsActive.replace(curveNum1 - 1, false);
                        replaceColor(componentsMap, 0xFF000000 | curveNum1, 0xFF000000 | curveNum);
                    }
                    else
                        curveNum = curveNum1;
                }

                // _______
                // | | | |
                // _______
                // |*|?| |
                //
                // TODO: теоритически, на это можно наложить ограничение, если сверху слева пусто и еще прямо сверху
                curveNum1 = *(y2 - 1) & 0xFFFFFF;
                if (curveNum1)
                {
                    if (curveNum != curveNum1 && curveNum)
                    {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                        componentsActive.replace(curveNum1 - 1, false);
                        replaceColor(componentsMap, 0xFF000000 | curveNum1, 0xFF000000 | curveNum);
                    }
                    else
                        curveNum = curveNum1;
                }

                // этот элемент может быть более новым, чем выше. чтобы не плодить новые
                // _______
                // | | |*|
                // _______
                // | |?| |
                if (x < origin->width() - 2)
                {
                    curveNum1 = *(y1 + 1) & 0xFFFFFF;
                    if (curveNum1)
                    {
                        if (curveNum != curveNum1 && curveNum)
                        {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                            componentsActive.replace(curveNum1 - 1, false);
                            replaceColor(componentsMap, 0xFF000000 | curveNum1, 0xFF000000 | curveNum);
                        }
                        else
                            curveNum = curveNum1;
                    }
                }

                // Если сосед есть
                if (curveNum)
                    *y2 = curveNum | 0xFF000000;
                else
                {   // Если соседа нету
                    *y2 = ++currColor | 0xFF000000;
                    componentsActive << true;
                }
            }
    }

    colorNumber = 1;

    for (int i = 0; i < componentsActive.size(); i++)
        if (componentsActive[i])
            replaceColor(componentsMap, 0xFF000000 + i + 1, 0xFF000000 + colorNumber++);

    componentsActive.clear();

    for (int i = 0; i < colorNumber; i++)
        componentsActive << true;

    return componentsMap;
}

QImage* coinViewDialog::imageGradient(const QImage *origin)
{
    // Преобразование в grayscale
    QImage* buffer = new QImage(origin->width(), origin->height(), QImage::Format_RGB32);
    buffer->fill(Qt::black);

    int width  = buffer->width();
    int height = buffer->height();

    int maskSize = (buffer->width() - 2) * (height - 2);

    int* gradientDenormStart;
    int* gradientDenorm = gradientDenormStart = new int[maskSize];

    QRgb *y1, *y2, *y3, *y4,
             *yOut;

    int min =  10000000;
    int max = -10000000;

    for (int y = 1; y < buffer->height() - 1; y++)
    {

        y1 = (QRgb*)origin->scanLine(y - 1);
        y2 = (QRgb*)origin->scanLine(y    );
        y3 = (QRgb*)origin->scanLine(y + 1);

        y1++;
        y2++;
        y3++;

        for (int x = 1; x < buffer->width() - 1; x++,
                                                 y1++, y2++, y3++,
                                                 gradientDenorm++)
        {
            // Вычисляем приближенное значение градиента
            *gradientDenorm = abs(-(hsvValue(*(y1 - 1)) + 2 * hsvValue(*y1) + hsvValue(*(y1 + 1)))
                                  + hsvValue(*(y3 - 1)) + 2 * hsvValue(*y3) + hsvValue(*(y3 + 1)))

                            + abs(-(hsvValue(*(y1 - 1)) + 2 * hsvValue(*(y2 - 1)) + hsvValue(*(y3 - 1)))
                                  + hsvValue(*(y1 + 1)) + 2 * hsvValue(*(y2 + 1)) + hsvValue(*(y3 + 1)));

            // Ищим максимум/минимум, чтобы потом отмасштабировать и уложить в изображение
            if (*gradientDenorm > max) max = *gradientDenorm;
            if (*gradientDenorm < min) min = *gradientDenorm;
        }
    }

    // Переходим на начало градиента
    gradientDenorm = gradientDenormStart;
    QRgb pixel;

    if (max-min > 254)
    {
        float mulCoef = 254. / (max - min);



        for (int y = 1; y < buffer->height() - 1; y++)
        {
            yOut = (QRgb*)buffer->scanLine(y);

            yOut++;

            for (int x = 1; x < buffer->width() - 1; x++,
                                                     yOut++,
                                                     gradientDenorm++)
            {
                pixel = (int)( (*gradientDenorm - min) * mulCoef);

                *yOut = (pixel << 16) + (pixel << 8) + pixel;
            }
        }
    }
    else
    {
        for (int y = 1; y < buffer->height() - 1; y++)
        {
            yOut = (QRgb*)buffer->scanLine(y);

            yOut++;

            for (int x = 1; x < buffer->width() - 1; x++,
                                                     yOut++,
                                                     gradientDenorm++)
            {
                pixel = *gradientDenorm & 0xFF;

                *yOut = (pixel << 16) + (pixel << 8) + pixel;
            }
        }
    }

    delete [] gradientDenormStart;

    // Копируем боковые участки
    memcpy(buffer->scanLine(0),          buffer->scanLine(1),          sizeof(QRgb) * width);
    memcpy(buffer->scanLine(height - 1), buffer->scanLine(height - 2), sizeof(QRgb) * width);

    y1 = y2 = (QRgb*)buffer->scanLine(0);
    y3 = y4 = (QRgb*)buffer->scanLine(1) - 1;
    y2++; y3--;
    for (int y = 0; y < height; y++, y1 += width, y2 += width, y3 += width, y4 += width)
    {
        *y1 = *y2;
        *y4 = *y3;
    }

    return buffer;
}

void coinViewDialog::loadCoins(const QImage *coins, const QImage& image)
{
    int colors;
//    QImage mask = coins->createMaskFromColor(0xFF000000).convertToFormat(QImage::Format_ARGB32);
    QImage* segments = selectComponents(coins, colors);

    coinsList.clear();
    for (int i = 0; i < colors; i++)
    {
        QImage coinAlpha(segments->createMaskFromColor(i + 1 + 0xFF000000));
        QImage coinImage(image);
        coinImage.setAlphaChannel(coinAlpha);
        coinsList << coinImage;
    }

    iterator = coinsList.begin();

    ui->imageView->setPixmap(QPixmap::fromImage(*(iterator + 1)));
}

void coinViewDialog::replaceColor(QImage *image, const QRgb colorToReplace, const QRgb newColor)
{
    QRgb* xLine = (QRgb*)image->scanLine(0);
    for (int y = 0; y < image->height(); y++)
        for (int x = 0; x < image->width(); x++, xLine++)
            if (*xLine == colorToReplace)
                *xLine = newColor;
}

int coinViewDialog::hsvValue(QRgb color)
{
    int R = (color >> 16) & 0xFF,
        G = (color >>  8) & 0xFF,
        B = (color      ) & 0xFF;

    int max = (R > G) ? R : G;

    return (max > B) ? max : B;
}

void coinViewDialog::nextImage()
{
    iterator++;
    if (iterator == coinsList.end())
    {
        iterator = coinsList.begin();
    }

    ui->imageView->setPixmap(QPixmap::fromImage(*iterator));
}

void coinViewDialog::prevImage()
{
    if (iterator == coinsList.begin())
        iterator = coinsList.end();

    iterator--;

    ui->imageView->setPixmap(QPixmap::fromImage(*iterator));
}
