#include <math.h>
#include <QColor>
#include <QImage>

#include "progressiveCut.h"

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

float
Bpq(const QColor a, const int xa, const int ya, const QColor b, const int xb, const int yb, const float delta)
{
    float res = - norma2(a, b) / ( 2 * delta );
    res = exp(res) / distance(xa, ya, xb, yb);

    return res;
}


Graph *createGraph(const QImage &image)
{
    int size = image.width() * image.height();

    Graph* graph = new Graph[size];
    memset(graph, 0, sizeof(Graph) * size);

    return graph;
}
