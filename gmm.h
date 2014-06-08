#ifndef GMM_H
#define GMM_H

#include <QColor>

struct Rgb
{
    int red, green, blue;
};

struct RgbF
{
    float redF, greenF, blueF;
};

struct Gaussian
{
    RgbF mu;					// mean of the gaussian
    float covariance[3][3];		// covariance matrix of the gaussian
    float determinant;			// determinant of the covariance matrix
    float inverse[3][3];			// inverse of the covariance matrix
    float pi;					// weighting of this gaussian in the GMM.

    // These are only needed during Orchard and Bouman clustering.
    float eigenvalues[3];		// eigenvalues of covariance matrix
    float eigenvectors[3][3];	// eigenvectors of   "          "
};

class GMM
{
public:

    // Initialize GMM with number of gaussians desired.
    GMM(unsigned int K);
    ~GMM();

    unsigned int K() const { return m_K; }

    // Returns the probability density of color c in this GMM
    float p(QRgb c);

private:

    // Returns the probability density of color c in just Gaussian k
    float p(unsigned int i, QRgb c);

    unsigned int m_K;		// number of gaussians
    Gaussian* m_gaussians;	// an array of K gaussians

    friend void buildGMMs(GMM& backgroundGMM, GMM& foregroundGMM, QImage& components, const QImage& image, QImage& hardSegmentation);
    friend void learnGMMs(GMM& backgroundGMM, GMM& foregroundGMM, QImage& components, const QImage& image, const QImage& hardSegmentation);
};

// Build the initial GMMs using the Orchard and Bouman color clustering algorithm
void buildGMMs(GMM& backgroundGMM, GMM& foregroundGMM, QImage& components, const QImage& image, QImage &hardSegmentation);

// Iteratively learn GMMs using GrabCut updating algorithm
void learnGMMs(GMM& backgroundGMM, GMM& foregroundGMM, QImage& components, const QImage& image, const QImage& hardSegmentation);


// Helper class that fits a single Gaussian to color samples
class GaussianFitter
{
public:
    GaussianFitter();

    // Add a color sample
    void add(QRgb &c);

    // Build the gaussian out of all the added color samples
    void finalize(Gaussian& g, unsigned int totalCount, bool computeEigens = false) const;

private:

    Rgb s;              // sum of r,g, and b
    int  p[3][3];		// matrix of products (i.e. r*r, r*g, r*b), some values are duplicated.

    unsigned int count;	// count of color samples added to the gaussian
};

RgbF rgbToFloat(Rgb input);

#endif // GMM_H
