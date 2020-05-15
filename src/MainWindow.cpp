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

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>

#include "MainWindow.h"


MainWindow::MainWindow
(
    const char* fileName,
    QWidget*    parent
) : QMainWindow(parent),
    m_database() {
    setWindowTitle(tr("BRL-CAD GUI"));

    QAction* dbOpenAction = new QAction(tr("Open database"));
    dbOpenAction->setShortcuts(QKeySequence::Open);
    dbOpenAction->setToolTip(tr("Open an existing .g database file"));
    connect(dbOpenAction, &QAction::triggered,
            this,         &MainWindow::OpenDatabase);

    QAction* exitAction = new QAction(tr("Exit"));
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setToolTip(tr("Terminates the program"));
    connect(exitAction, &QAction::triggered,
            qApp,       &QApplication::closeAllWindows);

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(dbOpenAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

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


void MainWindow::OpenDatabase(void) {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open BRL-CAD .g database file"),
                                                    QString(),
                                                    "BRL-CAD database file (*.g)");

    LoadDatabase(fileName.toUtf8().data());
}
