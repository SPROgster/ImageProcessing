#include <QImage>
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
