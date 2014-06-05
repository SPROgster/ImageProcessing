#include "mainwindow.h"

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



QImage *square(int a, const QColor color)
{
    QImage* square = new QImage(a, a, QImage::Format_ARGB32);

    square->fill(color);

    return square;
}
