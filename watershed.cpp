#include <QImage>
#include <QBitmap>
#include "watershed.h"

QImage* imageGradient(const QImage *origin)
{
    // Преобразование в grayscale
    QImage* buffer = new QImage(origin->width(), origin->height(), QImage::Format_RGB32);
    buffer->fill(Qt::black);

    int maskSize = (buffer->width() - 2) * (buffer->height() - 2);

    int* gradientDenormStart;
    int* gradientDenorm = gradientDenormStart = new int[maskSize];

    QRgb *y1, *y2, *y3,
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

    float mulCoef = 255. / (max - min);

    QRgb pixel;

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

    delete [] gradientDenormStart;

    return buffer;
}


int hsvValue(QRgb color)
{
    int R = (color >> 16) & 0xFF,
        G = (color >>  8) & 0xFF,
        B = (color      ) & 0xFF;

    int max = (R > G) ? R : G;

    return (max > B) ? max : B;
}


QImage* watershed(const QImage *origin)
{
    QImage* gradient = imageGradient(origin);

    QImage c = gradient->createMaskFromColor(0xff000000, Qt::MaskInColor);

    for (int i = 1; i < 256; i++)
    {
        ;
    }


}


QImage* selectComponents(const QImage* origin, QList<bool> &componentsActive)
{
    QImage bitmap = origin->convertToFormat(QImage::Format_Mono);

    QImage* componentsMap = new QImage(bitmap.width(), bitmap.height(), QImage::Format_RGB32);
    componentsMap->fill(QColor(0, 0, 0, 0));

    QRgb currColor = 0;

    // Берем 2ю по счету строку и 2й по счету элемент. Граница изображения игнорируется
    QRgb* y1 = (QRgb*)bitmap.scanLine(1);
    QRgb* y2;
    y1++;

    // Список активных маркеров. В последствие, когда будем соединять несколько линий, которая правее будет заменяться
    //на ту, что левее и помечаться в списке неактивной
    // QList<bool> componentsActive;
    componentsActive.clear();

    int  curveNum  = 0,
         curveNum1 = 0;   // В curveNum будет хранится активный номер рядомстоящей точки

    if (*y1 & 0x1)
    {
        componentsMap->setPixel(1, 1, ++currColor);
        componentsActive << true;

        curveNum = curveNum1 = currColor;
    }
    y1++;

    // Маркируем первую строку. Смотрим, стоит ли что слева, для этого curveNum пригодилась
    for (int x = 2; x < bitmap.width() - 1; x++, y1++)
    {
        if (*y1 & 0x1)
        {
            if (curveNum)
                componentsMap->setPixel(x, 1, curveNum);
            else
            {
                componentsMap->setPixel(x, 1, ++currColor);
                componentsActive << true;

                curveNum = currColor;
            }
        }
        else
            curveNum = 0;
    }

    curveNum = curveNum1;

    // Маркируем первый столбец
    for (int y = 2; y < bitmap.height() - 1; y++)
    {
        if (bitmap.pixel(1, y) & 0x1)
        {
            if (curveNum)
                componentsMap->setPixel(1, y, curveNum);
            else
            {
                componentsMap->setPixel(1, y, ++currColor);
                componentsActive << true;

                curveNum = currColor;
            }
        }
        else
            curveNum = 0;
    }

    // Теперь идем по внутренней части
    QRgb *bitmapPixel;

    for (int y = 2; y < bitmap.height() - 1; y++)
    {
        bitmapPixel = (QRgb*)bitmap.scanLine(y);
        bitmapPixel += 2;

        y1 = (QRgb *)componentsMap->scanLine(y - 1);
        y2 = (QRgb *)componentsMap->scanLine(y);
        y1 += 2; y2 += 2;

        // Смотрим связные элементы по строке
        for (int x = 2; x < bitmap.width() - 1; x++, y1++, y2++, bitmapPixel++)
            // Если пиксель не пустой, проверяем его окружение и относим к какой нибудь из областей
            if (*bitmapPixel & 0x1)
            {
                //Если мы тут, то под нами элемент. Осталось определить, к какой группе его определить

                // _______
                // |*| | |
                // _______
                // | |?| |

                curveNum1 = *(y1 - 1) & 0xFFFFFF;
                if (curveNum1)
                {   // Если выбранный пиксель пронумерован, то проверяемый соединяем с ним
                    curveNum = curveNum1;
                    *y2 = curveNum;
                }

                // _______
                // | |*| |
                // _______
                // | |?| |
                curveNum1 = *(y1) & 0xFFFFFF;
                if (curveNum1)
                    if (curveNum != curveNum1 && !curveNum)
                    {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                        componentsActive.replace(curveNum1, false);
                        replaceColor(componentsMap, curveNum1, curveNum);
                    }
                    else
                        curveNum = curveNum1;

                // _______
                // | | | |
                // _______
                // |*|?| |
                //
                // TODO: теоритически, на это можно наложить ограничение, если сверху слева пусто и еще прямо сверху
                curveNum1 = *(y2 - 1) & 0xFFFFFF;
                if (curveNum1)
                    if (curveNum != curveNum1 && !curveNum)
                    {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                        componentsActive.replace(curveNum1, false);
                        replaceColor(componentsMap, curveNum1, curveNum);
                    }
                    else
                        curveNum = curveNum1;

                // этот элемент может быть более новым, чем выше. чтобы не плодить новые
                // _______
                // | | |*|
                // _______
                // | |?| |
                if (x < bitmap.width() - 2)
                {
                    curveNum1 = *(y1 + 1) & 0xFFFFFF;
                    if (curveNum1)
                        if (curveNum != curveNum1 && !curveNum)
                        {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                            componentsActive.replace(curveNum1, false);
                            replaceColor(componentsMap, curveNum1, curveNum);
                        }
                        else
                            curveNum = curveNum1;
                }

                // Если сосед есть
                if (!curveNum)
                    *y2 = curveNum;
                else
                {   // Если соседа нету
                    *y2 = ++currColor;
                    componentsActive << true;
                }
            }
    }

    return componentsMap;
}


void replaceColor(QImage *image, const QRgb colorToReplace, const QRgb newColor)
{
    for (int y = 0; y < image->height(); y++)
    {
        QRgb* xLine = (QRgb*)image->scanLine(y);
        for (int x = 0; x < image->width(); x++, xLine++)
        {
            if (*xLine == colorToReplace)
                *xLine = newColor;
        }
    }
}
