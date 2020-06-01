#include "scoretreeview.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreTreeView
//---------------------------------------------------------

ScoreTreeView::ScoreTreeView()
{
    _dock = new QDockWidget("Score Tree", mscore);
    _dock->setAllowedAreas(Qt::RightDockWidgetArea);
    _treeView = new QTreeView(_dock);
    _dock->setWidget(_treeView);
    mscore->addDockWidget(Qt::RightDockWidgetArea, _dock);

    connect(_treeView, &QTreeView::clicked, this, &ScoreTreeView::clicked);
}

//---------------------------------------------------------
//   ~ScoreTreeView
//---------------------------------------------------------

ScoreTreeView::~ScoreTreeView()
{
    mscore->removeDockWidget(_dock);
    delete _dock;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreTreeView::setScore(Score* s)
{
    _score = s;

    QAbstractItemModel* m = _treeView->model();
    _treeView->setModel(new ScoreItemModel(_score, _treeView));
    delete m;
}


//---------------------------------------------------------
//   clicked
//---------------------------------------------------------

void ScoreTreeView::clicked(const QModelIndex& cur)
{
    ScoreElement* s = (ScoreElement*) cur.internalPointer();
    if (Element* e = dynamic_cast<Element*>(s)) {
        qDebug() << "selecting: " << e->name();
        _score->select(e);
        _score->update();
    }
}

}
