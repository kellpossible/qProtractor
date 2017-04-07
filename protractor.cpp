#include "protractor.h"

#include <QtGui>
#include <QMenu>
#include <QMessageBox>
#include <boost/assign/std/vector.hpp>

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

#include "ui_about.h"

using std::atan2;
using std::cos;
using std::sin;
using namespace boost::assign; // bring 'operator+=()' into scope

const int CIRCLE_WIDTH = 50;

namespace {
QPoint cartesian(double r, double ang) {
    return QPoint(r * cos(ang), r * sin(ang));
}
} // namespace

Protractor::Protractor(QWidget *parent)
    : QWidget(parent)
    , windowRotated_(false)
    , rotation_(0.0)
    , mouseGrabbed_(false)
    , scaleDirection_(1.0)
    , useRadians_(false)
{ 
    setMouseTracking(true);
    setWindowTitle(tr("Protractor"));
    setWindowFlags(Qt::FramelessWindowHint);

    // Without it pos() is broken on first call on X11
    resize(sizeHint());

    loadState();
}

Protractor::~Protractor()
{

}

void Protractor::mousePressEvent(QMouseEvent *event)
{
    if (mouseGrabbed_) {
        releaseMouse();
        mouseGrabbed_ = false;
    }
    if (event->button() == Qt::LeftButton) {
        dragPosition_ = event->globalPos() - pos();
        event->accept();
    }
    if (event->button() == Qt::RightButton) {
        QPoint heading = event->pos() - QPoint(width(), height()) / 2;
        // Y axis goes upside down
        rotatePosition_ = atan2(-heading.y(), heading.x());
        windowRotated_ = false;
    }
}

void Protractor::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition_);
        event->accept();
    } else if (event->buttons() & Qt::RightButton) {
        QPoint heading = event->pos() - QPoint(width(), height()) / 2;
        double angle = atan2(-heading.y(), heading.x());
        rotation_ -= *rotatePosition_ - angle;
        rotatePosition_ = angle;
        windowRotated_ = true;
        event->accept();
    }

    QPoint heading = event->pos() - QPoint(width(), height()) / 2;
    // Y axis goes upside down
    direction_ = atan2(-heading.y(), heading.x()) - rotation_;

    repaint(0, 0, width(), height());
}

void Protractor::mouseReleaseEvent(QMouseEvent * event) {
    if (rotatePosition_ && !windowRotated_) {
        QMenu menu(this);

        QAction* rotateAction = new QAction(tr("Rotate &right"), this);
        connect(rotateAction, SIGNAL(triggered()), this, SLOT(rotateRight()));
        menu.addAction(rotateAction);
        rotateAction = new QAction(tr("Rotate &up"), this);
        connect(rotateAction, SIGNAL(triggered()), this, SLOT(rotateUp()));
        menu.addAction(rotateAction);
        rotateAction = new QAction(tr("Rotate &left"), this);
        connect(rotateAction, SIGNAL(triggered()), this, SLOT(rotateLeft()));
        menu.addAction(rotateAction);
        rotateAction = new QAction(tr("Rotate &down"), this);
        connect(rotateAction, SIGNAL(triggered()), this, SLOT(rotateDown()));
        menu.addAction(rotateAction);
        menu.addSeparator();

        QAction* reverseAction = new QAction(tr("Re&verse"), this);
        connect(reverseAction, SIGNAL(triggered()), this, SLOT(reverse()));
        menu.addAction(reverseAction);
        QAction* unitAction = new QAction(tr("Change &units"), this);
        connect(unitAction, SIGNAL(triggered()), this, SLOT(nextUnit()));
        menu.addAction(unitAction);
        menu.addSeparator();

        QAction* saveAction = new QAction(tr("&Save state"), this);
        connect(saveAction, SIGNAL(triggered()), this, SLOT(saveState()));
        menu.addAction(saveAction);
        QAction* aboutAction = new QAction(tr("&About/help"), this);
        connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
        menu.addAction(aboutAction);
        QAction* quitAction = new QAction(tr("E&xit"), this);
        connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
        menu.addAction(quitAction);

        menu.move(event->globalPos());
        menu.exec();
    }
    rotatePosition_ = boost::none;
    repaint(0, 0, width(), height());
}

