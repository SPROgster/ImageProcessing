#include <QMessageBox>
#include <QPen>
#include <limits>

#include "progressiveCut.h"

void ProgressiveCut::initColorTables()
{
    maskColorTable.clear();
    maskColorTable << 0x00000000;       // 0
    maskColorTable << 0x80FF0000;       // 1

    strokeColorTable.clear();
    // enum strokeType { noStroke, strokeForeground, strokeBackground };
    strokeColorTable << 0xFF000000;     //noStroke;
    strokeColorTable << 0xFFFF0000;     //strokeForeground;
    strokeColorTable << 0xFF00FF00;     //strokeBackground;

    componentsColorTable.clear();
    int mul = 255 / (GMM_K + 1);
    QColor componentsColor;
    for (int i = 0; i < GMM_K; i++)
    {
        componentsColor.setHsv(mul * i, 255, 255);
        componentsColorTable << componentsColor.rgb();
    }
}

int ProgressiveCut::createStrokeMask()
{
    if (!foregroundStroke)
        return 1;

    if (!backgroundStroke)
        return 2;

    if (strokeSelection)
        delete strokeSelection;

    strokeSelection = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    strokeSelection->setColorTable(strokeColorTable);

    uchar* strokeSelectionPixel  = strokeSelection->bits();
    uchar* foregroundPixel = foregroundStroke->bits();
    uchar* backgroundPixel = backgroundStroke->bits();

    for (int i = 0; i < imageSizeInPixels; i++, strokeSelectionPixel++, foregroundPixel++, backgroundPixel++)
    {
        if (*foregroundPixel && *backgroundPixel)
            return 3;

        if (*foregroundPixel)
            *strokeSelectionPixel = strokeForeground;
        else if (*backgroundPixel)
            *strokeSelectionPixel = strokeBackground;
        else
            *strokeSelectionPixel = noStroke;
    }

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*strokeSelection));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "createStrokeMask").exec();
    }

    return 0;
}

