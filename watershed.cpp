#include <QImage>
#include <QBitmap>
#include <QPainter>
#include <QSet>
#include <QLabel>
#include <QMessageBox>
#include "watershed.h"
#include "morphology.h"
#include "structuralElements.h"

QImage* imageGradient(const QImage *origin)
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


int hsvValue(QRgb color)
{
    int R = (color >> 16) & 0xFF,
        G = (color >>  8) & 0xFF,
        B = (color      ) & 0xFF;

    int max = (R > G) ? R : G;

    return (max > B) ? max : B;
}


QImage* watershed(const QImage *origin, QLabel* imageDisplay, const int& threshold)
{
    QImage* gradient = imageGradient(origin);
    int width = origin->width();
    int height= origin->height();
    QImage* temp;

    QImage* border = new QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    border->fill(Qt::transparent);

    // Бассейны. Текущий и для предыдущего уровня
    QImage* C = gradientSumm(origin, threshold);
    QImage* Clast = new QImage(C->createMaskFromColor(0x000000, Qt::MaskInColor));
    delete C;
    C = new QImage(Clast->convertToFormat(QImage::Format_ARGB32_Premultiplied));

    // Компоненты связности для n - 1 и n
    int colorNumLast;
    QImage* Qlast = selectComponents(Clast, colorNumLast);
    int colorNum;
    QImage* Q;

    // Структурный элемент
    QImage* structuralElement = square(3, 0xFFFFFFFF);

    // Цвет/уровень воды. Текущее значение градиента
    QRgb color = threshold + 1;
    color += (color << 8) + (color << 16) + 0x000000;

    // Маска с точками с параметром градиента равным color
    QImage* T;

    // Тут мы будем хранить номера компонент, которые пересекает текущий компонент
    QList< QSet<QRgb> > intersectionComponents;
    // Больше чем изначально было, быть не может. Или может?
    intersectionComponents.reserve(colorNumLast);

    for (int colorI = threshold + 1; colorI < 255; colorI++, color += 0x010101)
    {
        T = new QImage(gradient->createMaskFromColor(color, Qt::MaskInColor).convertToFormat(QImage::Format_ARGB32_Premultiplied));
        temp = new QImage(*T);

        T->setAlphaChannel(*temp);
        delete temp;

        // Объединяем С и T
        {
            QPainter painterC(C);
            painterC.save();

            painterC.drawImage(0, 0, *T);

            painterC.restore();
        }

        // Выделяем связные компоненты
        Q = selectComponents(C, colorNum);

        // Расширяем наш список листов
        if (intersectionComponents.size() <= colorNum)
            for (int i = intersectionComponents.size(); i < colorNum; i++)
                intersectionComponents.append(QSet<QRgb>());

        // Очищаем сеты
        for(int i = 0; i < colorNum; i++)
            intersectionComponents[i].clear();


        ////////////////////////////////////////////////////////////////////////////
        ///Определяем сколько связных компонент пересекает данная связная компонента

        // Прямой доступ к массивам компонент
        QRgb* qn    = (QRgb*)Q->scanLine(0);
        QRgb* qlast = (QRgb*)Qlast->scanLine(0);

        QRgb currQ, currQLast;

        // Бегаем по картам компонент, собирая пересечения
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++, qn++, qlast++)
            {
                currQ     = *qn & 0xFFFFFF;
                currQLast = *qlast & 0xFFFFFF;
                if (currQ && currQLast)
                    intersectionComponents[currQ - 1] << currQLast;
            }
        ///
        /////////////////////////////////////////////////////////////////////////////


        // Проверяем, что делать. Ставить ли перегородку и прочее
        // Если у нас по одному пересечению, то ничего не делаем. Это лишь капля в море
        // А если связный компонент пересекает несколько

        for (int q = 0; q < colorNum; q++)
            if (intersectionComponents[q].size() > 1)
            {
                // Выделяем весь бассейн q
                QImage* qSpace = new QImage(Q->createMaskFromColor((q + 1) | 0xFF000000, Qt::MaskInColor).convertToFormat(QImage::Format_ARGB32_Premultiplied));
                temp = new QImage(*qSpace);
                temp->invertPixels();
                qSpace->setAlphaChannel(*temp);
                delete temp;

                // Выделяем бассейны, которые пересекает наш новый бассейн
                QList<QImage> qIntersec;

                QRgb qin;
                foreach(qin, intersectionComponents[q])
                {
                    QImage qadd = Qlast->createMaskFromColor(qin | 0xFF000000, Qt::MaskInColor).convertToFormat(QImage::Format_ARGB32_Premultiplied);
                    qadd.setAlphaChannel(qadd);

                    qIntersec << qadd;
                }

                //////////////////////////////////////////////////////////////////////
                /// Бассейны есть, теперь будем делать морфологию, чтобы различать
                QList<QImage> qIntersecLast;

                bool flag = true;
                while (flag)
                {
                    qIntersecLast.clear();
                    // Морфологически расширяем все бассейны
                    for (QList<QImage>::iterator qin = qIntersec.begin(); qin != qIntersec.end(); qin++)
                    {
                        // Для начала сохраняем предыдущую версию, чтобы было с чем сравнивать
                        qIntersecLast << QImage(*qin);

                        // Порфологически нарасщиваем
                        /// DEBUG тут мы видем, что морфология нарасчивает чуть больше, чем надо бы...
                        QImage* dilationRes = dilation(&(*qin), *structuralElement, QColor(0xFFFFFFFF), QColor(Qt::transparent));

                        // Отрезаем часть не принадлежащую q
                        QImage increased(*dilationRes);

                        QPainter painterAnd(&increased);
                        painterAnd.save();
                        painterAnd.setRenderHint(QPainter::Antialiasing, false);
                        painterAnd.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                        painterAnd.drawImage(0, 0, *qSpace);
                        painterAnd.restore();

                        // Делаем его активным
                        *qin = increased;
                    }

                    // Теперь смотрим на взаимное пересечение
                    for (QList<QImage>::iterator qin1 = qIntersec.begin(); qin1 != qIntersec.end() - 1; qin1++)
                        for (QList<QImage>::iterator qin2 = qin1 + 1; qin2 != qIntersec.end(); qin2++)
                        {
                            QImage crossIntersection(*qin1);

                            {
                                QPainter painterAnd(&crossIntersection);
                                painterAnd.save();
                                painterAnd.setRenderHint(QPainter::Antialiasing, false);
                                painterAnd.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                                painterAnd.drawImage(0, 0, *qin2);
                                painterAnd.restore();
                            }

                            {
                                QPainter borderAdd(border);
                                borderAdd.save();
                                borderAdd.setRenderHint(QPainter::Antialiasing, false);
                                borderAdd.drawImage(0, 0, crossIntersection);
                                borderAdd.restore();
                            }
                        }

                    // Теперь для всех удаляем новую границу
                    for (QList<QImage>::iterator qin = qIntersec.begin(); qin != qIntersec.end(); qin++)
                    {
                        QImage withoutBorder(*qin);

                        QPainter painterAnd(&withoutBorder);
                        painterAnd.save();
                        painterAnd.setRenderHint(QPainter::Antialiasing, false);
                        painterAnd.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                        painterAnd.drawImage(0, 0, *border);
                        painterAnd.restore();

                        *qin = withoutBorder;
                    }

                    int i;
                    for (i = 0; i < qIntersecLast.size() && flag; i++)
                        flag = (qIntersecLast[i] == qIntersec[i]);

                    flag = !flag;
                }
                ///
                //////////////////////////////////////////////////////////////////////

                delete qSpace;

                //////////////////////////////////////////////////////////////////////
                /// Добавляем новую границу в C. Ведь мы градиет использует только для
                /// получения новых T. А по C мы смотрим разделения

                QImage inversedBorder(*border);
                inversedBorder.invertPixels();

                QRgb* borderPixel = (QRgb*)inversedBorder.bits();
                QRgb* cPixel      = (QRgb*)C->bits();
                for (int i = 0; i < C->byteCount() / sizeof(QRgb); i++, borderPixel++, cPixel++)
                {
                    if (*borderPixel & 0xFF000000)
                        *cPixel = *borderPixel;
                }

                //////////////////////////////////////////////////////////////////////
                ///
                ///
                ///     Тут фигня какая-то
                ///
                ///
                //////////////////////////////////////////////////////////////////////
                /*painterC.save();
                painterC.setCompositionMode(QPainter::CompositionMode_SourceOver);
                /// DEBUG тут какая то фигня происходит
                painterC.drawImage(0, 0, inversedBorder);
                painterC.restore();*/
                ///
                //////////////////////////////////////////////////////////////////////
            }

        // Удаляем Т и пересчитываем связные компоненты у C
        delete T;
        delete Qlast;
        Qlast = selectComponents(C, colorNumLast);

        delete Clast;
        Clast = C;
    }

    delete Qlast;
    delete Clast;

    replaceColor(border, 0xFFFFFFFF, 0xFFFF0000);

    return border;
}


