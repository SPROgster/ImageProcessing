#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

// Морфологические операции
QImage *erosion(QImage *origin, const QImage &element, const QColor pixelColor,
                const QColor background = QColor(Qt::transparent));
QImage *dilation(QImage *origin, const QImage &element, const QColor pixelColor,
                const QColor background = QColor(Qt::transparent));
// наращивание
/*QImage *opening     (QImage *origin, QImage *element);*/
QImage *closing(QImage *origin, QImage *element, const QColor pixelColor,
                const QColor background = QColor(Qt::white));

// Бинарные операции
/*QImage *unioning    (QImage *A, QImage *B);
QImage *intersection(QImage *A, QImage *B);
QImage *complement  (QImage *A, QImage *B);
*/

#endif // MORPHOLOGY_H