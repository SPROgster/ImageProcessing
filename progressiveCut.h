#ifndef PROGRESSIVECUT_H
#define PROGRESSIVECUT_H

#include <QImage>
#include <QVector>
#include <QLabel>       // Для отладки
#include <QPixmap>      // Для отладки
#include <QPainter>     // Для отладки
#include <math.h>

#include "config.h"
#include "graph.h"
#include "gmm.h"

enum strokeType { noStroke, strokeForeground, strokeBackground };

struct Weight
{
    float toSource, toSink;         // toFore, toBack
    float BL, BB, BR, RR;           // Вниз-влево, вниз, вниз-вправо, вправо
};

class ProgressiveCut
{
private:
    Graph* graph;
    Graph::node_id* nodes;

    QImage* image;
    QImage* foregroundSelection;
    QImage* backgroundSelection;
    QImage* strokeSelection;
    QImage* strokeSelectionNew;
    QImage* foregroundStroke;
    QImage* backgroundStroke;

    QVector<QRgb> maskColorTable;
    QVector<QRgb> strokeColorTable;
    QVector<QRgb> componentsColorTable;

    int imageWidth, imageHeight, imageSizeInPixels;
    float infinity;

    QVector<Weight> lastWeights;

    void initColorTables();

    // Две маски вливаются в одну
    int createStrokeMask();

    /// Вычисление "энергии интереса"
    // Измерение границ объектов
    xy* getObjectArea(QImage &object);
    float* getDistanseFromStroke(QImage &object);
    void distanceDebug();
    float* getInterestsEnergy(bool isForeground);
    void debugIntesets(float* interestsEnergy);

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

    QImage* selection;

    void setImage(const QImage& imageToCut);
    bool setForeground(const QImage& foreground);
    bool setBackground(const QImage& background);

    bool updateForeground(const QImage& foreground);
    bool updateBackground(const QImage& background);

    void deleteUpdation();

    void connectNodes(bool update = false, bool foreground = false);

    bool createGraph();
    bool updateGraph(bool foreground);

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
    norma2(const RgbColor a, const RgbColor b)
    {
        float red  = a.R - b.R;
        float green= a.G - b.G;
        float blue = a.B - b.B;

        return (red * red + green * green + blue * blue) / (255. * 255.);
    }

    float
    Bpq(const QRgb a, const int xa, const int ya, const QRgb b, const int xb, const int yb);
};

#endif // PROGRESSIVECUT_H
