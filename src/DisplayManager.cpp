/*                      D I S P L A Y M A N A G E R . C P P
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/** @file DisplayManager.cpp
 *
 *  BRL-CAD GUI:
 *      the display functions class implementation
 */

#include <cmath>

#include "DisplayManager.h"


const float MaxFloat   = std::numeric_limits<float>::max();
const float SmallFloat = std::numeric_limits<float>::epsilon();


static double Distance
(
    const QPoint& from,
    const QPoint& to
) {
    QPoint vector = to - from;

    return sqrt(vector.x() * vector.x() + vector.y() * vector.y());
}


static double Length
(
    const QPoint& point
) {
    return sqrt(point.x() * point.x() + point.y() * point.y());
}


static QMatrix4x4 Projection
(
    const QVector3D& eyePoint,
    const QVector3D& targetPoint
) {
    QMatrix4x4 ret;
    QVector3D  dir = eyePoint - targetPoint;

    if (dir.length() > SmallFloat) {
        float length = sqrt(dir.x() * dir.x() + dir.y() * dir.y());

        if (length > SmallFloat)
            ret.lookAt(eyePoint, targetPoint, QVector3D(0.f, 0.f, 1.f));
        else
            ret.lookAt(eyePoint, targetPoint, QVector3D(0.f, 1.f, 0.f));
    }

    return ret;
}


DisplayManager::DisplayManager
(
    QWidget* parent
) : QOpenGLWidget(parent),
    QOpenGLFunctions(),
    m_displayListId(0),
    m_updateDisplayList(false),
    m_eyePoint(0.f, 0.f, 0.f),
    m_targetPoint(0.f, 0.f, -1.f),
    m_paintAction(PaintAction::None),
    m_trafoStack(),
    m_setDisplayAttributes(false),
    m_model(0),
    m_attributeStack(),
    m_setAttributes(false) {
    QRect geometry = parent->geometry();

    m_displayMin = QPoint(geometry.x(), geometry.y());
    m_displayMax = QPoint(geometry.x() + geometry.width(), geometry.y() + geometry.height());

    if (widthMM() > 0)
        m_displayUnit.setX(static_cast<float>(width()) / static_cast<float>(widthMM()));
    else
        m_displayUnit.setX(1.f);

    if (heightMM() > 0)
        m_displayUnit.setY(static_cast<float>(height()) / static_cast<float>(heightMM()));
    else
        m_displayUnit.setY(1.f);

    if (geometry.left() > geometry.right())
        m_displayUnit.setX(m_displayUnit.x() * -1.f);

    if (geometry.bottom() > geometry.top())
        m_displayUnit.setY(m_displayUnit.y() * -1.f);

    m_displayUnit.setZ(1.f);

    ResetTrafos();
    ResetAttributes();
}


DisplayManager::~DisplayManager(void) {
}


void DisplayManager::Flush(void) {
    update();
}


void DisplayManager::Show(void) {
    Redraw();
}


void DisplayManager::Redraw(void) {
    m_updateDisplayList = true;

    update();
}


void DisplayManager::FitToWindow(void) {
    m_paintAction = PaintAction::Fit;
}


void DisplayManager::SetToXYPlane(void) {
    m_paintAction = PaintAction::XyFit;
}


void DisplayManager::SetToXZPlane(void) {
    m_paintAction = PaintAction::XzFit;
}


void DisplayManager::SetToYZPlane(void) {
    m_paintAction = PaintAction::YzFit;
}


void DisplayManager::Zoom
(
    const QPoint& corner,
    const QPoint& diagonalCorner
) {
    QPoint minCorner(std::min(corner.x(), diagonalCorner.x()), std::min(corner.y(), diagonalCorner.y()));
    QPoint maxCorner(std::max(corner.x(), diagonalCorner.x()), std::max(corner.y(), diagonalCorner.y()));

    double    fX         = (m_displayMax.x() - m_displayMin.x()) / (maxCorner.x() - minCorner.x());
    double    fY         = (m_displayMax.y() - m_displayMin.y()) / (maxCorner.y() - minCorner.y());
    QVector3D zoomCentre = Display2Model((minCorner + maxCorner) / 2.);

    ShiftOnDisplay(zoomCentre);
    ScaleOnDisplay(std::min(fX, fY));

    QVector3D deviceCentre = Display2Model((m_displayMin + m_displayMax) / 2.f);
    ShiftOnDisplay(deviceCentre - zoomCentre);
}


