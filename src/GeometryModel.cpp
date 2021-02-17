/*                      G E O M E T R Y M O D E L . C P P
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
/** @file GeometryModel.cpp
 *
 *  BRL-CAD GUI:
 *      the internal geometry data model implementation
 */

#include "GeometryModel.h"


GeometryModel::~GeometryModel(void) {
    Clear();
}


std::list<Geometry*>::const_iterator GeometryModel::Begin(void) const {
    return m_geometries.begin();
}


std::list<Geometry*>::const_iterator GeometryModel::End(void) const {
    return m_geometries.end();
}


void GeometryModel::Append
(
    const Geometry& geometry
) {
    m_geometries.push_back(geometry.Clone());
}


void GeometryModel::Append
(
    Geometry* geometry
) {
    m_geometries.push_back(geometry);
}


void GeometryModel::Clear(void) {
    for (std::list<Geometry*>::iterator it = m_geometries.begin(); it != m_geometries.end(); ++it) {
        if (*it != 0)
            delete *it;
    }

    m_geometries.clear();
}
