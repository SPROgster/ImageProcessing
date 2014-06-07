#include <QMessageBox>

#include "progressiveCut.h"

void ProgressiveCut::initMaskColorTable()
{
    maskColorTable.clear();
    maskColorTable << 0;
    maskColorTable << 0xFFFFFF;
}

void ProgressiveCut::createGraph()
{
    if (graph)
        delete graph;

    graph = new Graph(imageWidth, imageHeight);


}

ProgressiveCut::ProgressiveCut()
{
    graph = 0;
    image = 0;

    foregroundSelection = 0;
    backgroundSelection = 0;

    imageWidth = 0;
    imageHeight= 0;

    initMaskColorTable();
}

ProgressiveCut::ProgressiveCut(const QImage &imageToCut)
{
    graph = 0;
    image = new QImage(imageToCut);

    foregroundSelection = 0;
    backgroundSelection = 0;

    imageWidth = image->width();
    imageHeight= image->height();

    initMaskColorTable();
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
}

void ProgressiveCut::setImage(const QImage &imageToCut)
{
    if (image)
        delete image;

    image = new QImage(imageToCut);

    imageWidth = image->width();
    imageHeight= image->height();
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

    return true;
}