void ProgressiveCut::connectNodes(bool update, bool foreground)
{
    // Соединяем пиксели со стоком и истоком
    uchar* forePixel = foregroundStroke->bits();
    uchar* backPixel = backgroundStroke->bits();
    QRgb* imagePixel = (QRgb*)image->bits();
    if (update)
    {
        uchar* prevPixel = foregroundSelection->bits();
        float* interest = getInterestsEnergy(foreground);
        debugIntesets(interest);

        for (int i = 0; i < imageSizeInPixels; i++, backPixel++, forePixel++, imagePixel++, interest++, prevPixel++)
        {
            float back, fore;

            if (*backPixel || ((*prevPixel) && *interest == 0))
            {
                back = infinity;
                fore = 0;
            }
            else if (*forePixel || (!(*prevPixel) && *interest == 0))
            {
                back = 0;
                fore = infinity;
            }
            else
            {
                if (*interest > 0)
                {
                    fore = gmmForeground->p(*imagePixel);
                    fore = -log(fore);
                    //fore = log(fore) / log(fore * back);
                    fore = fore * (*interest) + (1 - *interest) * lastWeights[i].toSource;
                    back = gmmBackground->p(*imagePixel);
                    back = -log(back);
                    //back = log(back) / log(fore * back);
                    back = back * (*interest) + (1 - *interest) * lastWeights[i].toSink;
                }
                else
                {
                    fore = lastWeights[i].toSource;
                    back = lastWeights[i].toSink;
                }

                lastWeights[i].toSource = fore;
                lastWeights[i].toSink   = back;
            }

            graph->set_tweights(nodes[i], fore, back);
        }

        interest -= imageSizeInPixels;
        delete [] interest;

        // Ветки меж пикселями
        imagePixel = (QRgb*)image->bits();
        int i = 0;                  // индекс элемента
        int iRight = 1;             // индекс элемента справо
        int iAbove = imageWidth;    // индекс элемента под выбранным

        for (int y = 0; y < imageHeight - 1; y++)
        {
            for (int x = 0; x < imageWidth - 1; x++, imagePixel++, i++, iRight++, iAbove++)
            {
                Weight &w = lastWeights[i];
                // Вниз влево
                if (x > 0)
                    graph->add_edge(nodes[i], nodes[iAbove - 1], w.BL, w.BL);

                // Вниз
                graph->add_edge(nodes[i], nodes[iAbove], w.BB, w.BB);

                // Вниз вправо
                graph->add_edge(nodes[i], nodes[iAbove + 1], w.BR, w.BR);

                // Вправо
                graph->add_edge(nodes[i], nodes[iRight], w.RR, w.RR);
            }
            Weight &w = lastWeights[i];
            // Вниз влево
            graph->add_edge(nodes[i], nodes[iAbove - 1], w.BL, w.BL);

            // Вниз
            graph->add_edge(nodes[i], nodes[iAbove], w.BB, w.BB);

            imagePixel++; i++; iAbove++; iRight++;
        }
        // Последняя итерация без соединения с низом
        for (int x = 0; x < imageWidth - 1; x++, imagePixel++, i++, iRight++, iAbove++)
        {
            float &w = lastWeights[i].RR;
            graph->add_edge(nodes[i], nodes[iRight], w, w);
        }
    }
    else
    {
        lastWeights.clear();
        for (int i = 0; i < imageSizeInPixels; i++, backPixel++, forePixel++, imagePixel++)
        {
            float back, fore;

            Weight w;
            memset(&w, 0, sizeof(Weight));

            if (*backPixel)
            {
                back = infinity;
                fore = 0;
            }
            else if (*forePixel)
            {
                back = 0;
                fore = infinity;
            }
            else
            {
                float pFore = gmmForeground->p(*imagePixel);
                float pBack = gmmBackground->p(*imagePixel);
                fore = -log(pFore);
                back = -log(pBack);

                w.toSource = fore;
                w.toSink   = back;
            }

            lastWeights << w;

            graph->set_tweights(nodes[i], fore, back);
        }

        // Ветки меж пикселями
        imagePixel = (QRgb*)image->bits();
        float weight;
        int i = 0;                  // индекс элемента
        int iRight = 1;             // индекс элемента справо
        int iAbove = imageWidth;    // индекс элемента под выбранным

        for (int y = 0; y < imageHeight - 1; y++)
        {
            for (int x = 0; x < imageWidth - 1; x++, imagePixel++, i++, iRight++, iAbove++)
            {
                Weight &w = lastWeights[i];
                // Вниз влево
                if (x > 0)
                {
                    weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth - 1), x - 1, y + 1);
                    w.BL = weight;
                    graph->add_edge(nodes[i], nodes[iAbove - 1], weight, weight);
                }

                // Вниз
                weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth), x, y + 1);
                w.BB = weight;
                graph->add_edge(nodes[i], nodes[iAbove], weight, weight);

                // Вниз вправо
                weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth + 1), x + 1, y + 1);
                w.BR = weight;
                graph->add_edge(nodes[i], nodes[iAbove + 1], weight, weight);

                // Вправо
                weight = Bpq(*imagePixel, x, y, *(imagePixel + 1), x + 1, y);
                w.RR = weight;
                graph->add_edge(nodes[i], nodes[iRight], weight, weight);
            }
            Weight &w = lastWeights[i];
            // Вниз влево
            weight = Bpq(*imagePixel, imageWidth - 1, y, *(imagePixel + imageWidth - 1), imageWidth - 2, y + 1);
            w.BL = weight;
            graph->add_edge(nodes[i], nodes[iAbove - 1], weight, weight);

            // Вниз
            weight = Bpq(*imagePixel, imageWidth - 1, y, *(imagePixel + imageWidth), imageWidth - 1, y + 1);
            w.BB = weight;
            graph->add_edge(nodes[i], nodes[iAbove], weight, weight);

            imagePixel++; i++; iAbove++; iRight++;
        }
        // Последняя итерация без соединения с низом
        for (int x = 0; x < imageWidth - 1; x++, imagePixel++, i++, iRight++, iAbove++)
        {
            weight = Bpq(*imagePixel, x, imageHeight, *(imagePixel + 1), x + 1, imageHeight);
            lastWeights[i].RR = weight;
            graph->add_edge(nodes[i], nodes[iRight], weight, weight);
        }

    }
}

