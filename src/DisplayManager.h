/*                      D I S P L A Y M A N A G E R . H
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
/** @file DisplayManager.h
 *
 *  BRL-CAD GUI:
 *      the display functions class declaration
 */

#ifndef DISPLAYMANAGER_INCLUDED
#define DISPLAYMANAGER_INCLUDED

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>

#include "GeometryModel.h"


class DisplayManager : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    DisplayManager(QWidget* parent = 0);

    ~DisplayManager(void);

    // widget handling
    void Flush(void);  // update, nothing has changed
    void Show(void);   // update, projection has changed
    void Redraw(void); // regenerate, model has changed

    // set projection
    void FitToWindow(void);
    void SetToXYPlane(void);
    void SetToXZPlane(void);
    void SetToYZPlane(void);

    void Zoom(const QPoint& corner,
              const QPoint& diagonalCorner);
    void Zoom(const QPoint& centre,
              double        scale);
    void Shift(const QPoint& from,
               const QPoint& to);
    void ArcRotate(const QPoint& from,
                   const QPoint& to);

    // model operations
    GeometryModel* SetModel(GeometryModel* geometryModel);
    void           Draw(void);
    void           ModelMinMax(QVector3D& minCorner,
                               QVector3D& maxCorner) const;

protected:
    void initializeGL(void);
    void paintGL(void);
    void resizeGL(int width,
                  int height);

private:
    QPoint                  m_displayMin;
    QPoint                  m_displayMax;
    QVector3D               m_displayUnit;
    GLuint                  m_displayListId;
    bool                    m_updateDisplayList;

    QVector3D               m_eyePoint;
    QVector3D               m_targetPoint;

    enum class PaintAction {
        None,
        Fit,
        XyFit,
        XzFit,
        YzFit
    };

    PaintAction             m_paintAction;

    std::vector<QMatrix4x4> m_trafoStack;

    enum TrafoPosition {
        Devicemm2Device    = 0,
        World2Devicemm     = 1,
        CentralProjection  = 2,
        ParallelProjection = 3,
        UserDefined        = 4
    };

    bool                    m_setDisplayAttributes;

    GeometryModel*          m_model;

public:
    // device
    void      SetDisplayProjection(void);

    void      SetColor(const QColor& color);
    void      DrawPoint(const QVector3D& point);
    void      DrawLine(const QVector3D& start,
                       const QVector3D& end);
    void      DrawTriangle(const QVector3D& a,
                           const QVector3D& b,
                           const QVector3D& c);

    // projection
    void      EyePoint(const QVector3D& point);
    QVector3D EyePoint(void) const;
    void      TargetPoint(const QVector3D& point);
    QVector3D TargetPoint(void) const;
    void      ShiftOnDisplay(const QVector3D& vector);
    void      ScaleOnDisplay(const QVector3D& vector);
    void      ScaleOnDisplay(double scale);
    void      RotateOnDisplay(const QVector3D& center,
                              double           rotationAroundX,
                              double           rotationAroundY,
                              double           rotationAroundZ);

    void      PushTrafo(const QMatrix4x4& trafo);
    void      PushDeviceMmTrafo(const QVector3D& center);
    void      PopTrafo(void);
    void      ResetTrafos(void);

    void      PropagateTrafo(size_t            which,
                             const QMatrix4x4& inverse);

    struct Attribute {
        QColor color;
        int    priority;
    };

    std::vector<Attribute> m_attributeStack;
    bool                   m_setAttributes;

    void      PushColor(const QColor& color,
                        int           priority = 0);
    void      PopAttribute(void);
    void      SetAttributes(void);
    void      ResetAttributes(void);

    // reverse ingeneering
    QVector3D Display2Model(const QPoint& displayPoint);
    QPoint    Model2Display(const QVector3D& modelPoint);
};


#endif // DISPLAYMANAGER_INCLUDED
