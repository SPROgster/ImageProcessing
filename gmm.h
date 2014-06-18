#ifndef GMM_H
#define GMM_H

#include "config.h"
#include <QVector>
#include <QImage>

struct Gaussian
{
    double w;            // Вес гауссиана
    RgbColor mu;        // Среднее
    double sigma[3][3];  // Матрица ковариаций [i][j]
    double det;          // Определитель матрицы ковариаций
    double sqrtDet;      // Квадрат определителя
    double inver[3][3];  // Матрица обратная к матрице ковариаций
    int pointsCount;    // Количество точек
    double coeff;

    double eigenValues[3];
    double eigenVectors[3][3];
};

float gaussN(RgbColor x, const Gaussian& g);
Gaussian* EMclustering(Gaussian* gaussians, const int K, const float delta);

class GMM
{
public:
    GMM(int k = 5);
    ~GMM();

    void loadImage(QImage &image_);
    void loadPoints(QImage &points);
    void finalize();

    float p(RgbColor x);
    float p(QRgb x);

private:
    int K;              // Число гауссиан. По крайней мере пока задается
    int pointsCount;    // Количество точек. Может понадобится в будущем

    QImage* image;

    Gaussian * g;
    QList<RgbColor> pointsList;

    inline float p(int i, RgbColor x)
    {
        return g[i].w * gaussN(x, g[i]);
    }

    friend class EM;
};

class EM
{
private:

    GMM* gmmParent;

    float R;                // Параметр разбиения вероятности
    float deltaMax;         // Параметр, по которому выходим из EM

    float* computeG(Gaussian);
    void fitGaussian(Gaussian *g, QList<RgbColor> &points);

    /// EM шаги
    // E шаг
    QVector<float>* Estep(Gaussian *g, int K, QList<RgbColor> &points);

    // M шаг
    void Mstep(Gaussian* g, QList<RgbColor> &points, QVector<float> &G);

public:
    EM(GMM* gmm = 0, float delta = 0.0005, float R_ = 2) :
        gmmParent(gmm), R(R_), deltaMax(delta)  {}

    // Обычный EM алгоритм
    Gaussian* split(int K, QList<RgbColor> &points, Gaussian* g = 0);
    // EM с последовательным добавлением элементов
    Gaussian* splitContinious(int &K, QList<RgbColor> &points, int minPoints = 0);

    Gaussian* splitNotEM(int K, QList<RgbColor> &points);
};

#endif // GMM_H
