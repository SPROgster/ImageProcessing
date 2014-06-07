#include "graph.h"

Graph::Graph(unsigned int width_, unsigned int height_)
{
    width = width_;
    height = height_;

    notes.clear();

    if (width && height)
    {
        notes.reserve(width*height);

        for (unsigned int y = 0; y < height; y++)
            for (unsigned int x = 0; x < width; x++)
            {
                GraphNote* newNote = new GraphNote(x, y);
                notes << newNote;
            }
    }
}

Graph::~Graph()
{
    GraphNote* iter;
    foreach (iter, notes)
    {
        delete iter;
    }

    notes.clear();
    notesToBegin.clear();
    notesToEnd.clear();
}

void Graph::setSize(unsigned int width_, unsigned int height_)
{
    GraphNote* iter;
    foreach (iter, notes)
    {
        delete iter;
    }

    width = width_;
    height = height_;

    notes.clear();

    if (width && height)
    {
        notes.reserve(width*height);

        for (unsigned int y = 0; y < height; y++)
            for (unsigned int x = 0; x < width; x++)
            {
                GraphNote* newNote = new GraphNote(x, y);
                notes << newNote;
            }
    }
}

void Graph::weightTo(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, float weight)
{
    unsigned int coord = x1 + y1 * width;
    notes[coord]->weightTo(x2, y2, weight);

    coord = x2 + y2 * width;
    notes[coord]->weightTo(x1, y1, weight);
}

void Graph::weightToBegin(unsigned int x, unsigned int y, float weight)
{
    unsigned int coord = x + y * width;
    notes[coord]->weightToBegin(weight);
}

void Graph::weightToEnd(unsigned int x, unsigned int y, float weight)
{
    unsigned int coord = x + y * width;
    notes[coord]->weightToEnd(weight);
}


Graph::GraphNote::GraphNote(int x_, int y_)
{
    x = x_;
    y = y_;
    weightsList.clear();

    weightBegin = 0;
    weightEnd = 0;
    linkToBegin = false;
    linkToEnd = false;
    isCutedBegin = false;
    isCutedEnd = false;
}

Graph::GraphNote::~GraphNote()
{
    weightsList.clear();
}

void Graph::GraphNote::weightTo(int x, int y, float weight)
{
    if (weightsList.size())
    {
        bool notFound = true;
        for (QList<weightXY>::iterator iter = weightsList.begin(), end = weightsList.end(); iter != end && notFound; ++iter)
        {
            if (iter->x == x && iter->y == y)
            {
                notFound = false;
                iter->weight = weight;
            }
        }

        if (notFound)
        {
            weightXY newLink;
            newLink.x = x;
            newLink.y = y;
            newLink.weight = weight;
            newLink.isCuted = false;

            weightsList << newLink;
        }
    }
    else
    {
        weightXY newLink;
        newLink.x = x;
        newLink.y = y;
        newLink.weight = weight;
        newLink.isCuted = false;

        weightsList << newLink;
    }
}

void Graph::GraphNote::weightToBegin(float weight)
{
    weightBegin = weight;
    linkToBegin = true;
    isCutedBegin = false;
}

void Graph::GraphNote::weightToEnd(float weight)
{
    weightEnd = weight;
    linkToEnd = true;
    isCutedEnd = false;
}
