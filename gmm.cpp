#include "gmm.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <cmath>

#include "Eigen/Eigen/Eigenvalues"

GMM::GMM(int k)
{
    K = k;
    g = 0;

    pointsList.clear();
    image = 0;
}

GMM::~GMM()
{
    if (image)
        delete image;

    if (g)
        delete [] g;
}

void GMM::loadImage(QImage &image_)
{
    if (image)
        delete image;

    image = new QImage(image_.convertToFormat(QImage::Format_ARGB32_Premultiplied));
}

float GMM::p(RgbColor x)
{
    float prob = 0;
    for (int i = 0; i < K; i++)
    {
        prob += p(i, x);
    }

    return prob;
}

float GMM::p(QRgb x)
{
    float prob = 0;
    RgbColor x_;
    x_.R = (x & 0xFF0000) >> 16;
    x_.G = (x & 0xFF00) >> 8;
    x_.B = (x & 0xFF);

    for (int i = 0; i < K; i++)
    {
        prob += p(i, x_);
    }

    if (prob < 1e-40)
        prob = 1e-40;

    return prob;
}

void GMM::loadPoints(QImage &points)
{
    uchar* iter = points.bits();
    QRgb* pixel = (QRgb*)image->bits();
    RgbColor newPoint;

    for (int i = 0; i < points.byteCount(); i++, iter++, pixel++)
        if (*iter)
        {
            newPoint.R = (*pixel & 0xFF0000) >> 16;
            newPoint.G = (*pixel & 0xFF00) >> 8;
            newPoint.B = *pixel & 0xFF;

            newPoint.Rf = (float)newPoint.R / 255.;
            newPoint.Gf = (float)newPoint.G / 255.;
            newPoint.Bf = (float)newPoint.B / 255.;

            pointsList << newPoint;
        }
}

void GMM::finalize()
{
    EM em(this);
    g = em.splitNotEM(K, pointsList);
    //g = em.splitContinious(K, pointsList, 20);
}

float gaussN(RgbColor x, const Gaussian &g)
{
    float x_mu[3];
    x_mu[0] = (float)x.R - g.mu.Rf;
    x_mu[1] = (float)x.G - g.mu.Gf;
    x_mu[2] = (float)x.B - g.mu.Bf;

    double expPower = x_mu[0] * (g.inver[0][0] * x_mu[0] + g.inver[0][1] * x_mu[1] + g.inver[0][2] * x_mu[2])
                    + x_mu[1] * (g.inver[1][0] * x_mu[0] + g.inver[1][1] * x_mu[1] + g.inver[1][2] * x_mu[2])
                    + x_mu[2] * (g.inver[2][0] * x_mu[0] + g.inver[2][1] * x_mu[1] + g.inver[2][2] * x_mu[2]);
            //(x - mu) * inv * (x - mu) * 0.5;

    expPower = exp(-0.5 * expPower);

    double pp = g.coeff * expPower;
    return pp;
}


