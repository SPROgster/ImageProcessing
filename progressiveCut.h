#ifndef PROGRESSIVECUT_H
#define PROGRESSIVECUT_H
#include <QColor>

struct Graph
{
    float left, right, top, down;
    bool leftExists, rightExists, topExists, downExists;
    bool leftCut, rightCut, topCut, downCut;
    float object, background;
};

float Bpq(const QColor a, const int xa, const int ya, const QColor b, const int xb, const int yb, const float delta);
Graph *createGraph(const QImage& image);

#endif // PROGRESSIVECUT_H
