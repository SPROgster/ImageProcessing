#include "imageentry.h"

imageEntry::imageEntry(QWidget *parent, QImage *newImage) :
    QWidget(parent)
{

}

imageEntry::~historyEntry()
{
    delete image;
}
