/*                         M A I N W I N D O W . H
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
/** @file MainWindow.h
 *
 *  BRL-CAD GUI:
 *      the main window class declaration
 */

#ifndef MAINWINDOW_INCLUDED
#define MAINWINDOW_INCLUDED

#include <QMainWindow>
#include <QTreeWidget>

#include <brlcad/Database/MemoryDatabase.h>

#include "DisplayManager.h"


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(const char* fileName,
               QWidget*    parent = 0);

private:
    BRLCAD::MemoryDatabase m_database;
    GeometryModel          m_model;
    DisplayManager*        m_display;
    QTreeWidget*           m_objectsTree;

    void LoadDatabase(const char* fileName);
    void FillObjectsTree(void);

private slots:
    void OpenDatabase(void);
    void FitToWindow(void);
    void SetToXYPlane(void);
    void SetToXZPlane(void);
    void SetToYZPlane(void);
    void SelectObjects(void);
};


#endif // MAINWINDOW_INCLUDED
