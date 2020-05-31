#include "scoretreeview.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreTreeView
//---------------------------------------------------------

ScoreTreeView::ScoreTreeView(){
    _dock = new QDockWidget("Score Tree", mscore);
    _dock->setAllowedAreas(Qt::RightDockWidgetArea);
    _treeView = new QTreeView(_dock);
    _dock->setWidget(_treeView);
    mscore->addDockWidget(Qt::RightDockWidgetArea, _dock);
}

//---------------------------------------------------------
//   ~ScoreTreeView
//---------------------------------------------------------

ScoreTreeView::~ScoreTreeView(){
    mscore->removeDockWidget(_dock);
    delete _dock;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreTreeView::setScore(Score* s) {
    _score = s;

    QAbstractItemModel* m = _treeView->model();
    _treeView->setModel(new ScoreItemModel(_score, _treeView));
    delete m;
}

}
