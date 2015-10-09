#ifndef PROTRACTOR_H
#define PROTRACTOR_H

#include <QWidget>
#include <boost/optional.hpp>

class Protractor : public QWidget
{
    Q_OBJECT

public:
    Protractor(QWidget *parent = 0);
    ~Protractor();

    QSize sizeHint() const;
    int heightForWidth(int w) {return w;}
    bool hasHeightForWidth() {return true;}

public slots:
    void rotateRight();
    void rotateLeft();
    void rotateDown();
    void rotateUp();
    void reverse();
    void about();
    void nextUnit();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent * event);
    void releaseEvent(QEvent * event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void saveState();

private:
    void reshape();
    QString configPath();
    void loadState();
    QString angleRepr(double angle);

    QPoint dragPosition_;
    bool windowRotated_;
    boost::optional<double> rotatePosition_;
    boost::optional<double> direction_;
    double rotation_;
    bool mouseGrabbed_;
    // 1 for counter clockwise -1 for clockwise
    double scaleDirection_;
    bool useRadians_;
};

#endif // PROTRACTOR_H
