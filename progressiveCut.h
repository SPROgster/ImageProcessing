#ifndef PROGRESSIVECUT_H
#define PROGRESSIVECUT_H

#define GMM_K 8
const float sigma = 50;

#include <QImage>
#include <QVector>
#include <QLabel>       // Для отладки
#include <QPixmap>      // Для отладки
#include <math.h>

#include "graph.h"
#include "gmm.h"

enum strokeType { noStroke, strokeForeground, strokeBackground };

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

protected:
    inline float
    distance(int x1, int y1, int x2, int y2)
    {
        int x_2 = x1 - x2;
        int y_2 = y1 - y2;
        return sqrt(x_2 * x_2 + y_2 * y_2);
    }

    inline float
    norma2(const RgbF a, const RgbF b)
    {
        float red  = a.redF   - b.redF;
        float green= a.greenF - b.greenF;
        float blue = a.blueF  - b.blueF;

        return (red * red + green * green + blue * blue);
    }

    float
    Bpq(const QRgb a, const int xa, const int ya, const QRgb b, const int xb, const int yb, const float delta);
};



#endif // PROGRESSIVECUT_H