void
EM::fitGaussian(Gaussian *g, QList<RgbColor> &points)
{
    memset(g, 0, sizeof(Gaussian));

    g->w = 1;

    // Количество точек. Понадобится в будущем
    double size = g->pointsCount = points.size();

    // Ссылки
    RgbColor& mu = g->mu;

    // Находим среднее
    RgbColor iter;
    foreach (iter, points)
    {
        // Сумма цветов
        mu.Rf += iter.R;
        mu.Gf += iter.G;
        mu.Bf += iter.B;

        // Для вычисления матрицы ковариаций
        g->sigma[0][0] += iter.R * iter.R;
        g->sigma[0][1] += iter.R * iter.G;
        g->sigma[0][2] += iter.R * iter.B;

        g->sigma[1][1] += iter.G * iter.G;
        g->sigma[1][2] += iter.G * iter.B;

        g->sigma[2][2] += iter.B * iter.B;
    }

    // Вычисляем среднее
    mu.Rf /= size; mu.Gf /= size; mu.Bf /= size;

    g->sigma[0][0] /= size; g->sigma[0][1] /= size; g->sigma[0][2] /= size;
                            g->sigma[1][1] /= size; g->sigma[1][2] /= size;
                                                    g->sigma[2][2] /= size;

    // - Средние
    g->sigma[0][0] -= (double)(mu.Rf * mu.Rf);
    g->sigma[0][1] -= (double)(mu.Rf * mu.Gf);
    g->sigma[0][2] -= (double)(mu.Rf * mu.Bf);

    g->sigma[1][1] -= (double)(mu.Gf * mu.Gf);
    g->sigma[1][2] -= (double)(mu.Gf * mu.Bf);

    g->sigma[2][2] -= (double)(mu.Bf * mu.Bf);

    // Копируем верхний триугольник на нижний триугольник
    g->sigma[1][0] = g->sigma[0][1];
    g->sigma[2][0] = g->sigma[0][2]; g->sigma[2][1] = g->sigma[1][2];

    // Вычисляем присоединеную матрицу для вычисления обратной матрицы.
    // !!! Это транспонированная матрица из миноров (транспонированная матрица алгебраических дополнений)
    // Но, матрица ковариаций симметрична, значит и эта симметрична
    //
    g->inver[0][0] = g->sigma[1][1] * g->sigma[2][2] - g->sigma[1][2] * g->sigma[2][1];
    g->inver[1][0] = g->sigma[1][2] * g->sigma[2][0] - g->sigma[1][0] * g->sigma[2][2];
    g->inver[2][0] = g->sigma[1][0] * g->sigma[2][1] - g->sigma[1][1] * g->sigma[2][0];

    g->inver[0][1] = g->inver[1][0];
    g->inver[1][1] = g->sigma[0][0] * g->sigma[2][2] - g->sigma[0][2] * g->sigma[2][0];
    g->inver[2][1] = g->sigma[0][1] * g->sigma[2][0] - g->sigma[0][0] * g->sigma[2][1];

    g->inver[0][2] = g->inver[2][0];
    g->inver[1][2] = g->inver[2][1];
    g->inver[2][2] = g->sigma[0][0] * g->sigma[1][1] - g->sigma[0][1] * g->sigma[1][0];

    g->det = g->sigma[0][0] * g->inver[0][0] + g->sigma[0][1] * g->inver[0][1]
           + g->sigma[0][2] * g->inver[0][2];

    if (g->det < 0)
    {
        if (g->det > -MinDet)
            g->det = -MinDet;
        g->sqrtDet = sqrt(-g->det);
    }
    else
    {
        if (g->det < MinDet)
            g->det = MinDet;
        g->sqrtDet = sqrt(g->det);
    }

    static double coef = sqrt(8 * M_PI * M_PI * M_PI);
    g->coeff = 1. / (coef * g->sqrtDet);

    // Обратная матрица
    g->inver[0][0] /= g->det;        g->inver[0][1] /= g->det;        g->inver[0][2] /= g->det;
    g->inver[1][0] = g->inver[0][1]; g->inver[1][1] /= g->det;        g->inver[1][2] /= g->det;
    g->inver[2][0] = g->inver[0][2]; g->inver[2][1] = g->inver[1][2]; g->inver[2][2] /= g->det;

    Eigen::MatrixXf cov(3,3);
    cov(0, 0) = g->sigma[0][0]; cov(0, 1) = g->sigma[0][1]; cov(0, 2) = g->sigma[0][2];
    cov(1, 0) = g->sigma[1][0]; cov(1, 1) = g->sigma[1][1]; cov(1, 2) = g->sigma[1][2];
    cov(2, 0) = g->sigma[2][0]; cov(2, 1) = g->sigma[2][1]; cov(2, 2) = g->sigma[2][2];

    Eigen::EigenSolver<Eigen::MatrixXf> eigen(cov);
    g->eigenValues[0] = eigen.eigenvalues()[0].real();
    g->eigenValues[1] = eigen.eigenvalues()[1].real();
    g->eigenValues[2] = eigen.eigenvalues()[2].real();

    g->eigenVectors[0][0] = eigen.eigenvectors()(0,0).real();
    g->eigenVectors[0][1] = eigen.eigenvectors()(0,1).real();
    g->eigenVectors[0][2] = eigen.eigenvectors()(0,2).real();
    g->eigenVectors[1][0] = eigen.eigenvectors()(1,0).real();
    g->eigenVectors[1][1] = eigen.eigenvectors()(1,1).real();
    g->eigenVectors[1][2] = eigen.eigenvectors()(1,2).real();
    g->eigenVectors[2][0] = eigen.eigenvectors()(2,0).real();
    g->eigenVectors[2][1] = eigen.eigenvectors()(2,1).real();
    g->eigenVectors[2][2] = eigen.eigenvectors()(2,2).real();
}


