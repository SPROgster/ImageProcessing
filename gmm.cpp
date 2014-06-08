#include <cmath>
#include <QImage>

#include "gmm.h"
#include <Eigen/Eigenvalues>

GMM::GMM(unsigned int K) : m_K(K)
{
    m_gaussians = new Gaussian[m_K];
}

GMM::~GMM()
{
    if (m_gaussians)
        delete [] m_gaussians;
}

float GMM::p(QColor c)
{
    float result = 0;

    if (m_gaussians)
    {
        for (unsigned int i=0; i < m_K; i++)
        {
            result += m_gaussians[i].pi * p(i, c);
        }
    }

    return result;
}

float GMM::p(unsigned int i, QColor c)
{
    float result = 0;

    if( m_gaussians[i].pi > 0 )
    {
        if (m_gaussians[i].determinant > 0)
        {
            float r = c.redF()   - m_gaussians[i].mu.redF;
            float g = c.greenF() - m_gaussians[i].mu.greenF;
            float b = c.blueF()  - m_gaussians[i].mu.blueF;

            float d = r * (r * m_gaussians[i].inverse[0][0] + g * m_gaussians[i].inverse[1][0] + b * m_gaussians[i].inverse[2][0]) +
                      g * (r * m_gaussians[i].inverse[0][1] + g * m_gaussians[i].inverse[1][1] + b * m_gaussians[i].inverse[2][1]) +
                      b * (r * m_gaussians[i].inverse[0][2] + g * m_gaussians[i].inverse[1][2] + b * m_gaussians[i].inverse[2][2]);

            result = (float)(1.0 / (sqrt(m_gaussians[i].determinant)) * exp(-0.5 * d));
        }
    }

    return result;
}