void DisplayManager::Zoom
(
    const QPoint& centre,
    double        scale
) {
    QVector3D zoomCentre = Display2Model(centre);

    ShiftOnDisplay(zoomCentre);
    ScaleOnDisplay(scale);

    QVector3D deviceCentre = Display2Model((m_displayMin + m_displayMax) / 2.f);
    ShiftOnDisplay(deviceCentre - zoomCentre);
}


void DisplayManager::Shift
(
    const QPoint& from,
    const QPoint& to
) {
    QVector3D modelFrom = Display2Model(from);
    QVector3D modelTo   = Display2Model(to);

    ShiftOnDisplay(modelTo - modelFrom);
}


void DisplayManager::ArcRotate
(
    const QPoint& from,
    const QPoint& to
) {
    QPoint eye    = Model2Display(EyePoint());
    QPoint target = Model2Display(TargetPoint());
    double radius = Distance(eye, target);
    double delta  = Distance(from, to);

    if ((radius > SmallFloat) && (delta > SmallFloat)) {
        QPoint diff      = to - from;
        QPoint localFrom = from - target;
        QPoint localTo   = to - target;
        double dFrom     = Length(localFrom);
        double dTo       = Length(localTo);
        double dRadius   = fabs(dFrom - dTo);
        double factor    = dRadius / delta;
        double xTurn     = diff.y() * factor / radius;
        double yTurn     = diff.x() * factor / radius;
        double zTurn     = 0.;
        double tmp       = 2. * dFrom * dTo;

        if (tmp > SmallFloat) {
            double dDiff       = Length(diff);
            double cosZTurn    = std::min(std::max((dFrom * dFrom + dTo * dTo - dDiff * dDiff) / tmp, -1.), 1.);
            double orientation = localFrom.x() * localTo.y() - localFrom.y() * localTo.x();

            zTurn = acos(cosZTurn) * std::min((dFrom + dTo) / (2. * radius), 1.);

            if (orientation > 0)
                zTurn *= -1.;
        }

        RotateOnDisplay(TargetPoint(), xTurn, yTurn, zTurn);
    }
}


GeometryModel* DisplayManager::SetModel
(
    GeometryModel* geometryModel
) {
    GeometryModel* ret = m_model;

    m_model = geometryModel;

    return ret;
}


void DisplayManager::Draw(void) {
    for (std::list<Geometry*>::const_iterator it = m_model->Begin(); it != m_model->End(); ++it) {
        if (*it != 0)
            (*it)->Draw(*this);
    }
}


void DisplayManager::ModelMinMax
(
    QVector3D& minCorner,
    QVector3D& maxCorner
) const {
    for (std::list<Geometry*>::const_iterator it = m_model->Begin(); it != m_model->End(); ++it) {
        if (*it != 0)
            (*it)->MinMax(minCorner, maxCorner);
    }
}


void DisplayManager::initializeGL(void) {
    initializeOpenGLFunctions();
    glClearColor(1.f, 1.f, 1.f, 1.f);
    SetDisplayProjection();
    glColor4f(0.5f, 0.5f, 0.5f, 1.f);
    glEnable(GL_NORMALIZE);

    float lightColor[] = {1.f, 1.f, 1.f, 1.f};

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_displayListId = glGenLists(1);
}