QVector<float>*
EM::Estep(Gaussian *g, int K, QList<RgbColor> &points)
{
    QVector<float>* G = new QVector<float>[K];

    float newG;
    QVector<float> *currP = new QVector<float>[K];

    // Вычисляем текущие вероятности попадания каждой точки в тот или иной кластер
    for (int k = 0; k < K; k++)
    {
        currP[k].clear();
        currP[k].reserve(points.size());

        RgbColor iter;
        foreach (iter, points)
        {
            newG = g[k].w * gaussN(iter, g[k]);
            currP[k] << newG;
        }
    }

    // Считаем векторы скрытых переменных
    for (int k = 0; k < K; k++)
    {
        G[k].clear();
        G[k].reserve(points.size());

        for(int i = 0; i < points.size(); i++)
        {
            newG = 0;

            // знаменатель
            for(int k_ = 0; k_ < K; k_++)
                newG += currP[k_].at(i);

            // Числитель делим на знаменатель
            newG = currP[k].at(i) / newG;

            G[k] << newG;
        }
    }

    delete [] currP;

    return G;
}


void
EM::Mstep(Gaussian *g, QList<RgbColor> &points, QVector<float> &G)
{
    memset(g, 0, sizeof(Gaussian));

    // Ссылки
    RgbColor& mu = g->mu;

    // Количество точек. Понадобится в будущем
    int size = g->pointsCount = points.size();

    /// Вычисляем новый вес
    float gIterf;
    foreach (gIterf, G)
    {
        g->w += gIterf;
    }
    g->w /= size;

    /////////////////////////////////////////////////////////
    /// Находим сумму и матрицу ковариаций
    QList<RgbColor>::iterator iter;
    QVector<float>::iterator gIter;

    for (iter = points.begin(), gIter = G.begin();
         iter != points.end() && gIter != G.end();
         iter++, gIter++)
    {
        // Сумма цветов
        mu.Rf += iter->R * *gIter;
        mu.Gf += iter->G * *gIter;
        mu.Bf += iter->B * *gIter;

        // Для вычисления матрицы ковариаций
        g->sigma[0][0] += iter->R * iter->R * *gIter;
        g->sigma[0][1] += iter->R * iter->G * *gIter;
        g->sigma[0][2] += iter->R * iter->B * *gIter;

        g->sigma[1][0] += iter->G * iter->R * *gIter;
        g->sigma[1][1] += iter->G * iter->G * *gIter;
        g->sigma[1][2] += iter->G * iter->B * *gIter;

        g->sigma[2][0] += iter->B * iter->R * *gIter;
        g->sigma[2][1] += iter->B * iter->G * *gIter;
        g->sigma[2][2] += iter->B * iter->B * *gIter;
    }

    g->mu.Rf /= size; g->mu.Gf /= size; g->mu.Bf /= size;

    g->sigma[0][0] /= size; g->sigma[0][1] /= size; g->sigma[0][2] /= size;
                            g->sigma[1][1] /= size; g->sigma[1][2] /= size;
                                                    g->sigma[2][2] /= size;
    // - Средние
    g->sigma[0][0] -= (double)(mu.Rf * mu.Rf);
    g->sigma[0][1] -= (double)(mu.Rf * mu.Gf);
    g->sigma[0][2] -= (double)(mu.Rf * mu.Bf);

    g->sigma[1][1] -= (double)(mu.Gf * mu.Gf);
    g->sigma[1][2] -= (double)(mu.Gf * mu.Bf);

    g->sigma[2][2] -= (double)(mu.Bf * mu.Bf);

    // Копируем верхний триугольник на нижний триугольник
    g->sigma[1][0] = g->sigma[0][1];
    g->sigma[2][0] = g->sigma[0][2]; g->sigma[2][1] = g->sigma[1][2];
    ///
    /////////////////////////////////////////////////////////

    g->inver[0][0] = g->sigma[1][1] * g->sigma[2][2] - g->sigma[1][2] * g->sigma[2][1];
    g->inver[1][0] = g->sigma[1][2] * g->sigma[2][0] - g->sigma[1][0] * g->sigma[2][2];
    g->inver[2][0] = g->sigma[1][0] * g->sigma[2][1] - g->sigma[1][1] * g->sigma[2][0];

    g->inver[0][1] = g->inver[1][0];
    g->inver[1][1] = g->sigma[0][0] * g->sigma[2][2] - g->sigma[0][2] * g->sigma[2][0];
    g->inver[2][1] = g->sigma[0][1] * g->sigma[2][0] - g->sigma[0][0] * g->sigma[2][1];

    g->inver[0][2] = g->inver[2][0];
    g->inver[1][2] = g->inver[2][1];
    g->inver[2][2] = g->sigma[0][0] * g->sigma[1][1] - g->sigma[0][1] * g->sigma[1][0];

    g->det = g->sigma[0][0] * g->inver[0][0] + g->sigma[0][1] * g->inver[0][1]
           + g->sigma[0][2] * g->inver[0][2];

    if (g->det > 0)
    {
        if (g->det < MinDet)
            g->det = MinDet;
        g->sqrtDet = sqrt(g->det);
    }
    else
    {
        if (g->det > -MinDet)
                    g->det = -MinDet;
        g->sqrtDet = sqrt(-g->det);
    }

    static double coef = sqrt(8 * M_PI * M_PI * M_PI);
    g->coeff = 1. / (coef * g->sqrtDet);

    // Обратная матрица
    g->inver[0][0] /= g->det;        g->inver[0][1] /= g->det;        g->inver[0][2] /= g->det;
    g->inver[1][0] = g->inver[0][1]; g->inver[1][1] /= g->det;        g->inver[1][2] /= g->det;
    g->inver[2][0] = g->inver[0][2]; g->inver[2][1] = g->inver[1][2]; g->inver[2][2] /= g->det;
}


