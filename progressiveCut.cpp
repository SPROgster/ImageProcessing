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

void ProgressiveCut::connectNodes()
{
    // Соединяем пиксели со стоком и истоком
    uchar* forePixel = foregroundStroke->bits();
    uchar* backPixel = backgroundStroke->bits();
    QRgb* imagePixel = (QRgb*)image->bits();

    for (int i = 0; i < imageSizeInPixels; i++, backPixel++, forePixel++, imagePixel++)
    {
        float back, fore;

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
            back = -log(gmmBackground->p(*imagePixel));
            fore = -log(gmmForeground->p(*imagePixel));
        }

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
            // Вниз влево
            if (x > 0)
            {
                weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth - 1), x - 1, y + 1);
                graph->add_edge(nodes[i], nodes[iAbove - 1], weight, weight);
            }
            // Вниз
            weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth), x, y + 1);
            graph->add_edge(nodes[i], nodes[iAbove], weight, weight);
            // Вниз вправо
            weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth + 1), x + 1, y + 1);
            graph->add_edge(nodes[i], nodes[iAbove + 1], weight, weight);
            // Вправо
            weight = Bpq(*imagePixel, x, y, *(imagePixel + 1), x + 1, y);
            graph->add_edge(nodes[i], nodes[iRight], weight, weight);
        }
        weight = Bpq(*imagePixel, imageWidth - 1, y, *(imagePixel + imageWidth - 1), imageWidth - 2, y + 1);
        graph->add_edge(nodes[i], nodes[iAbove - 1], weight, weight);
        // Вниз
        weight = Bpq(*imagePixel, imageWidth - 1, y, *(imagePixel + imageWidth), imageWidth - 1, y + 1);
        graph->add_edge(nodes[i], nodes[iAbove], weight, weight);

        imagePixel++; i++; iAbove++; iRight++;
    }
    // Последняя итерация без соединения с низом
    for (int x = 0; x < imageWidth - 1; x++, imagePixel++, i++, iRight++, iAbove++)
    {
        weight = Bpq(*imagePixel, x, imageHeight, *(imagePixel + 1), x + 1, imageHeight);
        graph->add_edge(nodes[i], nodes[iRight], weight, weight);
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

bool ProgressiveCut::updateGraph(QVector<xy> &cursorWay, bool foreground, int strokeSize)
{
    createLinesFromXy(cursorWay, strokeSize);
    return createGraph();
}

void ProgressiveCut::createLinesFromXy(QVector<xy> &cursorWay, int strokeSize)
{
    QPainter painter;
    QPainter painterAreaMask;

    QImage* newStroke = new QImage(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);
    newStroke->setColorTable(maskColorTable);
    newStroke->fill(0);
    QImage* areaMask = new QImage(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);
    areaMask->setColorTable(maskColorTable);
    areaMask->fill(0);

    painter.begin(newStroke);
    QPen strokePen;
    strokePen.setColor(QColor(maskColorTable[1]));
    strokePen.setWidth(strokeSize);
    strokePen.setCapStyle(Qt::RoundCap);
    painter.setPen(strokePen);
    painterAreaMask.begin(areaMask);
    QPen areaPen;
    areaPen.setColor(QColor(maskColorTable[1]));
    areaPen.setWidth(strokeSize);
    areaPen.setCapStyle(Qt::RoundCap);
    painterAreaMask.setPen(areaPen);
    painterAreaMask.setPen(QPen(QBrush(QColor(maskColorTable[1])), strokeSize * 5));

    QVector<xy>::iterator iter = cursorWay.begin();
    xy lastPos = *iter;

    for(iter++; iter != cursorWay.end(); iter++)
    {
        painter.drawLine(lastPos.x, lastPos.y, iter->x, iter->y);
        painterAreaMask.drawLine(lastPos.x, lastPos.y, iter->x, iter->y);
        lastPos = *iter;
    }

    painter.end();
    painterAreaMask.end();

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*newStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "newStoke").exec();
    }

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*areaMask));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "areaMask").exec();
    }

    delete newStroke;
    delete areaMask;
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

    if (components)
        delete components;

    if (gmmForeground)
        delete gmmForeground;
    if (gmmBackground)
        delete gmmBackground;

    if (selection)
        delete selection;
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

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*foregroundStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "updateForeground").exec();
    }

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

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*backgroundStroke));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "updateBackground").exec();
    }

    return true;
}

void ProgressiveCut::deleteUpdation()
{
    if (strokeSelectionNew)
        delete strokeSelectionNew;
}
