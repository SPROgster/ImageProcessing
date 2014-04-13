#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

#include <QWidget>

class imageEntry : public QWidget
{
    Q_OBJECT
public:
    explicit imageEntry(QWidget *parent = 0, QImage* newImage = 0);
    ~historyEntry();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:
    Qimage* image;
};

#endif // IMAGEENTRY_H