QImage* selectComponents(const QImage* origin, int& colorNumber)
{
    QImage bitmap(origin->createMaskFromColor(0xFF000000, Qt::MaskOutColor).convertToFormat(QImage::Format_RGB32));

    QImage* componentsMap = new QImage(bitmap.width(), bitmap.height(), QImage::Format_RGB32);
    componentsMap->fill(QColor(0, 0, 0, 0));

    QRgb currColor = 0xFF000000;

    // Берем 2ю по счету строку и 2й по счету элемент. Граница изображения игнорируется
    QRgb* y1 = (QRgb*)bitmap.scanLine(0);
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

    int width = bitmap.width();
    // Маркируем первую строку. Смотрим, стоит ли что слева, для этого curveNum пригодилась
    for (int x = 1; x < bitmap.width(); x++, y1++, y2++)
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
    for (int y = 1; y < bitmap.height(); y++, y1 += width, y2 += width)
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
    bitmapPixel = (QRgb*)bitmap.scanLine(1);

    y1 = (QRgb *)componentsMap->scanLine(0);
    y2 = (QRgb *)componentsMap->scanLine(1);

    for (int y = 1; y < bitmap.height(); y++)
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
                /*curveNum1 = *(y1 - 1) & 0xFFFFFF;
                if (curveNum1)
                    // Если выбранный пиксель пронумерован, то проверяемый соединяем с ним
                    curveNum = curveNum1;*/

                // _______
                // | |*| |
                // _______
                // | |?| |
                curveNum1 = *y1 & 0xFFFFFF;
                if (curveNum1)
                {
                    /*if (curveNum != curveNum1 && curveNum)
                    {   // TODO сверху 2 ячейки занумерованы. но теоритически, этот if всегда -
                        componentsActive.replace(curveNum1 - 1, false);
                        replaceColor(componentsMap, 0xFF000000 | curveNum1, 0xFF000000 | curveNum);
                    }
                    else*/
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
                /*if (x < bitmap.width() - 2)
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
                }*/

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


void replaceColor(QImage *image, const QRgb colorToReplace, const QRgb newColor)
{
    QRgb* xLine = (QRgb*)image->scanLine(0);
    for (int y = 0; y < image->height(); y++)
        for (int x = 0; x < image->width(); x++, xLine++)
            if (*xLine == colorToReplace)
                *xLine = newColor;
}


QImage *gradientSumm(const QImage *origin, unsigned int treshhold)
{
    QImage* gradient = imageGradient(origin);

    QRgb* pixel = (QRgb*)gradient->bits();
    for (int x = 0; x < gradient->width(); x++)
        for (int y = 0; y < gradient->height(); y++, pixel++)
            if ((*pixel & 0xFF) <= treshhold)
                *pixel &= 0xFF000000;

    return gradient;
}


QImage *displayComponents(const QImage *origin)
{
    int colorNumber;
    QImage* componentsMap = selectComponents(origin, colorNumber);

    float dc = 254./ (colorNumber + 1);

    for (int i = 0; i < colorNumber; i++)
    {
        int newColor = dc * (i + 1);
        newColor = newColor + (newColor << 8) + (newColor << 16);
        QColor clr;
        clr.setHsv(newColor, 255, 255);
        replaceColor(componentsMap, 0xFF000000 + i + 1, clr.rgba());
    }

    return componentsMap;
}
