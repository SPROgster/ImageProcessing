#ifndef WATERSHED_H
#define WATERSHED_H

int hsvValue(QRgb color);
QImage* imageGradient(const QImage* origin);
QImage* gradientSumm(const QImage* origin, int treshhold);
QImage* watershed(const QImage* origin, const int &threshold = 0);

void replaceColor(QImage* image, const QRgb colorToReplace, const QRgb newColor);

QImage* selectComponents(const QImage *origin, int &colorNumber);
QImage* displayComponents(const QImage *origin);

#endif // WATERSHED_H