Gaussian *EM::split(int K, QList<RgbColor> &points, Gaussian *g)
{
    float delta = deltaMax + 1;

    // Работает с гауссианами
    Gaussian* gaussians = g;
    if (!gaussians)
    {
        Gaussian* gaussians = new Gaussian[K];
        memset(gaussians, 0, sizeof(Gaussian) * K);
    }

    QVector<float>* Glast;
    QVector<float>* G;

    // Первая итерация
    Glast = Estep(gaussians, K, points);
    for (int k = 0; k < K; k++)
        Mstep(gaussians + k, points, Glast[k]);

    while (delta > deltaMax)
    {
        G = Estep(gaussians, K, points);
        Mstep(gaussians, points, *G);

        // Вычисляем ограничения по выходу
        delta = 0;
        for (int i = 0; i < G->size(); i++)
        {
            float sum = 0;
            for (int k = 0; k < K; k++)
            {
                sum += (G[k][i] - Glast[k][i]) * (G[k][i] - Glast[k][i]);
            }
            sum = sqrt(sum);

            if (sum > delta)
                delta = sum;
        }

        delete [] Glast;
        Glast = G;
    }

    return gaussians;
}


Gaussian*
EM::splitContinious(int &K, QList<RgbColor> &points, int minPoints)
{
    // Создаем место под гауссианы
    Gaussian* gaussians = new Gaussian[K];
    memset(gaussians, 0, sizeof(Gaussian) * K);

    int pointsCount = points.size();

    QVector<float>* G = 0;

    QVector<float> currP;

    QList<RgbColor> U;

    // Первый кластер
    fitGaussian(gaussians, points);
    gaussians[0].w = 1;

    for (int k = 1; k < K; k++)
    {
        currP.clear();
        currP.reserve(pointsCount);

        // Поиск максимальной вероятности
        float maxP = 0;
        float minP = 9999999;
        float p;
        RgbColor iter;
        foreach (iter, points)
        {
            p = 0;
            // Поиск вероятности
            for (int i = 0; i < k; i++)
            {
                p += gaussians[i].w * gaussN(iter, gaussians[i]);
            }

            // Ищем максимум
            if (p > maxP)
                maxP = p;
            if (p < minP)
                minP = p;

            currP << p;
        }
        maxP /= R;

        // Выделение нового кластера
        U.clear();
        U.reserve(pointsCount);
        for (int i = 0; i < pointsCount; i++)
            if (currP[i] < maxP)
                U << points[i];

        if (U.size() < minPoints)
        {
            K = k;

            if (G)
                delete [] G;

            return gaussians;
        }

        // Вычисление коэффициентов
        fitGaussian(gaussians + k, U);
        // Новый вес
        gaussians[k].w = (float)U.size() / points.size();
        for (int i = 0; i < k; i++)
            gaussians[i].w -= gaussians[k].w;

        split(k + 1, points, gaussians);
    }

    return gaussians;
}