void buildGMMs(GMM& backgroundGMM, GMM& foregroundGMM, QImage& components, const QImage& image, const QImage& hardSegmentation)
{
    // Step 3: Build GMMs using Orchard-Bouman clustering algorithm

    // Set up Gaussian Fitters (Пики )
    GaussianFitter* backFitters = new GaussianFitter[backgroundGMM.K()];
    GaussianFitter* foreFitters = new GaussianFitter[foregroundGMM.K()];

    unsigned int foreCount = 0, backCount = 0;

    QRgb* imagePixel = (QRgb*)image.bits();
    QRgb* componentsPixel = (QRgb*)components.bits();
    QRgb* hardSegmentationPixel = (QRgb*)hardSegmentation.bits();

    // Initialize the first foreground and background clusters
    for (int y = 0; y < image.height(); y++)
    {
        for (int x = 0; x < image.width(); x++, imagePixel++, componentsPixel++, hardSegmentationPixel++)
        {
            *componentsPixel = 0;

            if (*hardSegmentationPixel == segmentForeground)
            {
                foreFitters[0].add(*imagePixel);
                foreCount++;
            }
            else if (*hardSegmentationPixel == segmentBackground)
            {
                foreFitters[0].add(*imagePixel);
                foreCount++;
            }
        }
    }

    backFitters[0].finalize(backgroundGMM.m_gaussians[0], backCount, true);
    foreFitters[0].finalize(foregroundGMM.m_gaussians[0], foreCount, true);

    unsigned int nBack = 0, nFore = 0;		// Which cluster will be split
    unsigned int maxK = backgroundGMM.K() > foregroundGMM.K() ? backgroundGMM.K() : foregroundGMM.K();

    // Compute clusters
    for (unsigned int i = 1; i < maxK; i++)
    {
        // Reset the fitters for the splitting clusters
        backFitters[nBack] = GaussianFitter();
        foreFitters[nFore] = GaussianFitter();

        // For brevity, get references to the splitting Gaussians
        Gaussian& bg = backgroundGMM.m_gaussians[nBack];
        Gaussian& fg = foregroundGMM.m_gaussians[nFore];

        // Compute splitting points
        float splitBack = bg.eigenvectors[0][0] * bg.mu.redF + bg.eigenvectors[1][0] * bg.mu.greenF + bg.eigenvectors[2][0] * bg.mu.blueF;
        float splitFore = fg.eigenvectors[0][0] * fg.mu.redF + fg.eigenvectors[1][0] * fg.mu.greenF + fg.eigenvectors[2][0] * fg.mu.blueF;

        QRgb* imagePixel = (QRgb*)image.bits();
        QRgb* componentsPixel = (QRgb*)components.bits();
        QRgb* hardSegmentationPixel = (QRgb*)hardSegmentation.bits();

        // Split clusters nBack and nFore, place split portion into cluster i
        for (int y = 0; y < image.height(); y++)
        {
            for(int x = 0; x < image.width(); x++, imagePixel++, componentsPixel++, hardSegmentationPixel++)
            {
                QColor c(*imagePixel);

                // For each pixel
                if (i < foregroundGMM.K() && *hardSegmentationPixel == segmentForeground && *componentsPixel == nFore)
                {
                    if (fg.eigenvectors[0][0] * c.redF() + fg.eigenvectors[1][0] * c.greenF() + fg.eigenvectors[2][0] * c.blueF() > splitFore)
                    {
                        *componentsPixel = i;
                        foreFitters[i].add(c);
                    }
                    else
                    {
                        foreFitters[nFore].add(c);
                    }
                }
                else if (i < backgroundGMM.K() && *hardSegmentationPixel == segmentBackground && *componentsPixel == nBack)
                {
                    if (bg.eigenvectors[0][0] * c.redF() + bg.eigenvectors[1][0] * c.greenF() + bg.eigenvectors[2][0] * c.blueF() > splitBack)
                    {
                        *componentsPixel = i;
                        backFitters[i].add(c);
                    }
                    else
                    {
                        backFitters[nBack].add(c);
                    }
                }
            }
        }


        // Compute new split Gaussians
        backFitters[nBack].finalize(backgroundGMM.m_gaussians[nBack], backCount, true);
        foreFitters[nFore].finalize(foregroundGMM.m_gaussians[nFore], foreCount, true);

        if (i < backgroundGMM.K())
            backFitters[i].finalize(backgroundGMM.m_gaussians[i], backCount, true);
        if (i < foregroundGMM.K())
            foreFitters[i].finalize(foregroundGMM.m_gaussians[i], foreCount, true);

        // Find clusters with highest eigenvalue
        nBack = 0;
        nFore = 0;

        for (unsigned int j = 0; j <= i; j++ )
        {
            if (j < backgroundGMM.K() && backgroundGMM.m_gaussians[j].eigenvalues[0] > backgroundGMM.m_gaussians[nBack].eigenvalues[0])
                nBack = j;

            if (j < foregroundGMM.K() && foregroundGMM.m_gaussians[j].eigenvalues[0] > foregroundGMM.m_gaussians[nFore].eigenvalues[0])
                nFore = j;
        }
    }

    delete [] backFitters;
    delete [] foreFitters;
}

