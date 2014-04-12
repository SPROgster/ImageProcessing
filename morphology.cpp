#include "mainwindow.h"
#include "ui_mainwindow.h"

// Морфологические операции
QImage* MainWindow::
erosion(QImage *origin, const QImage &element, const QColor pixelColor,
         const QColor background)
{
    int width       = origin->width(),
        height      = origin->height(),
        elementSizeX= element.width(),
        elementSizeY= element.height(),
        elementRadX = elementSizeX / 2,
        elementRadY = elementSizeY / 2,
        xSumStart, ySumStart,
        xSumEnd,   ySumEnd,
        xRect,     yRect;

    bool flag;

    QImage *erosionResult = new QImage(width, height, origin->format());
    erosionResult->fill(background);

    QPainter painter;

    QColor pixelColor1(pixelColor);
    pixelColor1.setAlpha(255);

    painter.begin(erosionResult);
    for (int x = 0; x < width; x++)
    {
        xRect       = x - elementRadX;

        if (xRect > 0)
            xSumStart = 0;
        else
            xSumStart = -xRect;

        xSumEnd     = width - x - elementRadX - 2;
        if (xSumEnd > 0)
            xSumEnd = elementSizeX;
        else
            xSumEnd+= elementSizeX;

        for (int y = 0; y < height; y++)
        {
            yRect       = y - elementRadY;

            if (yRect > 0)
                ySumStart = 0;
            else
                ySumStart = -yRect;

            ySumEnd     = height - y + elementRadY - 2;
            if (ySumEnd > 0)
                ySumEnd = elementSizeY;
            else
                ySumEnd+= elementSizeY;

            // Проверяем, пиксель
            flag = true;
            for (int x1 = xSumStart; (x1 < xSumEnd) && flag; x1++)
                for (int y1 = ySumStart; (y1 < ySumEnd) && flag; y1++)
                    if ((element.pixel(x1, y1) >> 8 * 3) > 0 )
                    {
                        QColor color(origin->pixel(xRect + x1, yRect + y1));
                        flag = (color == pixelColor1);
                    }

            if (flag)
                painter.drawImage(xRect, yRect, element);
        }
    }
    painter.end();

    return erosionResult;
}

QImage* MainWindow::
dilation(QImage *origin, const QImage &element, const QColor pixelColor,
          const QColor background)
{
    int width       = origin->width(),
        height      = origin->height(),
        elementSizeX= element.width(),
        elementSizeY= element.height(),
        elementRadX = elementSizeX / 2,
        elementRadY = elementSizeY / 2;


    QImage *delationResult = new QImage(width, height, origin->format());

    delationResult->fill(background);

    QPainter painter;
    QColor pixelColor1(pixelColor);

    painter.begin(delationResult);

    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
        {
            QColor color(origin->pixel(x, y));
            // Проверяем, пиксель
            if (color == pixelColor1)
                painter.drawImage(x - elementRadX,
                                  y - elementRadY,
                                  element);
        }

    painter.end();

    return delationResult;
}
/*
QImage *opening     (QImage *origin, QImage *element)
{

}
*/
QImage* MainWindow::
closing(QImage *origin, QImage *element, const QColor pixelColor,
                const QColor background)
{
    QImage *tmpResult = dilation(origin, *element, pixelColor, background);

    QImage *result = erosion(tmpResult, *element, pixelColor, background);

    delete tmpResult;

    return result;
}
/*
// Бинарные операции
QImage *unioning    (QImage *A, QImage *B)
{

}

QImage *intersection(QImage *A, QImage *B)
{

}

QImage *complement  (QImage *A, QImage *B)
{

}*/


// Структурные элементы
QImage*  MainWindow::
ring        (int radius, const QColor color)
{
    int size = radius * 2 - 1;
    QImage *ring = new QImage(size, size, QImage::Format_ARGB32);

    ring->fill(Qt::transparent);

    QPainter painter;

    painter.begin(ring);
    painter.setPen(color);

    painter.drawEllipse(0, 0, size, size);

    painter.end();

    return ring;
}

QImage* MainWindow::
disk        (int radius, const QColor color)
{
    int size = radius * 2 - 1;
    QImage *disk = new QImage(radius * 2 - 1, radius * 2 - 1, QImage::Format_ARGB32);

    disk->fill(Qt::transparent);

    QPainter painter;

    painter.begin(disk);
    painter.setPen(color);
    painter.setBrush(QColor(color));

    painter.drawEllipse(0, 0, size, size);

    painter.end();

    return disk;
}
