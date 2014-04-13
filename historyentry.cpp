#include "historyentry.h"

historyEntry::historyEntry(QWidget *parent, QImage *newImage) :
    QWidget(parent)
{
}

historyEntry::~historyEntry()
{
    delete image;
}

void historyEntry::paintEvent(QPaintEvent *event)
{

}
