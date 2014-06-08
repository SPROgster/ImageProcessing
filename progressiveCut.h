#ifndef PROGRESSIVECUT_H
#define PROGRESSIVECUT_H

#define GMM_K 5

#include <QImage>
#include <QVector>
#include <QLabel>       // Для отладки
#include <QPixmap>      // Для отладки
#include <math.h>

#include "graph.h"
#include "gmm.h"

class ProgressiveCut
{
private:
    Graph* graph;
    QImage* image;
    QImage* foregroundSelection;
    QImage* backgroundSelection;
    QImage* strokeSelection;

    QVector<QRgb> maskColorTable;
    QVector<QRgb> strokeColorTable;
    QVector<QRgb> componentsColorTable;

    int imageWidth, imageHeight, imageSizeInPixels;
    float infinity;

    void initColorTables();

    // Две маски вливаются в одну
    int createStrokeMask();
    enum strokeType { noStroke, strokeForeground, strokeBackground };

    //GMM
    GMM* gmmBackground;
    GMM* gmmForeground;
    QImage* components;

    //Debugging
    QLabel* imageOutput;

public:
    ProgressiveCut();
    ProgressiveCut(const QImage& imageToCut);
    ~ProgressiveCut();

    void setImage(const QImage& imageToCut);
    bool setForeground(const QImage& foreground);
    bool setBackground(const QImage& background);

    bool updateForeground(const QImage& foreground);
    bool updateBackground(const QImage& background);

    bool createGraph();
    void updateGraph();

    void setImageOutput(QLabel* imageView);
};

float
Bpq(const QColor a, const int xa, const int ya, const QColor b, const int xb, const int yb, const float delta);

inline float
distance(int x1, int y1, int x2, int y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

inline float
norma2(const QColor a, const QColor b)
{
    int red  = a.red()   - b.red();
    int green= a.green() - b.green();
    int blue = a.blue()  - b.blue();

    return (red * red + green * green + blue * blue);
}

#endif // PROGRESSIVECUT_H