void Protractor::leaveEvent(QEvent * /* event */) {
    direction_ = boost::none;
    rotatePosition_ = boost::none;
    repaint(0, 0, width(), height());
}

void Protractor::mouseDoubleClickEvent(QMouseEvent * /* event */) {
    grabMouse();
    mouseGrabbed_ = true;
}

void Protractor::resizeEvent(QResizeEvent *event)
{
    if (width() != height()) {
        // Preserve aspect ratio. heightForWidth does not work for X11.
        // They say, that calling resize from resizeEvent is a bad idea and may
        // lead to infinite loops. Feel free to suggest better way.
        resize(QSize(height(), height()));
    }
    reshape();
}

void Protractor::wheelEvent(QWheelEvent * event) {
    QSize newSize = size() + QSize(1, 1) * (event->delta() / 10.0);
    if (newSize.width() > CIRCLE_WIDTH * 3.0) {
        resize(newSize);
    }
    event->accept();
}

void Protractor::reshape() {
    QRegion region(QRect(1, 1, width() - 2, height() - 2), QRegion::Ellipse);
    region = region.subtracted(QRegion(
        QRect(CIRCLE_WIDTH, CIRCLE_WIDTH, width() - CIRCLE_WIDTH * 2,
            height() - CIRCLE_WIDTH * 2),
        QRegion::Ellipse));
    region = region.united(QRegion(
        QRect(width()/2 - 1, CIRCLE_WIDTH / 2, 3, height() - CIRCLE_WIDTH)));
    region = region.united(QRegion(
        QRect(CIRCLE_WIDTH / 2, height() / 2 - 1, width() - CIRCLE_WIDTH, 3)));

    setMask(region);
}

QSize Protractor::sizeHint() const
{
     return QSize(400, 400);
}

void Protractor::keyPressEvent(QKeyEvent * event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void Protractor::paintEvent(QPaintEvent * /* event */) {
    QPainter painter(this);

    painter.setRenderHint(QPainter::TextAntialiasing, false);

    QPoint center(width()/2, height()/2);

    QRadialGradient radialGradient(center, width()/2);
    radialGradient.setColorAt(0.5, QColor("#FCD580"));
    radialGradient.setColorAt(1.0, QColor("#C69C3F"));
    painter.setBrush(radialGradient);
    painter.drawRect(0, 0, width(), height());

    std::vector<QPoint> crossPoints;
    crossPoints +=
        QPoint(width()/2 - 1, CIRCLE_WIDTH),
        QPoint(width()/2 + 1, CIRCLE_WIDTH),
        QPoint(width() - CIRCLE_WIDTH, height() / 2 - 1),
        QPoint(width() - CIRCLE_WIDTH, height() / 2 + 1),
        QPoint(width()/2 + 1, height() - CIRCLE_WIDTH),
        QPoint(width()/2 - 1, height() - CIRCLE_WIDTH),
        QPoint(CIRCLE_WIDTH, height() / 2 + 1),
        QPoint(CIRCLE_WIDTH, height() / 2 - 1);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawConvexPolygon(&crossPoints[0], crossPoints.size());

    painter.setPen(Qt::white);
    painter.drawLine(width()/2, CIRCLE_WIDTH, width()/2, height() - CIRCLE_WIDTH);
    painter.drawLine(CIRCLE_WIDTH, height()/2, width() - CIRCLE_WIDTH, height()/2);

    painter.translate(width() / 2, height() / 2);

    painter.setPen(Qt::black);
    if (direction_) {
        painter.drawText(-15, height() / 2 - CIRCLE_WIDTH + 3, 30, 15,
            Qt::AlignHCenter, angleRepr(*direction_));
    }
    if (rotatePosition_) {
        painter.drawText(-15, - height() / 2 + CIRCLE_WIDTH - 30, 20, 15,
            Qt::AlignHCenter, angleRepr(rotation_));
    }

    painter.rotate(-rotation_ / M_PI * 180.0 - 90);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);
    painter.save();
    for (int i = 0; i < 36; ++i) {
        painter.drawLine(0, height() / 2, 0, height() / 2 - 15);
        painter.rotate(5.0);
        painter.drawLine(0, height() / 2, 0, height() / 2 - 7);
        painter.rotate(5.0);
    }
    for (int i = 0; i < 360; ++i) {
        painter.drawLine(0, height() / 2, 0, height() / 2 - 3);
        painter.rotate(1.0);
    }
    painter.restore();
    painter.save();
    static const int TEXT_POS = 30;
    static const int TEXT_WIDTH = 30;
    int minTextStep = 360 / (2 * M_PI * (height() / 2 - TEXT_POS) / TEXT_WIDTH);
    minTextStep = ((minTextStep / 5) + 1) * 5;
    for (int angle = 0; angle < 360; angle += minTextStep) {
        painter.drawText(-15, height() / 2 - 30, 30, 15,
            Qt::AlignHCenter, angleRepr(angle / 180.0 * M_PI));
        painter.rotate(minTextStep * scaleDirection_);
    }
    painter.restore();

    painter.setPen(Qt::NoPen);
    if (direction_) {
        const QPoint directionPointer1[3] = {
            QPoint(width()/2 - CIRCLE_WIDTH + 2, 0),
            QPoint(width()/2 - CIRCLE_WIDTH + 8, 3),
            QPoint(width()/2 - CIRCLE_WIDTH + 8, -3)
        };
        const QPoint directionPointer2[3] = {
            QPoint(width()/2 - 2, 0),
            QPoint(width()/2 - 8, 3),
            QPoint(width()/2 - 8, -3)
        };

        painter.rotate(-*direction_ / M_PI * 180.0 + 90.0);
        painter.setBrush(Qt::red);
        painter.drawConvexPolygon(directionPointer1, 3);
        painter.drawConvexPolygon(directionPointer2, 3);
    }
}

