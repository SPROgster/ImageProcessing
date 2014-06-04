#ifndef WATERSHED_H
#define WATERSHED_H

int hsvValue(QRgb color);
QImage* imageGradient(const QImage* origin);
QImage* watershed(const QImage* origin);

void replaceColor(QImage* image, const QRgb colorToReplace, const QRgb newColor);
QImage* selectComponents(const QImage *origin, QList<bool> &componentsActive);

#endif // WATERSHED_H