bool ProgressiveCut::createGraph()
{
    if (createStrokeMask())
        return false;

    if (nodes)
        delete [] nodes;
    if (graph)
        delete graph;

    graph = new Graph();
    nodes = new Graph::node_id[imageSizeInPixels];
    for (int i = 0; i < imageSizeInPixels; i++)
        nodes[i] = graph->add_node();

    gmmForeground = new GMM(GMM_K);
    gmmBackground = new GMM(GMM_K);

    gmmForeground->loadImage(*image);
    gmmBackground->loadImage(*image);

    gmmForeground->loadPoints(*foregroundStroke);
    gmmBackground->loadPoints(*backgroundStroke);

    gmmBackground->finalize();
    gmmForeground->finalize();

    connectNodes();

    graph->maxflow();

    if (backgroundSelection)
        delete backgroundSelection;
    backgroundSelection = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    backgroundSelection->setColorTable(maskColorTable);
    if (foregroundSelection)
        delete foregroundSelection;
    foregroundSelection = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    foregroundSelection->setColorTable(maskColorTable);

    uchar* strokeSelectionPixel = strokeSelection->bits();
    uchar* backgroundSelectionPixel = backgroundSelection->bits();
    uchar* foregroundSelectionPixel = foregroundSelection->bits();

    for (int i = 0; i < imageSizeInPixels; i++, strokeSelectionPixel++, backgroundSelectionPixel++, foregroundSelectionPixel++)
    {
        if (*strokeSelectionPixel == noStroke)
        {
            bool isForeground = (graph->what_segment(nodes[i]) == Graph::SINK);
            if (isForeground)
            {
                *backgroundSelectionPixel = 0;
                *foregroundSelectionPixel = 1;
            }
            else
            {
                *backgroundSelectionPixel = 1;
                *foregroundSelectionPixel = 0;
            }
        }
        else if (*strokeSelectionPixel == strokeBackground)
        {
            *backgroundSelectionPixel = 1;
            *foregroundSelectionPixel = 0;
        }
        else if (*strokeSelectionPixel == strokeForeground)
        {
            *foregroundSelectionPixel = 1;
            *backgroundSelectionPixel = 0;
        }
    }

    if (selection)
        delete selection;
    selection = new QImage(*image);

    QPainter painter;
    painter.begin(selection);
    painter.drawImage(0, 0, *backgroundSelection);
    painter.end();

    delete [] nodes;
    nodes = 0;
    delete graph;
    graph = 0;

    return true;
}

bool ProgressiveCut::updateGraph(bool foreground)
{
    if (nodes)
        delete [] nodes;
    if (graph)
        delete graph;

    graph = new Graph();
    nodes = new Graph::node_id[imageSizeInPixels];
    for (int i = 0; i < imageSizeInPixels; i++)
        nodes[i] = graph->add_node();

    gmmForeground = new GMM(GMM_K);
    gmmBackground = new GMM(GMM_K);

    gmmForeground->loadImage(*image);
    gmmBackground->loadImage(*image);

    gmmForeground->loadPoints(*foregroundStroke);
    gmmBackground->loadPoints(*backgroundStroke);

    gmmForeground->finalize();
    gmmBackground->finalize();

    connectNodes(true, foreground);

    graph->maxflow();

    if (backgroundSelection)
        delete backgroundSelection;
    backgroundSelection = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    backgroundSelection->setColorTable(maskColorTable);
    foregroundSelection = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    foregroundSelection->setColorTable(maskColorTable);

    uchar* strokeSelectionPixel = strokeSelection->bits();
    uchar* backgroundSelectionPixel = backgroundSelection->bits();
    uchar* foregroundSelectionPixel = foregroundSelection->bits();

    for (int i = 0; i < imageSizeInPixels; i++, strokeSelectionPixel++, backgroundSelectionPixel++, foregroundSelectionPixel++)
    {
        if (*strokeSelectionPixel == noStroke)
        {
            bool isForeground = (graph->what_segment(nodes[i]) == Graph::SINK);
            if (isForeground)
            {
                *backgroundSelectionPixel = 0;
                *foregroundSelectionPixel = 1;
            }
            else
            {
                *backgroundSelectionPixel = 1;
                *foregroundSelectionPixel = 0;
            }
        }
        else if (*strokeSelectionPixel == strokeBackground)
        {
            *backgroundSelectionPixel = 1;
            *foregroundSelectionPixel = 0;
        }
        else if (*strokeSelectionPixel == strokeForeground)
        {
            *foregroundSelectionPixel = 1;
            *backgroundSelectionPixel = 0;
        }
    }

    if (selection)
        delete selection;
    selection = new QImage(*image);

    QPainter painter;
    painter.begin(selection);
    painter.drawImage(0, 0, *backgroundSelection);
    painter.end();

    delete [] nodes;
    nodes = 0;
    delete graph;
    graph = 0;

    return true;
}