void learnGMMs(GMM& backgroundGMM, GMM& foregroundGMM, QImage& components, const QImage& image, const QImage& hardSegmentation)
{
    QRgb* imagePixel = (QRgb*)image.bits();
    QRgb* componentsPixel = (QRgb*)components.bits();
    QRgb* hardSegmentationPixel = (QRgb*)hardSegmentation.bits();

    // Step 4: Assign each pixel to the component which maximizes its probability
    for (int y = 0; y < image.height(); y++)
    {
        for (int x = 0; x < image.width(); x++, imagePixel++, componentsPixel++, hardSegmentationPixel++)
        {
            QColor c(*imagePixel);

            if (*hardSegmentationPixel == segmentForeground)
            {
                int k = 0;
                float max = 0;

                for (unsigned int i = 0; i < foregroundGMM.K(); i++)
                {
                    float p = foregroundGMM.p(i, c);
                    if (p > max)
                    {
                        k = i;
                        max = p;
                    }
                }

                *componentsPixel = k;
            }
            else
            {
                int k = 0;
                float max = 0;

                for (unsigned int i = 0; i < backgroundGMM.K(); i++)
                {
                    float p = backgroundGMM.p(i, c);
                    if (p > max)
                    {
                        k = i;
                        max = p;
                    }
                }

                *componentsPixel = k;
            }
        }
    }

    // Step 5: Relearn GMMs from new component assignments

    // Set up Gaussian Fitters
    GaussianFitter* backFitters = new GaussianFitter[backgroundGMM.K()];
    GaussianFitter* foreFitters = new GaussianFitter[foregroundGMM.K()];

    imagePixel = (QRgb*)image.bits();
    componentsPixel = (QRgb*)components.bits();
    hardSegmentationPixel = (QRgb*)hardSegmentation.bits();

    unsigned int foreCount = 0, backCount = 0;

    for (int y = 0; y < image.height(); y++)
    {
        for(int x = 0; x < image.width(); x++, imagePixel++, componentsPixel++, hardSegmentationPixel++)
        {
            QColor c(*imagePixel);

            if(*hardSegmentationPixel == segmentForeground)
            {
                foreFitters[*componentsPixel].add(c);
                foreCount++;
            }
            else if (*hardSegmentationPixel == segmentBackground)
            {
                backFitters[*componentsPixel].add(c);
                backCount++;
            }
        }
    }

    for (unsigned int i = 0; i < backgroundGMM.K(); i++)
        backFitters[i].finalize(backgroundGMM.m_gaussians[i], backCount, false);

    for (unsigned int i = 0; i < foregroundGMM.K(); i++)
        foreFitters[i].finalize(foregroundGMM.m_gaussians[i], foreCount, false);

    delete [] backFitters;
    delete [] foreFitters;
}


// GaussianFitter functions
GaussianFitter::GaussianFitter()
{
    memset(&s, 0, sizeof(s));

    p[0][0] = 0; p[0][1] = 0; p[0][2] = 0;
    p[1][0] = 0; p[1][1] = 0; p[1][2] = 0;
    p[2][0] = 0; p[2][1] = 0; p[2][2] = 0;

    count = 0;
}

// Add a color sample
void GaussianFitter::add(QColor c)
{
    s.redF   += c.redF();
    s.greenF += c.greenF();
    s.blueF  += c.blueF();

    p[0][0] += c.redF()   * c.redF();
    p[0][1] += c.redF()   * c.greenF();
    p[0][2] += c.redF()   * c.blueF();
    p[1][0] += c.greenF() * c.redF();
    p[1][1] += c.greenF() * c.greenF();
    p[1][2] += c.greenF() * c.blueF();
    p[2][0] += c.blueF()  * c.redF();
    p[2][1] += c.blueF()  * c.greenF();
    p[2][2] += c.blueF()  * c.blueF();

    count++;
}