void DisplayManager::paintGL(void) {
    if (m_updateDisplayList) {
        m_updateDisplayList = false;

        glNewList(m_displayListId, GL_COMPILE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColor4f(0.5f, 0.5f, 0.5f, 1.f);

        QMatrix4x4 inverse = m_trafoStack[Devicemm2Device].inverted();
        m_trafoStack[Devicemm2Device] = QMatrix4x4();
        m_trafoStack[Devicemm2Device].scale(m_displayUnit);
        PropagateTrafo(Devicemm2Device, inverse);

        m_setDisplayAttributes = true;

        if (m_paintAction == PaintAction::XyFit) {
            ResetTrafos();
            ResetAttributes();

            QVector3D minCorner(MaxFloat, MaxFloat, MaxFloat);
            QVector3D maxCorner(-MaxFloat, -MaxFloat, -MaxFloat);
            ModelMinMax(minCorner, maxCorner);

            if (minCorner.x() > maxCorner.x()) {
                minCorner.setX(0.f);
                maxCorner.setX(0.f);
            }

            if (minCorner.y() > maxCorner.y()) {
                minCorner.setY(0.f);
                maxCorner.setY(0.f);
            }

            if (minCorner.z() > maxCorner.z()) {
                minCorner.setZ(0.f);
                maxCorner.setZ(0.f);
            }

            QVector3D modelCenter = (minCorner + maxCorner) / 2.f;
            QVector3D modelSize   = maxCorner - minCorner;
            QVector3D eyePoint(modelCenter.x(), modelCenter.y(), maxCorner.z());
            QVector3D targetPoint = modelCenter;

            if (modelSize.z() == 0.f)
                targetPoint.setZ(targetPoint.z() - 1.f);

            EyePoint(eyePoint);
            TargetPoint(targetPoint);

            QVector3D minModel = Display2Model(m_displayMin);
            QVector3D maxModel = Display2Model(m_displayMax);

            if (minModel.x() > maxModel.x()) {
                double temp = minModel.x();

                minModel.setX(maxModel.x());
                maxModel.setX(temp);
            }

            if (minModel.y() > maxModel.y()) {
                double temp = minModel.y();

                minModel.setY(maxModel.y());
                maxModel.setY(temp);
            }

            QVector3D center = (minModel + maxModel) / 2.f;
            double    dX     = maxModel.x() - minModel.x();
            double    dY     = maxModel.y() - minModel.y();
            double    fX     = 1.;
            double    fY     = 1.;

            if (modelSize.x() > SmallFloat)
                fX = dX / modelSize.x();

            if (modelSize.y() > SmallFloat)
                fY = dY / modelSize.y();

            ShiftOnDisplay(center);
            ScaleOnDisplay(std::min(fX, fY));
            ShiftOnDisplay(-eyePoint);
        }
        else if (m_paintAction == PaintAction::XzFit) {
            ResetTrafos();
            ResetAttributes();

            QVector3D minCorner(MaxFloat, MaxFloat, MaxFloat);
            QVector3D maxCorner(-MaxFloat, -MaxFloat, -MaxFloat);
            ModelMinMax(minCorner, maxCorner);

            if (minCorner.x() > maxCorner.x()) {
                minCorner.setX(0.f);
                maxCorner.setX(0.f);
            }

            if (minCorner.y() > maxCorner.y()) {
                minCorner.setY(0.f);
                maxCorner.setY(0.f);
            }

            if (minCorner.z() > maxCorner.z()) {
                minCorner.setZ(0.f);
                maxCorner.setZ(0.f);
            }

            QVector3D modelCenter = (minCorner + maxCorner) / 2.f;
            QVector3D modelSize   = maxCorner - minCorner;
            QVector3D eyePoint(modelCenter.x(), minCorner.y(), modelCenter.z());
            QVector3D targetPoint = modelCenter;

            if (modelSize.y() == 0.f)
                targetPoint.setY(targetPoint.y() + 1.f);

            EyePoint(eyePoint);
            TargetPoint(targetPoint);

            QVector3D minModel = Display2Model(m_displayMin);
            QVector3D maxModel = Display2Model(m_displayMax);

            if (minModel.x() > maxModel.x()) {
                double temp = minModel.x();

                minModel.setX(maxModel.x());
                maxModel.setX(temp);
            }

            if (minModel.z() > maxModel.z()) {
                double temp = minModel.z();

                minModel.setZ(maxModel.z());
                maxModel.setZ(temp);
            }

            QVector3D center = (minModel + maxModel) / 2.f;
            double    dX     = maxModel.x() - minModel.x();
            double    dZ     = maxModel.z() - minModel.z();
            double    fX     = 1.;
            double    fZ     = 1.;

            if (modelSize.x() > SmallFloat)
                fX = dX / modelSize.x();

            if (modelSize.z() > SmallFloat)
                fZ = dZ / modelSize.z();

            ShiftOnDisplay(center);
            ScaleOnDisplay(std::min(fX, fZ));
            ShiftOnDisplay(-eyePoint);
        }
        else if (m_paintAction == PaintAction::YzFit) {
            ResetTrafos();
            ResetAttributes();

            QVector3D minCorner(MaxFloat, MaxFloat, MaxFloat);
            QVector3D maxCorner(-MaxFloat, -MaxFloat, -MaxFloat);
            ModelMinMax(minCorner, maxCorner);

            if (minCorner.x() > maxCorner.x()) {
                minCorner.setX(0.f);
                maxCorner.setX(0.f);
            }

            if (minCorner.y() > maxCorner.y()) {
                minCorner.setY(0.f);
                maxCorner.setY(0.f);
            }

            if (minCorner.z() > maxCorner.z()) {
                minCorner.setZ(0.f);
                maxCorner.setZ(0.f);
            }

            QVector3D modelCenter = (minCorner + maxCorner) / 2.f;
            QVector3D modelSize   = maxCorner - minCorner;
            QVector3D eyePoint(maxCorner.x(), modelCenter.y(), modelCenter.z());
            QVector3D targetPoint = modelCenter;

            if (modelSize.x() == 0.f)
                targetPoint.setX(targetPoint.x() - 1.f);

            EyePoint(eyePoint);
            TargetPoint(targetPoint);

            QVector3D minModel = Display2Model(m_displayMin);
            QVector3D maxModel = Display2Model(m_displayMax);

            if (minModel.y() > maxModel.y()) {
                double temp = minModel.y();

                minModel.setY(maxModel.y());
                maxModel.setY(temp);
            }

            if (minModel.z() > maxModel.z()) {
                double temp = minModel.z();

                minModel.setZ(maxModel.z());
                maxModel.setZ(temp);
            }

            QVector3D center = (minModel + maxModel) / 2.f;
            double    dY     = maxModel.y() - minModel.y();
            double    dZ     = maxModel.z() - minModel.z();
            double    fY     = 1.;
            double    fZ     = 1.;

            if (modelSize.y() > SmallFloat)
                fY = dY / modelSize.y();

            if (modelSize.z() > SmallFloat)
                fZ = dZ / modelSize.z();

            ShiftOnDisplay(center);
            ScaleOnDisplay(std::min(fY, fZ));
            ShiftOnDisplay(-eyePoint);
        }
        else if (m_paintAction == PaintAction::Fit) {
            QVector3D minCorner(MaxFloat, MaxFloat, MaxFloat);
            QVector3D maxCorner(-MaxFloat, -MaxFloat, -MaxFloat);
            ModelMinMax(minCorner, maxCorner);

            if (minCorner.x() > maxCorner.x()) {
                minCorner.setX(0.f);
                maxCorner.setX(0.f);
            }

            if (minCorner.y() > maxCorner.y()) {
                minCorner.setY(0.f);
                maxCorner.setY(0.f);
            }

            if (minCorner.z() > maxCorner.z()) {
                minCorner.setZ(0.f);
                maxCorner.setZ(0.f);
            }

            QVector3D modelCenter     = (minCorner + maxCorner) / 2.f;
            QVector3D modelSize       = maxCorner - minCorner;
            QVector3D cameraDirection = TargetPoint() - EyePoint();

            cameraDirection.normalize();

            QVector3D eyePoint = modelCenter - cameraDirection;
            double    length   = 0.;

            if ((fabs(cameraDirection.x()) > fabs(cameraDirection.y())) && (fabs(cameraDirection.x()) > fabs(cameraDirection.z()))) {
                if (cameraDirection.x() < 0.) {
                    if (eyePoint.x() <= maxCorner.x())
                        length = (maxCorner.x() - eyePoint.x()) / cameraDirection.x() - 1.;
                }
                else {
                    if (eyePoint.x() >= minCorner.x())
                        length = (minCorner.x() - eyePoint.x()) / cameraDirection.x() - 1.;
                }
            }
            else if (fabs(cameraDirection.z()) > fabs(cameraDirection.y())) {
                if (cameraDirection.z() < 0.) {
                    if (eyePoint.z() <= maxCorner.z())
                        length = (maxCorner.z() - eyePoint.z()) / cameraDirection.z() - 1.;
                }
                else {
                    if (eyePoint.z() >= minCorner.z())
                        length = (minCorner.z() - eyePoint.z()) / cameraDirection.z() - 1.;
                }
            }
            else {
                if (cameraDirection.y() < 0.) {
                    if (eyePoint.y() <= maxCorner.y())
                        length = (maxCorner.y() - eyePoint.y()) / cameraDirection.y() - 1.;
                }
                else {
                    if (eyePoint.y() >= minCorner.y())
                        length = (minCorner.y() - eyePoint.y()) / cameraDirection.y() - 1.;
                }
            }

            eyePoint += length * cameraDirection;

            EyePoint(eyePoint);
            TargetPoint(modelCenter);

            QVector3D minModel = Display2Model(m_displayMin);
            QVector3D maxModel = Display2Model(m_displayMax);

            if (minModel.x() > maxModel.x()) {
                double temp = minModel.x();

                minModel.setX(maxModel.x());
                maxModel.setX(temp);
            }

            if (minModel.y() > maxModel.y()) {
                double temp = minModel.y();

                minModel.setY(maxModel.y());
                maxModel.setY(temp);
            }

            if (minModel.z() > maxModel.z()) {
                double temp = minModel.z();

                minModel.setZ(maxModel.z());
                maxModel.setZ(temp);
            }

            QVector3D center = (minModel + maxModel) / 2.f;
            double    dX     = maxModel.x() - minModel.x();
            double    dY     = maxModel.y() - minModel.y();
            double    fX     = 1.;
            double    fY     = 1.;

            if (modelSize.x() > SmallFloat)
                fX = dX / modelSize.x();

            if (modelSize.y() > SmallFloat)
                fY = dY / modelSize.y();

            ShiftOnDisplay(center);
            ScaleOnDisplay(std::min(fX, fY));
            ShiftOnDisplay(-eyePoint);
        }

        m_paintAction = PaintAction::None;

        Draw();
        glEndList();
    }

    glCallList(m_displayListId);
}


void DisplayManager::resizeGL
(
    int width,
    int height
) {
    m_displayMax.setX(m_displayMin.x() + width);
    m_displayMax.setY(m_displayMin.y() + height);

    SetDisplayProjection();
}


void DisplayManager::SetDisplayProjection(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(m_displayMin.x(), m_displayMax.x(), m_displayMax.y(), m_displayMin.y(), -1000000., 1000000.);
    glPushMatrix();
}


void DisplayManager::SetColor
(
    const QColor& color
) {
    float ambient[] = {static_cast<float>(color.redF() * 0.2),
                       static_cast<float>(color.greenF() * 0.2),
                       static_cast<float>(color.blueF() * 0.2),
                       static_cast<float>(color.alphaF())};
    float diffuse[] = {static_cast<float>(color.redF() * 0.8),
                       static_cast<float>(color.greenF() * 0.8),
                       static_cast<float>(color.blueF() * 0.8),
                       static_cast<float>(color.alphaF())};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
}


void DisplayManager::DrawPoint
(
    const QVector3D& point
) {
    QVector3D devicePoint = m_trafoStack.back().map(point);

    SetAttributes();

    glBegin(GL_POINTS);
    glNormal3f(0.f, 0.f, 1.f);
    glVertex3f(devicePoint.x(), devicePoint.y(), devicePoint.z());
    glEnd();
}


void DisplayManager::DrawLine
(
    const QVector3D& start,
    const QVector3D& end
) {
    QVector3D deviceStart = m_trafoStack.back().map(start);
    QVector3D deviceEnd   = m_trafoStack.back().map(end);

    SetAttributes();

    glBegin(GL_LINES);
    glNormal3f(0.f, 0.f, 1.f);
    glVertex3f(deviceStart.x(), deviceStart.y(), deviceStart.z());
    glVertex3f(deviceEnd.x(), deviceEnd.y(), deviceEnd.z());
    glEnd();
}


void DisplayManager::DrawTriangle
(
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c
) {
    QVector3D deviceA = m_trafoStack.back().map(a);
    QVector3D deviceB = m_trafoStack.back().map(b);
    QVector3D deviceC = m_trafoStack.back().map(c);

    SetAttributes();

    QVector3D normal = QVector3D::crossProduct(deviceB - deviceA, deviceC - deviceA);
    float     length = normal.length();

    if (length > SmallFloat)
        normal /= length;
    else
        normal = QVector3D(0.f, 0.f, 1.f);

    if (normal.z() < 0.f)
        normal *= -1.f;

    glBegin(GL_TRIANGLES);
    glNormal3f(normal.x(), normal.y(), normal.z());
    glVertex3f(deviceA.x(), deviceA.y(), deviceA.z());
    glVertex3f(deviceB.x(), deviceB.y(), deviceB.z());
    glVertex3f(deviceC.x(), deviceC.y(), deviceC.z());
    glEnd();
}


void DisplayManager::EyePoint
(
    const QVector3D& point
) {
    QMatrix4x4 inverse = m_trafoStack[ParallelProjection].inverted();

    if (m_trafoStack.size() > (ParallelProjection + 1)) {
        QMatrix4x4 trafo = inverse * m_trafoStack.back();

        m_eyePoint = trafo.map(point);
    }
    else
        m_eyePoint = point;

    m_trafoStack[ParallelProjection] = m_trafoStack[ParallelProjection - 1] * Projection(m_eyePoint, m_targetPoint);
    PropagateTrafo(ParallelProjection, inverse);
}


QVector3D DisplayManager::EyePoint(void) const {
    QVector3D ret = m_eyePoint;

    if (m_trafoStack.size() > (ParallelProjection + 1)) {
        QMatrix4x4 inverse  = m_trafoStack.back().inverted();
        QMatrix4x4 toActual = inverse * m_trafoStack[ParallelProjection];

        ret = toActual.map(m_eyePoint);
    }

    return ret;
}


void DisplayManager::TargetPoint
(
    const QVector3D& point
) {
    QMatrix4x4 inverse = m_trafoStack[ParallelProjection].inverted();

    if (m_trafoStack.size() > (ParallelProjection + 1)) {
        QMatrix4x4 trafo = inverse * m_trafoStack.back();

        m_targetPoint = trafo.map(point);
    }
    else
        m_targetPoint = point;

    m_trafoStack[ParallelProjection] = m_trafoStack[ParallelProjection - 1] * Projection(m_eyePoint, m_targetPoint);
    PropagateTrafo(ParallelProjection, inverse);
}


QVector3D DisplayManager::TargetPoint(void) const {
    QVector3D ret = m_targetPoint;

    if (m_trafoStack.size() > (ParallelProjection + 1)) {
        QMatrix4x4 inverse  = m_trafoStack.back().inverted();
        QMatrix4x4 toActual = inverse * m_trafoStack[ParallelProjection];

        ret = toActual.map(m_targetPoint);
    }

    return ret;
}



void DisplayManager::ShiftOnDisplay
(
    const QVector3D& vector
) {
    QMatrix4x4 inverse     = m_trafoStack[World2Devicemm].inverted();
    QMatrix4x4 vectorTrafo = inverse * m_trafoStack.back();

    m_trafoStack[World2Devicemm].translate(vectorTrafo.map(vector) - vectorTrafo.map(QVector3D(0.f, 0.f, 0.f)));
    PropagateTrafo(World2Devicemm, inverse);
}


void DisplayManager::ScaleOnDisplay
(
    const QVector3D& vector
) {
    if (vector.length() > SmallFloat) {
        QMatrix4x4 inverse     = m_trafoStack[World2Devicemm].inverted();
        QMatrix4x4 vectorTrafo = inverse * m_trafoStack.back();
        QVector3D  origin      = vectorTrafo.map(QVector3D(0.f, 0.f, 0.f));

        m_trafoStack[World2Devicemm].translate(origin);
        m_trafoStack[World2Devicemm].scale(vector);
        m_trafoStack[World2Devicemm].translate(-origin);
        PropagateTrafo(World2Devicemm, inverse);
    }
}


void DisplayManager::ScaleOnDisplay
(
    double scale
) {
    ScaleOnDisplay(QVector3D(scale, scale, scale));
}


void DisplayManager::RotateOnDisplay
(
    const QVector3D& center,
    double           rotationAroundX,
    double           rotationAroundY,
    double           rotationAroundZ
) {
    QMatrix4x4 inverse       = m_trafoStack[World2Devicemm].inverted();
    QMatrix4x4 subInverse    = m_trafoStack[World2Devicemm - 1].inverted();
    QMatrix4x4 centerTrafo   = subInverse * m_trafoStack.back();
    QVector3D  middle        = centerTrafo.map(center);
    QMatrix4x4 rotationTrafo = m_trafoStack[World2Devicemm - 1];

    rotationTrafo.translate(middle);
    rotationTrafo.rotate(rotationAroundX, 1.f, 0.f, 0.f);
    rotationTrafo.rotate(rotationAroundY, 0.f, 1.f, 0.f);
    rotationTrafo.rotate(rotationAroundZ, 0.f, 0.f, 1.f);
    rotationTrafo.translate(-middle);
    rotationTrafo               *= subInverse;
    m_trafoStack[World2Devicemm] = rotationTrafo * m_trafoStack[World2Devicemm];
    PropagateTrafo(World2Devicemm, inverse);
}


void DisplayManager::PushTrafo
(
    const QMatrix4x4& trafo
) {
    if (m_trafoStack.size() > 0)
        m_trafoStack.push_back(m_trafoStack.back() * trafo);
    else
        m_trafoStack.push_back(trafo);
}


void DisplayManager::PushDeviceMmTrafo
(
    const QVector3D& center
) {
    QMatrix4x4 trafo  = m_trafoStack[Devicemm2Device].inverted() * m_trafoStack.back();
    QVector3D  middle = trafo.map(center);

    trafo  = m_trafoStack.back().inverted();
    trafo *= m_trafoStack[Devicemm2Device];
    trafo.translate(middle);

    PushTrafo(trafo);
}


void DisplayManager::PopTrafo(void) {
    if (m_trafoStack.size() > UserDefined)
        m_trafoStack.pop_back();
}


void DisplayManager::ResetTrafos(void) {
    m_trafoStack.clear();
    m_eyePoint    = QVector3D(0.f, 0.f, 0.f);
    m_targetPoint = QVector3D(0.f, 0.f, -1.f);

    QMatrix4x4 trafo;
    trafo.scale(m_displayUnit);

    PushTrafo(trafo);
    PushTrafo(QMatrix4x4());
    PushTrafo(QMatrix4x4());
    PushTrafo(QMatrix4x4());
}


void DisplayManager::PropagateTrafo
(
    size_t            which,
    const QMatrix4x4& inverse
) {
    QMatrix4x4 trafo = m_trafoStack[which] * inverse;

    for (size_t i = which + 1; i < m_trafoStack.size(); ++i)
        m_trafoStack[i] = trafo * m_trafoStack[i];
}


void DisplayManager::PushColor
(
    const QColor& color,
    int           priority
) {
    Attribute attribute;

    if (m_attributeStack.size() > 0) {
        attribute = m_attributeStack.back();

        if (priority >= attribute.priority) {
            if (color != attribute.color) {
                attribute.color = color;
                m_setAttributes = true;
            }

            attribute.priority = priority;
        }
    }
    else {
        attribute.color    = color;
        attribute.priority = priority;
        m_setAttributes    = true;
    }

    m_attributeStack.push_back(attribute);
}


void DisplayManager::PopAttribute(void) {
    if (m_attributeStack.size() > 1) {
        Attribute oldAttribute = m_attributeStack.back();

        m_attributeStack.pop_back();

        if (oldAttribute.color != m_attributeStack.back().color)
            m_setAttributes = true;
    }
}


void DisplayManager::SetAttributes(void) {
    if (m_setAttributes && (m_attributeStack.size() > 0)) {
        SetColor(m_attributeStack.back().color);
        m_setAttributes = false;
    }
}


void DisplayManager::ResetAttributes(void) {
    m_attributeStack.clear();
    PushColor(QColor("black"));
    m_setAttributes = true;
}


QVector3D DisplayManager::Display2Model
(
    const QPoint& displayPoint
) {
    QMatrix4x4 inverse = m_trafoStack.back().inverted();

    return inverse.map(QVector3D(static_cast<float>(displayPoint.x()), static_cast<float>(displayPoint.y()), 0.f));
}


QPoint DisplayManager::Model2Display
(
    const QVector3D& modelPoint
) {
    QVector3D displayPoint = m_trafoStack.back().map(modelPoint);
    QPoint    ret(static_cast<int>(round(displayPoint.x())), static_cast<int>(round(displayPoint.y())));

    return ret;
}
