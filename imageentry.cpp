#include "imageentry.h"

imageEntry::imageEntry(QWidget *parent, QImage *newImage, const QString &text) :
    QWidget(parent)
{
    if (newImage == 0)
    {
        image = new QImage();
        thumbnail = new QImage();
        label = new QLabel(this);
    }
    else
    {
        image = new QImage(*newImage);
        thumbnail = new QImage(image->scaled(THUMBW, THUMBH));
        label = new QLabel(text, this);
    }
    label->setAlignment(Qt::AlignHCenter & Qt::AlignBottom);

    label->setMinimumWidth(THUMBW);
    label->setMaximumWidth(THUMBW);

    label->setMinimumHeight(THUMBH);
    label->setMaximumHeight(THUMBH);

    setMinimumWidth(THUMBW);
    setMaximumWidth(THUMBW);

    setMinimumHeight(THUMBH);
    setMaximumHeight(THUMBH);
}

imageEntry::~imageEntry()
{
    delete label;
    delete thumbnail;
    delete image;
}

void imageEntry::setLabel(const QString &newText)
{
    label->setText(newText);
}

void imageEntry::setSelected(bool isSelected)
{
    if (isSelected)
        label->setFrameShape(QFrame::Box);
    else
        label->setFrameShape(QFrame::NoFrame);
}

void imageEntry::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    painter.drawImage(0, 0, *thumbnail);
    painter.restore();

}
