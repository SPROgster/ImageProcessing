#include <math.h>
#include <QColor>

inline float
distance(int x1, int y1, int x2, int y2)
{
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

inline float
norma2(const QColor a, const QColor b)
{
    int r = a.red()   - b.red();
    int g = a.green() - b.green();
    int b = a.blue()  - b.blue();

    return (r * r + g * g + b * b);
}

float
Bpq(const QColor a, const int xa, const int ya, const QColor b, const int xb, const int yb, const float delta)
{
    float res = - norma2(a, b) / ( 2 * delta );
    res = exp(res) / distance(xa, ya, xb, yb);

    return res;
}