Gaussian *EM::splitNotEM(int K, QList<RgbColor> &points)
{
    // Создаем место под гауссианы
    Gaussian* g = new Gaussian[K];
    memset(g, 0, sizeof(Gaussian) * K);
    QList<RgbColor>* cluster = new QList<RgbColor>[K];

    int pointsCount = points.size();

    foreach (RgbColor iter, points) {
        cluster[0] << iter;
    }
    fitGaussian(g, points);

    int clusterNum = 0;

    for (int k = 1; k < K; k++)
    {
        // Разбиваем кластер
        Gaussian &toSplit = g[clusterNum];
        float splitPoint = toSplit.eigenVectors[0][0] * toSplit.mu.Rf
                         + toSplit.eigenVectors[1][0] * toSplit.mu.Gf
                         + toSplit.eigenVectors[2][0] * toSplit.mu.Bf;
        for (int i = cluster[clusterNum].size() - 1; i > 0; i--)
        {
            RgbColor pixel = cluster[clusterNum][i];
            float splitParam = toSplit.eigenVectors[0][0] * pixel.R
                             + toSplit.eigenVectors[1][0] * pixel.G
                             + toSplit.eigenVectors[2][0] * pixel.B;

            if (splitParam > splitPoint)
            {
                cluster[k] << pixel;
                cluster[clusterNum].removeAt(i);
            }
        }

        // Пересчитываем кластеры
        fitGaussian(g + clusterNum, cluster[clusterNum]);
        fitGaussian(g + k, cluster[k]);
        g[clusterNum].w = (float)cluster[clusterNum].size() / pointsCount;
        g[k].w = (float)cluster[k].size() / pointsCount;

        // Ищем, какой бы дальше разбить
        float maxEigen = g[0].eigenValues[0];
        clusterNum = 0;
        for (int i = 1; i <= k; i++)
        {
            if (float curEigen = g[i].eigenValues[0] > maxEigen)
            {
                clusterNum = i;
                maxEigen = curEigen;
            }
        }

    }

    delete [] cluster;

    return g;
}
