#include <QMessageBox>
#include <limits>

#include "progressiveCut.h"

void ProgressiveCut::initColorTables()
{
    maskColorTable.clear();
    maskColorTable << 0xFF000000;       // 0
    maskColorTable << 0xFFFFFFFF;       // 1

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
    if (!foregroundSelection)
        return 1;

    if (!backgroundSelection)
        return 2;

    if (strokeSelection)
        delete strokeSelection;

    strokeSelection = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    strokeSelection->setColorTable(strokeColorTable);

    uchar* strokeSelectionPixel  = strokeSelection->bits();
    uchar* foregroundPixel = foregroundSelection->bits();
    uchar* backgroundPixel = backgroundSelection->bits();

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

bool ProgressiveCut::createGraph()
{
    if (graph)
        delete graph;

    if (createStrokeMask())
        return false;

    graph = new Graph(imageWidth, imageHeight);

    gmmForeground = new GMM(GMM_K);
    gmmBackground = new GMM(GMM_K);

    buildGMMs(*gmmBackground, *gmmForeground, *components, *image, *strokeSelection);

    // Соединяем пиксели со стоком и истоком
    uchar* forePixel = foregroundSelection->bits();
    uchar* backPixel = backgroundSelection->bits();
    QRgb* imagePixel = (QRgb*)image->bits();
    for (int y = 0; y < imageHeight; y++)
    {
        for (int x = 0; x < imageWidth; x++, backPixel++, forePixel++, imagePixel++)
        {
            float toBegin, toEnd;

            if (*backPixel)
            {
                toBegin = infinity;
                toEnd = 0;
            }
            else if (*forePixel)
            {
                toBegin = 0;
                toEnd = infinity;
            }
            else
            {
                toBegin = -log(gmmBackground->p(*imagePixel));
                toEnd   = -log(gmmForeground->p(*imagePixel));
            }

            graph->weightToBeginAndEnd(x, y, toBegin, toEnd);
        }
    }

    // Ветки меж пикселями
    imagePixel = (QRgb*)image->bits();
    float weight;
    for (int y = 0; y < imageHeight - 1; y++)
    {
        for (int x = 0; x < imageWidth - 1; x++, imagePixel++)
        {
            // Вниз
            weight = Bpq(*imagePixel, x, y, *(imagePixel + imageWidth), x, y + 1, sigma);
            graph->weightTo(x, y, x, y + 1, weight);
            // Вправо
            weight = Bpq(*imagePixel, x, y, *(imagePixel + 1), x + 1, y, sigma);
            graph->weightTo(x, y, x + 1, y, weight);

        }
        // Вниз
        weight = Bpq(*imagePixel, imageWidth - 1, y, *(imagePixel + imageWidth), imageWidth - 1, y + 1, sigma);
        graph->weightTo(imageWidth - 1, y, imageWidth - 1, y + 1, weight);
        imagePixel++;
    }
    // Последняя итерация без соединения с низом
    for (int x = 0; x < imageWidth - 1; x++, imagePixel++)
    {
        weight = Bpq(*imagePixel, x, imageHeight, *(imagePixel + 1), x + 1, imageHeight, sigma);
        graph->weightTo(x, imageHeight, x + 1, imageHeight, weight);
    }

    return true;
}

void ProgressiveCut::updateGraph()
{

}

void ProgressiveCut::setImageOutput(QLabel *imageView)
{
    imageOutput = imageView;
}

float ProgressiveCut::Bpq(const QRgb a, const int xa, const int ya, const QRgb b, const int xb, const int yb, const float delta)
{
    RgbF aF, bF;
    QRgb aCopy = a;
    QRgb bCopy = b;

    aF.blueF    = aCopy & 0xFF;
    bF.blueF    = bCopy & 0xFF;
    aCopy >>= 8;
    bCopy >>= 8;
    aF.greenF   = aCopy & 0xFF;
    bF.greenF   = bCopy & 0xFF;
    aCopy >>= 8;
    bCopy >>= 8;
    aF.redF     = aCopy & 0xFF;
    bF.redF     = bCopy & 0xFF;

    return exp(-0.5 * norma2(aF, bF) / delta) / distance(xa, ya, xb, yb);
}

ProgressiveCut::ProgressiveCut()
{
    graph = 0;
    image = 0;

    foregroundSelection = 0;
    backgroundSelection = 0;
    strokeSelection = 0;

    imageWidth        = 0;
    imageHeight       = 0;
    imageSizeInPixels = 0;

    initColorTables();

    infinity = std::numeric_limits<float>::infinity();

    gmmForeground = 0;
    gmmBackground = 0;
    components = 0;

    imageOutput = 0;
}

ProgressiveCut::ProgressiveCut(const QImage &imageToCut)
{
    graph = 0;
    image = new QImage(imageToCut);

    foregroundSelection = 0;
    backgroundSelection = 0;
    strokeSelection = 0;

    imageWidth = image->width();
    imageHeight= image->height();
    imageSizeInPixels = imageWidth * imageHeight;

    initColorTables();

    components = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    components->setColorTable(componentsColorTable);

    infinity = std::numeric_limits<float>::infinity();

    gmmForeground = 0;
    gmmBackground = 0;

    imageOutput = 0;
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

    if (components)
        delete components;

    if (gmmForeground)
        delete gmmForeground;
    if (gmmBackground)
        delete gmmBackground;
}

void ProgressiveCut::setImage(const QImage &imageToCut)
{
    if (image)
        delete image;

    image = new QImage(imageToCut);

    imageWidth = image->width();
    imageHeight= image->height();
    imageSizeInPixels = imageWidth * imageHeight;

    components = new QImage(imageWidth, imageHeight, QImage::Format_Indexed8);
    components->setColorTable(componentsColorTable);
}

bool ProgressiveCut::setForeground(const QImage &foreground)
{
    if (!image)
        return false;

    if (foreground.width() != imageWidth)
        return false;

    if (foregroundSelection)
        delete foregroundSelection;

    foregroundSelection = new QImage(foreground.convertToFormat(QImage::Format_Indexed8, maskColorTable));

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*foregroundSelection));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "createStrokeMask").exec();
    }

    return true;
}

bool ProgressiveCut::setBackground(const QImage &background)
{
    if (!image)
        return false;

    if (background.width() != imageWidth)
        return false;

    if (backgroundSelection)
        delete backgroundSelection;

    backgroundSelection = new QImage(background.convertToFormat(QImage::Format_Indexed8, maskColorTable));

    if (imageOutput)
    {
        imageOutput->setPixmap(QPixmap::fromImage(*backgroundSelection));
        QMessageBox(QMessageBox::NoIcon, "Отладка", "createStrokeMask").exec();
    }

    return true;
}

bool ProgressiveCut::updateForeground(const QImage &foreground)
{
    return true;
}

bool ProgressiveCut::updateBackground(const QImage &background)
{
    return true;
}
