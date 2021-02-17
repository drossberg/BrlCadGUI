/*                      P L O T G E O M E T R Y . H
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
/** @file PlotGeometry.h
 *
 *  BRL-CAD GUI:
 *      a BRL-CAD plot (wire-frame) geometry model declaration
 */

#ifndef PLOTGEOMETRY_INCLUDED
#define PLOTGEOMETRY_INCLUDED

#include <brlcad/VectorList.h>

#include "GeometryModel.h"


class PlotGeometry : public Geometry {
public:
    PlotGeometry(void);
    PlotGeometry(const PlotGeometry& original);
    virtual ~PlotGeometry(void);

    virtual Geometry*         Clone(void) const;

    virtual void              Draw(DisplayManager& displayManager);
    virtual void              MinMax(QVector3D& minCorner,
                                     QVector3D& maxCorner) const;

    const BRLCAD::VectorList& VectorList(void) const {
        return m_vectorList;
    }

    BRLCAD::VectorList&       VectorList(void) {
        return m_vectorList;
    }

private:
    BRLCAD::VectorList m_vectorList;

    PlotGeometry& operator=(const PlotGeometry& original);
};


#endif // PLOTGEOMETRY_INCLUDED
