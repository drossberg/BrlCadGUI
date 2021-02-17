/*                      P L O T G E O M E T R Y . C P P
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
/** @file PlotGeometry.cpp
 *
 *  BRL-CAD GUI:
 *      a BRL-CAD plot (wire-frame) geometry model implementation
 */

#include "DisplayManager.h"
#include "PlotGeometry.h"


PlotGeometry::PlotGeometry(void) : Geometry(), m_vectorList() {}


PlotGeometry::PlotGeometry
(
    const PlotGeometry& original
) : Geometry(original), m_vectorList(original.m_vectorList) {}


PlotGeometry::~PlotGeometry(void) {}


Geometry* PlotGeometry::Clone(void) const {
    return new PlotGeometry(*this);
}


class DrawCallback {
public:
    DrawCallback(DisplayManager& displayManager) : m_displayManager(displayManager), m_lastPoint() {}

    bool operator()(const BRLCAD::VectorList::Element* element) {
        if (element != 0) {
            switch (element->Type()) {
                case BRLCAD::VectorList::Element::ElementType::PointDraw:
                    break;

                case BRLCAD::VectorList::Element::ElementType::PointSize:
                    break;

                case BRLCAD::VectorList::Element::ElementType::LineMove: {
                        BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::LineMove*>(element)->Point();

                        m_lastPoint = QVector3D(static_cast<float>(point.coordinates[0]), static_cast<float>(point.coordinates[1]), static_cast<float>(point.coordinates[2]));
                    }

                    break;

                case BRLCAD::VectorList::Element::ElementType::LineDraw: {
                        BRLCAD::Vector3D point    = static_cast<const BRLCAD::VectorList::LineMove*>(element)->Point();
                        QVector3D        newPoint = QVector3D(static_cast<float>(point.coordinates[0]), static_cast<float>(point.coordinates[1]), static_cast<float>(point.coordinates[2]));

                        m_displayManager.DrawLine(m_lastPoint, newPoint);
                        m_lastPoint = newPoint;
                    }

                    break;

                case BRLCAD::VectorList::Element::ElementType::LineWidth:
                    break;

                case BRLCAD::VectorList::Element::ElementType::TriangleStart:
                    break;

                case BRLCAD::VectorList::Element::ElementType::TriangleMove:
                    break;

                case BRLCAD::VectorList::Element::ElementType::TriangleDraw:
                    break;

                case BRLCAD::VectorList::Element::ElementType::TriangleEnd:
                    break;

                case BRLCAD::VectorList::Element::ElementType::TriangleVertexNormal:
                    break;

                case BRLCAD::VectorList::Element::ElementType::PolygonStart:
                    break;

                case BRLCAD::VectorList::Element::ElementType::PolygonMove:
                    break;

                case BRLCAD::VectorList::Element::ElementType::PolygonDraw:
                    break;

                case BRLCAD::VectorList::Element::ElementType::PolygonEnd:
                    break;

                case BRLCAD::VectorList::Element::ElementType::PolygonVertexNormal:
                    break;

                case BRLCAD::VectorList::Element::ElementType::DisplaySpace:
                    break;

                //case BRLCAD::VectorList::Element::ElementType::ModelSpace:
            }
        }

        return true;
    }

private:
    DisplayManager& m_displayManager;
    QVector3D       m_lastPoint;
};


void PlotGeometry::Draw
(
    DisplayManager& displayManager
) {
    DrawCallback callback(displayManager);

    m_vectorList.Iterate(callback);
}


class MinMaxCallback {
public:
    MinMaxCallback(QVector3D& minCorner,
                   QVector3D& maxCorner) : m_minCorner(minCorner), m_maxCorner(maxCorner) {}

    bool operator()(const BRLCAD::VectorList::Element* element) {
        if (element != 0) {
            switch (element->Type()) {
                case BRLCAD::VectorList::Element::ElementType::PointDraw: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::PointDraw*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::LineMove: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::LineMove*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::LineDraw: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::LineDraw*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::TriangleMove: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::TriangleMove*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::TriangleDraw: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::TriangleDraw*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::TriangleEnd: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::TriangleEnd*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::PolygonMove: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::PolygonMove*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::PolygonDraw: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::PolygonDraw*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::PolygonEnd: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::PolygonEnd*>(element)->Point();
                    AdjustMinMax(point);
                    break;
                }

                case BRLCAD::VectorList::Element::ElementType::DisplaySpace: {
                    BRLCAD::Vector3D point = static_cast<const BRLCAD::VectorList::DisplaySpace*>(element)->ReferencePoint();
                    AdjustMinMax(point);
                }
            }
        }

        return true;
    }

private:
    QVector3D& m_minCorner;
    QVector3D& m_maxCorner;

    void AdjustMinMax
    (
        const BRLCAD::Vector3D& point
    ) {
        m_minCorner.setX(std::min(m_minCorner.x(), static_cast<float>(point.coordinates[0])));
        m_minCorner.setY(std::min(m_minCorner.y(), static_cast<float>(point.coordinates[1])));
        m_minCorner.setZ(std::min(m_minCorner.z(), static_cast<float>(point.coordinates[2])));

        m_maxCorner.setX(std::max(m_maxCorner.x(), static_cast<float>(point.coordinates[0])));
        m_maxCorner.setY(std::max(m_maxCorner.y(), static_cast<float>(point.coordinates[1])));
        m_maxCorner.setZ(std::max(m_maxCorner.z(), static_cast<float>(point.coordinates[2])));
    }
};


void PlotGeometry::MinMax
(
    QVector3D& minCorner,
    QVector3D& maxCorner
) const {
    MinMaxCallback callback(minCorner, maxCorner);

    m_vectorList.Iterate(callback);
}
