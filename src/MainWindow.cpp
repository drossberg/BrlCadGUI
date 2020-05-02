/*                       M A I N W I N D O W . C P P
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
/** @file MainWindow.cpp
 *
 *  BRL-CAD GUI:
 *      the main window class implementation
 */

#include "MainWindow.h"


MainWindow::MainWindow
(
    const char* fileName,
    QWidget*    parent
) : QMainWindow(parent),
    m_database() {
    setWindowTitle(tr("BRL-CAD GUI"));

    if (fileName != 0)
        LoadDatabase(fileName);
}


void MainWindow::LoadDatabase
(
    const char* fileName
) {
    if (m_database.Load(fileName))  {
        QString title = m_database.Title();

        title += " [";
        title += fileName;
        title += "]";

        setWindowTitle(title);
    }
}
