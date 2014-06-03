#include <QImage>
#include "watershed.h"

QImage* imageGradient(const QImage *origin)
{
    QImage* buffer = new QImage(origin->convertToFormat(QImage::Format_RGB32, Qt::MonoOnly));

    /*QRgb *y1, *y2, *y3,
         *yOut;

    int currR, currG, currB,
        origMin, origMax,
        maskR, maskG, maskB;

    int maskSize = (buffer->width() - 2) * (buffer->height() - 2);

    float min, max,
          currRf, currGf, currBf;

    float *outR, *outG, *outB,
          *R = outR = new float[maskSize],
          *G = outG = new float[maskSize],
          *B = outB = new float[maskSize];

    origMin = 0;
    origMax = 255;

    min =  10000000;
    max = -10000000;

    if (fullSquare)
    {
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
                                                     outR++, outG++, outB++)
            {
                maskR =((*(y1 - 1) & 0xFF0000) + (*(y1 + 0) & 0xFF0000) + (*(y1 + 1) & 0xFF0000) +
                        (*(y2 - 1) & 0xFF0000)                          + (*(y2 + 1) & 0xFF0000) +
                        (*(y3 - 1) & 0xFF0000) + (*(y3 + 0) & 0xFF0000) + (*(y3 + 1) & 0xFF0000) )
                        >> 16;

                maskG =((*(y1 - 1) & 0x00FF00) + (*(y1 + 0) & 0x00FF00) + (*(y1 + 1) & 0x00FF00) +
                        (*(y2 - 1) & 0x00FF00)                          + (*(y2 + 1) & 0x00FF00) +
                        (*(y3 - 1) & 0x00FF00) + (*(y3 + 0) & 0x00FF00) + (*(y3 + 1) & 0x00FF00))
                        >> 8;

                maskB = (*(y1 - 1) & 0x0000FF) + (*(y1 + 0) & 0x0000FF) + (*(y1 + 1) & 0x0000FF) +
                        (*(y2 - 1) & 0x0000FF)                          + (*(y2 + 1) & 0x0000FF) +
                        (*(y3 - 1) & 0x0000FF) + (*(y3 + 0) & 0x0000FF) + (*(y3 + 1) & 0x0000FF);

                currR = (*y2 & 0xFF0000) >> 16;
                currG = (*y2 & 0x00FF00) >> 8;
                currB =  *y2 & 0x0000FF;

                if (currR > origMax) origMax = currR;
                if (currG > origMax) origMax = currG;
                if (currB > origMax) origMax = currB;

                if (currR < origMin) origMin = currR;
                if (currG < origMin) origMin = currG;
                if (currB < origMin) origMin = currB;

                currRf = (A + 8.) * (float)currR - (float)maskR;
                currGf = (A + 8.) * (float)currG - (float)maskG;
                currBf = (A + 8.) * (float)currB - (float)maskB;

                if (currRf > max) max = currRf;
                if (currGf > max) max = currGf;
                if (currBf > max) max = currBf;

                if (currRf < min) min = currRf;
                if (currGf < min) min = currGf;
                if (currBf < min) min = currBf;

                *outR = currRf;
                *outG = currGf;
                *outB = currBf;
            }
        }
    }
    else
    {
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
                                                     outR++, outG++, outB++)
            {
                maskR =(                         (*(y1 + 0) & 0xFF0000)                          +
                        (*(y2 - 1) & 0xFF0000)                          + (*(y2 + 1) & 0xFF0000) +
                                                 (*(y3 + 0) & 0xFF0000)                          )
                        >> 16;

                maskG =(                         (*(y1 + 0) & 0x00FF00)                          +
                        (*(y2 - 1) & 0x00FF00)                          + (*(y2 + 1) & 0x00FF00) +
                                                 (*(y3 + 0) & 0x00FF00)                         )
                        >> 8;

                maskB =                          (*(y1 + 0) & 0x0000FF)                          +
                        (*(y2 - 1) & 0x0000FF)                          + (*(y2 + 1) & 0x0000FF) +
                                                 (*(y3 + 0) & 0x0000FF)                         ;

                currR = (*y2 & 0xFF0000) >> 16;
                currG = (*y2 & 0x00FF00) >> 8;
                currB =  *y2 & 0x0000FF;

                if (currR > origMax) origMax = currR;
                if (currG > origMax) origMax = currG;
                if (currB > origMax) origMax = currB;

                if (currR < origMin) origMin = currR;
                if (currG < origMin) origMin = currG;
                if (currB < origMin) origMin = currB;

                currRf = (A + 4.) * (float)currR - (float)maskR;
                currGf = (A + 4.) * (float)currG - (float)maskG;
                currBf = (A + 4.) * (float)currB - (float)maskB;

                if (currRf > max) max = currRf;
                if (currGf > max) max = currGf;
                if (currBf > max) max = currBf;

                if (currRf < min) min = currRf;
                if (currGf < min) min = currGf;
                if (currBf < min) min = currBf;

                *outR = currRf;
                *outG = currGf;
                *outB = currBf;
            }
        }
    }

    outR = R; outG = G; outB = B;

    float mulCoef = (origMax - origMin) / (max - min);

    for (int y = 1; y < buffer->height() - 1; y++)
    {
        yOut = (QRgb*)buffer->scanLine(y);

        yOut++;

        for (int x = 1; x < buffer->width() - 1; x++,
                                                 yOut++,
                                                 outR++, outG++, outB++)
        {
            currR = (int)( (*outR - min) * mulCoef) + origMin;
            currG = (int)( (*outG - min) * mulCoef) + origMin;
            currB = (int)( (*outB - min) * mulCoef) + origMin;

            *yOut = (currR << 16) + (currG << 8) + currB;
        }
    }

    if (selection == 0)
    {
        ui->imageView->setPixmap(QPixmap::fromImage(*buffer));
        repaint();
    }
    else
    {
        if (selectionBuffer != 0)
            delete selectionBuffer;
        selectionBuffer = new QImage(*buffer);

        selectionPreview();

    }

    delete [] R; delete [] G; delete [] B;
    delete buffer;*/
    return buffer;
}