xy *ProgressiveCut::getObjectArea(QImage &object)
{
    xy* res = new xy[2];
    res[0].x = imageWidth + 1;
    res[0].y = imageHeight+ 1;
    res[1].x = 0;
    res[1].y = 0;

    uchar* pixel = object.bits();

    for (int y = 0; y < imageHeight; y++)
        for (int x = 0; x < imageWidth; x++, pixel++)
        {
            if (*pixel)
            {
                // Смотрим левый верхний край
                if (y < res[0].y)
                    res[0].y = y;
                if (x < res[0].x)
                    res[0].x = x;

                // Смотрим правый нижний край
                if (y > res[1].y)
                    res[1].y = y;
                if (x > res[1].x)
                    res[1].x = x;
            }
        }

    if (res->x == -1)
    {
        delete [] res;
        return 0;
    }

    return res;
}

float *ProgressiveCut::getDistanseFromStroke(QImage &object)
{
    int x2max = imageWidth - 1;
    int y2max = imageHeight - 1;

    float* result1 = new float[imageWidth * imageHeight];
    float* result2 = new float[imageWidth * imageHeight];
    float* result3 = new float[imageWidth * imageHeight];
    float* result4 = new float[imageWidth * imageHeight];

    uchar* pixel = object.bits();
    for (int i = 0; i < imageSizeInPixels; i++, pixel++)
    {
        if (*pixel)
            result1[i] = result2[i] = result3[i] = result4[i] = 0;
        else
            result1[i] = result2[i] = result3[i] = result4[i] = infinity;
    }

    float sqrt2 = sqrt(2.);
    float many = (float)imageSizeInPixels;

    xy* strokeCrop = getObjectArea(object);
    // В прямом направлении
    {
        int x1 = strokeCrop[0].x;
        int y1 = strokeCrop[0].y;
        int x2 = strokeCrop[1].x;
        int y2 = strokeCrop[1].y;

        bool somethingToChange = true;
        while (somethingToChange)
        {
            somethingToChange = false;

            if (x1 > 0)
                x1--;
            if (y1 > 0)
                y1--;
            if (x2 < x2max)
                x2++;
            if (y2 < y2max)
                y2++;

            for (int y = y1; y <= y2; y++)
            {
                float *y_2 = result1 + imageWidth * y + x1;
                float *y_1 = y_2 - imageWidth;
                float *y_3 = y_2 + imageWidth;

                for (int x = x1; x <= x2; x++, y_1++, y_2++, y_3++)
                {
                    if (*y_2 > many) // это означает, что мы стоим на бесконечности
                    {
                        float minDistance = infinity;

                        bool yTop   = (y > 0);
                        bool yButtom= (y < y2max);
                        bool xLeft  = (x > 0);
                        bool xRight = (x < x2max);
                        float curr;

                        // Смотрим по часовой стрелке

                        if (yButtom)
                            if ((curr = *y_3 + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yTop)
                            if ((curr = *y_1 + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (xRight)
                            if ((curr = *(y_2 + 1) + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (xLeft)
                            if ((curr = *(y_2 - 1) + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yButtom && xLeft)
                            if ((curr = *(y_3 - 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yButtom && xRight)
                            if ((curr = *(y_3 + 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yTop && xLeft)
                            if ((curr = *(y_1 - 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yTop && xRight)
                            if ((curr = *(y_1 + 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;


                        if (minDistance < many)
                        {
                            somethingToChange = true;
                            *y_2 = minDistance;
                        }
                    }
                }
            }
        }
    }
    // В обратном направлении
    {
        int x1 = strokeCrop[0].x;
        int y1 = strokeCrop[0].y;
        int x2 = strokeCrop[1].x;
        int y2 = strokeCrop[1].y;

        bool somethingToChange = true;
        while (somethingToChange)
        {
            somethingToChange = false;

            if (x1 > 0)
                x1--;
            if (y1 > 0)
                y1--;
            if (x2 < x2max)
                x2++;
            if (y2 < y2max)
                y2++;

            for (int y = y2; y >= y1; y--)
            {
                float *y_2 = result2 + imageWidth * y + x2;
                float *y_1 = y_2 - imageWidth;
                float *y_3 = y_2 + imageWidth;

                for (int x = x2; x >= x1; x--, y_1--, y_2--, y_3--)
                {
                    if (*y_2 > many) // это означает, что мы стоим на бесконечности
                    {
                        float minDistance = infinity;

                        bool yTop   = (y > 0);
                        bool yButtom= (y < y2max);
                        bool xLeft  = (x > 0);
                        bool xRight = (x < x2max);
                        float curr;

                        // Смотрим по часовой стрелке

                        if (yButtom)
                            if ((curr = *y_3 + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yTop)
                            if ((curr = *y_1 + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (xRight)
                            if ((curr = *(y_2 + 1) + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (xLeft)
                            if ((curr = *(y_2 - 1) + 1) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yButtom && xLeft)
                            if ((curr = *(y_3 - 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yButtom && xRight)
                            if ((curr = *(y_3 + 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yTop && xLeft)
                            if ((curr = *(y_1 - 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;
                        if (yTop && xRight)
                            if ((curr = *(y_1 + 1) + sqrt2) < many)
                                if (curr < minDistance)
                                    minDistance = curr;


                        if (minDistance < many)
                        {
                            somethingToChange = true;
                            *y_2 = minDistance;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < imageSizeInPixels; i++)
    {
        result1[i] = (result1[i] < result2[i]) ? result1[i] : result2[i];
    }

    delete [] strokeCrop;

    delete [] result2;
    delete [] result3;
    delete [] result4;

    return result1;
}

void ProgressiveCut::distanceDebug()
{
    if (imageOutput)
    {
        float* forDel;
        float* interestsEnergy = forDel = getDistanseFromStroke(*strokeSelectionNew);

        float max = 0;
        for (int i = 0; i < imageSizeInPixels; i++, interestsEnergy++)
        {
            if (*interestsEnergy > max)
                max = *interestsEnergy;
        }

        QImage debug(imageWidth, imageHeight, QImage::Format_RGB32);
        QRgb* pixel = (QRgb*)debug.bits();

        interestsEnergy = forDel;
        for (int i = 0; i < imageSizeInPixels; i++, pixel++, interestsEnergy++)
        {
            QColor c;
            *interestsEnergy /= max;
            *interestsEnergy = 1 - *interestsEnergy;
            c.setRgbF(*interestsEnergy, *interestsEnergy, *interestsEnergy);
            *pixel = c.rgb();
        }

        QPainter paint;
        paint.begin(&debug);
        paint.drawImage(0, 0, strokeSelectionNew->convertToFormat(QImage::Format_ARGB32_Premultiplied));
        paint.end();

        imageOutput->setPixmap(QPixmap::fromImage(debug));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "interestsEnergy").exec();

        delete forDel;
    }
}

float *ProgressiveCut::getInterestsEnergy(bool isForeground)
{
    float* interestsEnergy = getDistanseFromStroke(*strokeSelectionNew);
    float* iter = interestsEnergy;
    uchar* maskPixel = foregroundSelection->bits();

    float maxDistance = 0;
    for (int i = 0; i < imageSizeInPixels; i++)
    {
        if (iter[i] > maxDistance)
            maxDistance = iter[i];
    }

    for (int i = 0; i < imageSizeInPixels; i++, iter++, maskPixel++)
    {
        if ((!isForeground && *maskPixel) || (isForeground && !(*maskPixel)))
        {
            // Если тут, то идет смена типа выделения (фон помечен объектом)
            *iter = (maxDistance - *iter / progressiveCutR) / (maxDistance);
            if (*iter < 0)
                *iter = 0;
        }
            //*iter = progressiveCutR / (*iter + progressiveCutR); // TODO: но это не так
        else
            // А тут фон не надо менять
            *iter = 0;
    }

    return interestsEnergy;
}

void ProgressiveCut::debugIntesets(float* interestsEnergy)
{
    if (imageOutput)
    {
        QImage debug(imageWidth, imageHeight, QImage::Format_RGB32);
        QRgb* pixel = (QRgb*)debug.bits();

        for (int i = 0; i < imageSizeInPixels; i++, pixel++, interestsEnergy++)
        {
            QColor c;
            c.setRgbF(*interestsEnergy, *interestsEnergy, *interestsEnergy);
            *pixel = c.rgb();
        }

        imageOutput->setPixmap(QPixmap::fromImage(debug));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "interestsEnergy").exec();
    }
}

void ProgressiveCut::setImageOutput(QLabel *imageView)
{
    imageOutput = imageView;
}

float ProgressiveCut::Bpq(const QRgb a, const int xa, const int ya, const QRgb b, const int xb, const int yb)
{
    RgbColor aF, bF;
    QRgb aCopy = a;
    QRgb bCopy = b;

    aF.B    = aCopy & 0xFF;
    bF.B    = bCopy & 0xFF;
    aCopy   >>= 8;
    bCopy   >>= 8;

    aF.G    = aCopy & 0xFF;
    bF.G    = bCopy & 0xFF;
    aCopy   >>= 8;
    bCopy   >>= 8;

    aF.R    = aCopy & 0xFF;
    bF.R    = bCopy & 0xFF;

    return 1 / (norma2(aF, bF) + 1) * distance(xa, ya, xb, yb);
    //return exp(-0.5 * norma2(aF, bF) / (delta * delta)) / distance(xa, ya, xb, yb);
}

ProgressiveCut::ProgressiveCut()
{
    graph = 0;
    nodes = 0;
    image = 0;

    foregroundSelection = 0;
    backgroundSelection = 0;
    strokeSelection = 0;
    strokeSelectionNew = 0;
    foregroundStroke = 0;
    backgroundStroke = 0;

    imageWidth        = 0;
    imageHeight       = 0;
    imageSizeInPixels = 0;

    initColorTables();

    infinity = std::numeric_limits<float>::infinity();

    gmmForeground = 0;
    gmmBackground = 0;
    components = 0;

    imageOutput = 0;

    selection = 0;
}

ProgressiveCut::ProgressiveCut(const QImage &imageToCut)
{
    graph = 0;
    nodes = 0;
    image = new QImage(imageToCut);

    foregroundSelection = 0;
    backgroundSelection = 0;
    strokeSelection = 0;
    strokeSelectionNew = 0;
    foregroundStroke = 0;
    backgroundStroke = 0;

    imageWidth = image->width();
    imageHeight= image->height();
    imageSizeInPixels = imageWidth * imageHeight;

    nodes = new Graph::node_id[imageSizeInPixels];

    initColorTables();

    components = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    components->setColorTable(componentsColorTable);

    infinity = std::numeric_limits<float>::infinity();

    gmmForeground = 0;
    gmmBackground = 0;

    imageOutput = 0;

    selection = 0;
}

ProgressiveCut::~ProgressiveCut()
{
    if (graph)
        delete graph;
    if (image)
        delete image;

    if (foregroundSelection)
        delete foregroundSelection;
    if (backgroundSelection)
        delete backgroundSelection;

    if (foregroundStroke)
        delete foregroundStroke;
    if (backgroundStroke)
        delete backgroundStroke;
    if (strokeSelectionNew)
        delete strokeSelectionNew;

    if (components)
        delete components;

    if (gmmForeground)
        delete gmmForeground;
    if (gmmBackground)
        delete gmmBackground;

    if (selection)
        delete selection;

    lastWeights.clear();
}

void ProgressiveCut::setImage(const QImage &imageToCut)
{
    if (image)
        delete image;

    image = new QImage(imageToCut);

    imageWidth = image->width();
    imageHeight= image->height();
    imageSizeInPixels = imageWidth * imageHeight;

    nodes = new Graph::node_id[imageSizeInPixels];

    components = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    components->setColorTable(componentsColorTable);
}

bool ProgressiveCut::setForeground(const QImage &foreground)
{
    if (!image)
        return false;

    if (foreground.width() != imageWidth || foreground.height() != imageHeight)
        return false;

    if (foregroundStroke)
        delete foregroundStroke;

    foregroundStroke = new QImage(foreground.convertToFormat(QImage::Format_Indexed8, maskColorTable));

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*foregroundStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "setForeground").exec();
    }

    return true;
}

bool ProgressiveCut::setBackground(const QImage &background)
{
    if (!image)
        return false;

    if (background.width() != imageWidth || background.height() != imageHeight)
        return false;

    if (backgroundStroke)
        delete backgroundStroke;

    backgroundStroke = new QImage(background.convertToFormat(QImage::Format_Indexed8, maskColorTable));

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*backgroundStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "setBackground").exec();
    }

    return true;
}

bool ProgressiveCut::updateForeground(const QImage &foreground)
{
    if (!image)
        return false;

    if (foreground.width() != imageWidth || foreground.height() != imageHeight)
        return false;

    if (!strokeSelection)
        return false;

    QImage foregroundMask(foreground.convertToFormat(QImage::Format_Indexed8, maskColorTable));

    if (strokeSelectionNew)
        delete strokeSelectionNew;
    strokeSelectionNew = new QImage(foregroundMask.copy());

    uchar* foregroundPixel = foregroundMask.bits();
    uchar* strokePixel = foregroundStroke->bits();
    uchar* strokesPixel = strokeSelection->bits();

    for (int i = 0; i < imageSizeInPixels; i++, foregroundPixel++, strokePixel++, strokesPixel++)
    {
        if (*foregroundPixel)
        {
            *strokePixel = 1;
            *strokesPixel = strokeForeground;
        }
    }

    /*if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*foregroundStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "updateForeground").exec();
    }*/

    return true;
}

bool ProgressiveCut::updateBackground(const QImage &background)
{
    if (!image)
        return false;

    if (background.width() != imageWidth || background.height() != imageHeight)
        return false;

    if (!strokeSelection)
        return false;

    QImage backgroundMask(background.convertToFormat(QImage::Format_Indexed8, maskColorTable));

    if (strokeSelectionNew)
        delete strokeSelectionNew;
    strokeSelectionNew = new QImage(backgroundMask.copy());

    uchar* backgroundPixel = backgroundMask.bits();
    uchar* strokePixel = backgroundStroke->bits();
    uchar* strokesPixel = strokeSelection->bits();

    for (int i = 0; i < imageSizeInPixels; i++, backgroundPixel++, strokePixel++, strokesPixel++)
    {
        if (*backgroundPixel)
        {
            *strokePixel = 1;
            *strokesPixel = strokeBackground;
        }
    }

    /*if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*backgroundStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "updateBackground").exec();
    }*/

    return true;
}

void ProgressiveCut::deleteUpdation()
{
    if (strokeSelectionNew)
        delete strokeSelectionNew;
}
