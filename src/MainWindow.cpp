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
#include <QDockWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>

#include <brlcad/Database/Combination.h>

#include "PlotGeometry.h"
#include "MainWindow.h"


// static helpers
class SubObjectCallback;

static void WalkTree(const BRLCAD::Combination::ConstTreeNode& tree,
                     const BRLCAD::ConstDatabase&              database,
                     SubObjectCallback&                        callback);


class SubObjectCallback {
public:
    SubObjectCallback(QTreeWidgetItem*       treeItem,
                      BRLCAD::ConstDatabase& database) : m_treeItem(treeItem),
                                                         m_database(database) {}

    void operator()(const BRLCAD::Object& object) {
        QTreeWidgetItem* treeItem = new QTreeWidgetItem(m_treeItem);
        treeItem->setText(0, QString::fromUtf8(object.Name()));

        const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(&object);

        if (combination != 0) {
            SubObjectCallback subObjectCallback(treeItem, m_database);

            WalkTree(combination->Tree(), m_database, subObjectCallback);
        }
    }

private:
    QTreeWidgetItem*       m_treeItem;
    BRLCAD::ConstDatabase& m_database;
};


class TopObjectCallback {
public:
    TopObjectCallback(QTreeWidget*           tree,
                      BRLCAD::ConstDatabase& database) : m_tree(tree),
                                                         m_database(database) {}

    void operator()(const BRLCAD::Object& object) {
        QTreeWidgetItem* treeItem = new QTreeWidgetItem(m_tree);
        treeItem->setText(0, QString::fromUtf8(object.Name()));

        const BRLCAD::Combination* combination = dynamic_cast<const BRLCAD::Combination*>(&object);

        if (combination != 0) {
            SubObjectCallback subObjectCallback(treeItem, m_database);

            WalkTree(combination->Tree(), m_database, subObjectCallback);
        }
    }

private:
    QTreeWidget*           m_tree;
    BRLCAD::ConstDatabase& m_database;
};


static void WalkTree
(
    const BRLCAD::Combination::ConstTreeNode& tree,
    const BRLCAD::ConstDatabase&              database,
    SubObjectCallback&                        callback
) {
    switch (tree.Operation()) {
        case BRLCAD::Combination::ConstTreeNode::Union:
        case BRLCAD::Combination::ConstTreeNode::Intersection:
        case BRLCAD::Combination::ConstTreeNode::Subtraction:
        case BRLCAD::Combination::ConstTreeNode::ExclusiveOr:
            WalkTree(tree.LeftOperand(), database, callback);
            WalkTree(tree.RightOperand(), database, callback);
            break;

        case BRLCAD::Combination::ConstTreeNode::Not:
            WalkTree(tree.Operand(), database, callback);
            break;

        case BRLCAD::Combination::ConstTreeNode::Leaf:
            database.Get(tree.Name(), callback);
    }
}


// MainWindow
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

    // the display
    m_display = new DisplayManager(this);
    m_display->SetModel(&m_model);
    setCentralWidget(m_display);

    // objects' tree
    QDockWidget* objectsDock = new QDockWidget(tr("Database object tree"));
    m_objectsTree = new QTreeWidget();
    m_objectsTree->setRootIsDecorated(true);
    m_objectsTree->setColumnCount(1);
    m_objectsTree->header()->hide();
    connect(m_objectsTree, &QTreeWidget::itemSelectionChanged,
            this,          &MainWindow::SelectObjects);

    objectsDock->setWidget(m_objectsTree);
    addDockWidget(Qt::LeftDockWidgetArea, objectsDock);

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
        FillObjectsTree();
    }
}


void MainWindow::FillObjectsTree(void) {
    m_objectsTree->clear();

    BRLCAD::ConstDatabase::TopObjectIterator topObjectIterator = m_database.FirstTopObject();

    while (topObjectIterator.Good()) {
        TopObjectCallback topObjectCallback(m_objectsTree, m_database);

        m_database.Get(topObjectIterator.Name(), topObjectCallback);
        ++topObjectIterator;
    }
}


void MainWindow::OpenDatabase(void) {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open BRL-CAD .g database file"),
                                                    QString(),
                                                    "BRL-CAD database file (*.g)");

    LoadDatabase(fileName.toUtf8().data());
}


void MainWindow::SelectObjects(void) {
    QList<QTreeWidgetItem*> selectedItems = m_objectsTree->selectedItems();

    m_database.UnSelectAll();
    m_model.Clear();

    for (QList<QTreeWidgetItem*>::const_iterator it = selectedItems.begin(); it != selectedItems.end(); ++it) {
        QByteArray objectName = (*it)->text(0).toUtf8();
        PlotGeometry* plot = new PlotGeometry();

        m_database.Select(objectName);
        m_database.Plot(objectName, plot->VectorList());
        m_model.Append(plot);
    }

    m_display->FitToWindow();
    m_display->Redraw();
}