void Protractor::rotateRight() {
    rotation_ = 0.0;
    repaint(0, 0, width(), height());
}
void Protractor::rotateLeft() {
    rotation_ = M_PI;
    repaint(0, 0, width(), height());
}
void Protractor::rotateDown() {
    rotation_ = M_PI * 1.5;
    repaint(0, 0, width(), height());
}
void Protractor::rotateUp() {
    rotation_ = M_PI / 2.0;
    repaint(0, 0, width(), height());
}

void Protractor::saveState() {
    QFile file(configPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(0, "Can't open file",
            "Can't open config file for writing. Path:" + configPath());
        return;
    }
    QTextStream config(&file);
    config << rotation_ << "\n"
           << scaleDirection_ << "\n"
           << pos().x() << " " << pos().y() << "\n"
           << width() << " " << height() << "\n"
           << useRadians_ << "\n";
}

void Protractor::loadState() {
    QFile file(configPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream config(&file);
    int x, y;
    int w, h;
    config >> rotation_ >> scaleDirection_ >> x >> y >> w >> h;
    move(x, y);
    resize(w, h);

    int useRadians;
    config >> useRadians;
    useRadians_ = useRadians;
}

QString Protractor::configPath() {
    QString folderName =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (!QDir(folderName).exists()) {
        QDir().mkpath(folderName);
    }
    return folderName + "/qProtractor.conf";
}

void Protractor::reverse() {
    scaleDirection_ *= -1.0;
}

void Protractor::about() {
    QDialog *dialog = new QDialog(this);
    Ui::AboutDialog ui;
    ui.setupUi(dialog);

    dialog->show();
}

void Protractor::nextUnit() {
    useRadians_ = !useRadians_;
}

QString Protractor::angleRepr(double angle) {
    if (!useRadians_) {
        angle *= 180.0 / M_PI;
    }
    angle *= scaleDirection_;

    double maxAngle = useRadians_ ? 2.0 * M_PI : 360.0;
    angle = fmod(angle + maxAngle, maxAngle);

    if (useRadians_) {
        return QString::number(angle, 'd', 2);
    } else {
        return QString::number(angle, 'd', 0);
    }
}
