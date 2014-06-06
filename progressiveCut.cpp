#include <math.h>
#include <QColor>
#include <QImage>

#include "progressiveCut.h"

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
