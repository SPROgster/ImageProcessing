#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

// Морфологические операции
QImage *erosion(QImage *origin, const QImage &element, const QColor pixelColor,
                const QColor background = QColor(Qt::white));
QImage *dilation(QImage *origin, const QImage &element, const QColor pixelColor,
                const QColor background = QColor(Qt::white));
// наращивание
/*QImage *opening     (QImage *origin, QImage *element);*/
QImage *closing(QImage *origin, QImage *element, const QColor pixelColor,
                const QColor background = QColor(Qt::white));

// Бинарные операции
/*QImage *unioning    (QImage *A, QImage *B);
QImage *intersection(QImage *A, QImage *B);
QImage *complement  (QImage *A, QImage *B);
*/
// Структурные элементы
QImage *ring        (int radius, const QColor color);
QImage *disk        (int radius, const QColor color);

#endif // MORPHOLOGY_H
