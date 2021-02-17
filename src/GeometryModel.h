/*                      G E O M E T R Y M O D E L . H
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
/** @file GeometryModel.h
 *
 *  BRL-CAD GUI:
 *      the internal geometry data model declaration
 */

#ifndef GEOMETRYMODEL_INCLUDED
#define GEOMETRYMODEL_INCLUDED

#include <QVector3D>


class DisplayManager;


class Geometry {
public:
    virtual ~Geometry(void) {}

    virtual Geometry* Clone(void) const                    = 0;

    virtual void      Draw(DisplayManager& displayManager) = 0;
    virtual void      MinMax(QVector3D& minCorner,
                             QVector3D& maxCorner) const   = 0;

protected:
    Geometry(void) {}
    Geometry(const Geometry& original) {}

private:
    Geometry& operator=(const Geometry& original);
};



class GeometryModel {
public:
    ~GeometryModel(void);

    std::list<Geometry*>::const_iterator Begin(void) const;
    std::list<Geometry*>::const_iterator End(void) const;

    void                                 Append(const Geometry& geometry);
    void                                 Append(Geometry* geometry);

    void                                 Clear(void);

private:
    std::list<Geometry*> m_geometries;
};


#endif // GEOMETRYMODEL_INCLUDED