// Build the gaussian out of all the added colors
void GaussianFitter::finalize(Gaussian& g, unsigned int totalCount, bool computeEigens) const
{
    // Running into a singular covariance matrix is problematic. So we'll add a small epsilon
    // value to the diagonal elements to ensure a positive definite covariance matrix.
    const float Epsilon = (float)0.0001;

    if (count==0)
    {
        g.pi = 0;
    }
    else
    {
        // Compute mean of gaussian
        g.mu.redF   = s.redF   / count;
        g.mu.greenF = s.greenF / count;
        g.mu.blueF  = s.blueF  / count;

        // Compute covariance matrix
        g.covariance[0][0] = p[0][0] / count - g.mu.redF   * g.mu.redF + Epsilon;
        g.covariance[0][1] = p[0][1] / count - g.mu.redF   * g.mu.greenF;
        g.covariance[0][2] = p[0][2] / count - g.mu.redF   * g.mu.blueF;
        g.covariance[1][0] = p[1][0] / count - g.mu.greenF * g.mu.redF;
        g.covariance[1][1] = p[1][1] / count - g.mu.greenF * g.mu.greenF + Epsilon;
        g.covariance[1][2] = p[1][2] / count - g.mu.greenF * g.mu.blueF;
        g.covariance[2][0] = p[2][0] / count - g.mu.blueF  * g.mu.redF;
        g.covariance[2][1] = p[2][1] / count - g.mu.blueF  * g.mu.greenF;
        g.covariance[2][2] = p[2][2] / count - g.mu.blueF  * g.mu.blueF + Epsilon;

        // Compute determinant of covariance matrix
        g.determinant = g.covariance[0][0] * (g.covariance[1][1]*g.covariance[2][2]-g.covariance[1][2]*g.covariance[2][1])
                      - g.covariance[0][1] * (g.covariance[1][0]*g.covariance[2][2]-g.covariance[1][2]*g.covariance[2][0])
                      + g.covariance[0][2] * (g.covariance[1][0]*g.covariance[2][1]-g.covariance[1][1]*g.covariance[2][0]);

        // Compute inverse (cofactor matrix divided by determinant)
        g.inverse[0][0] =  (g.covariance[1][1]*g.covariance[2][2] - g.covariance[1][2]*g.covariance[2][1]) / g.determinant;
        g.inverse[1][0] = -(g.covariance[1][0]*g.covariance[2][2] - g.covariance[1][2]*g.covariance[2][0]) / g.determinant;
        g.inverse[2][0] =  (g.covariance[1][0]*g.covariance[2][1] - g.covariance[1][1]*g.covariance[2][0]) / g.determinant;
        g.inverse[0][1] = -(g.covariance[0][1]*g.covariance[2][2] - g.covariance[0][2]*g.covariance[2][1]) / g.determinant;
        g.inverse[1][1] =  (g.covariance[0][0]*g.covariance[2][2] - g.covariance[0][2]*g.covariance[2][0]) / g.determinant;
        g.inverse[2][1] = -(g.covariance[0][0]*g.covariance[2][1] - g.covariance[0][1]*g.covariance[2][0]) / g.determinant;
        g.inverse[0][2] =  (g.covariance[0][1]*g.covariance[1][2] - g.covariance[0][2]*g.covariance[1][1]) / g.determinant;
        g.inverse[1][2] = -(g.covariance[0][0]*g.covariance[1][2] - g.covariance[0][2]*g.covariance[1][0]) / g.determinant;
        g.inverse[2][2] =  (g.covariance[0][0]*g.covariance[1][1] - g.covariance[0][1]*g.covariance[1][0]) / g.determinant;

        // The weight of the gaussian is the fraction of the number of pixels in this Gaussian to the number of
        // pixels in all the gaussians of this GMM.
        g.pi = (float)count/totalCount;

        if (computeEigens)
        {

            Eigen::MatrixX2f cov(3, 3);
            cov(0, 0) = g.covariance[0][0];
            cov(0, 1) = g.covariance[0][1];
            cov(0, 2) = g.covariance[0][2];
            cov(1, 0) = g.covariance[1][0];
            cov(1, 1) = g.covariance[1][1];
            cov(1, 2) = g.covariance[1][2];
            cov(2, 0) = g.covariance[2][0];
            cov(2, 1) = g.covariance[2][1];
            cov(2, 2) = g.covariance[2][2];

            Eigen::EigenSolver<Eigen::MatrixXf> eigen(cov);
            g.eigenvalues[0] = eigen.eigenvalues()[0].real();
            g.eigenvalues[1] = eigen.eigenvalues()[1].real();
            g.eigenvalues[2] = eigen.eigenvalues()[2].real();

            g.eigenvectors[0][0] = eigen.eigenvectors()(0, 0).real();
            g.eigenvectors[0][1] = eigen.eigenvectors()(0, 1).real();
            g.eigenvectors[0][2] = eigen.eigenvectors()(0, 2).real();
            g.eigenvectors[1][0] = eigen.eigenvectors()(1, 0).real();
            g.eigenvectors[1][1] = eigen.eigenvectors()(1, 1).real();
            g.eigenvectors[1][2] = eigen.eigenvectors()(1, 2).real();
            g.eigenvectors[2][0] = eigen.eigenvectors()(2, 0).real();
            g.eigenvectors[2][1] = eigen.eigenvectors()(2, 1).real();
            g.eigenvectors[2][2] = eigen.eigenvectors()(2, 2).real();
        }
    }
}

