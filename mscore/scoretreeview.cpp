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

    if (Element* root = dynamic_cast<Element*>(s)) {
    
        qDebug() << "Clicked: " << root->name();
        _score->selection().deselectAll();
        _score->selection().setState(SelState::LIST);

        std::function<void(Element*)> selectChildren = [&](Element* el) -> void {
            _score->selection().add(el);
            qDebug() << "selecting: " << el->name();
            for (int i = 0; i < el->treeChildCount(); i++) {
                Element* ch = (Element*) el->treeChild(i); // Child of Element will always be Element, never ScoreElement
                selectChildren(ch);
            }
        };
        
        selectChildren(root);
        _score->setSelectionChanged(true);
        _score->update();

    }
}

}
