#ifndef CONFIG_H
#define CONFIG_H

struct RgbColor
{
    unsigned int R, G, B;
    float Rf, Gf, Bf;
};

struct xy
{
    int x, y;
};

#define GMM_K 5
#define MinDet 1e-6

#endif // CONFIG_H
