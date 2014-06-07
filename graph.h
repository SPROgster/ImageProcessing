#ifndef GRAPH_H
#define GRAPH_H

#include <QList>

class Graph
{
private:
    // Класс узла графа. Сток и исток к ним не относятся
    class GraphNote
    {
    private:
        struct weightXY;
        bool linkToBegin, linkToEnd;
        bool isCutedBegin, isCutedEnd;

    private:
        int x, y;
        QList<weightXY> weightsList;
        float weightBegin, weightEnd;

    public:
        GraphNote(int x_ = -1, int y_ = -1);
        ~GraphNote();

        void weightTo(int x, int y, float weight);
        void weightToBegin(float weight);
        void weightToEnd(float weight);

        // Добавить создание маски разбиения или объектов/фона

        friend class Graph;
    };

    struct GraphNote::weightXY
    {
        int x, y;
        float weight;
        bool isCuted;
    };

    QList<GraphNote*> notes;
    QList<GraphNote::weightXY> notesToBegin;
    QList<GraphNote::weightXY> notesToEnd;
    unsigned int width, height;

public:
    Graph(unsigned int width_ = 0, unsigned int height_ = 0);
    ~Graph();

    void setSize(unsigned int width_,unsigned int height_);

    void weightTo(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, float weight);
    void weightToBegin(unsigned int x, unsigned int y, float weight);
    void weightToEnd(unsigned int x, unsigned int y, float weight);

    friend class GraphNote;
};

#endif // GRAPH_H
