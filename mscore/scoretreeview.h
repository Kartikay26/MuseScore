#ifndef __STREEVIEW_H__
#define __STREEVIEW_H__

#include <QtWidgets>
#include "libmscore/score.h"
#include "scoreitemmodel.h"

namespace Ms {

class ScoreTreeView
{
    QTreeView* _treeView;
    Score* _score;

public:
    ScoreTreeView(QWidget* parent) {
        _treeView = new QTreeView(parent);
    }

    void setScore(Score* s) {
        _score = s;

        QAbstractItemModel* m = _treeView->model();
        _treeView->setModel(new ScoreItemModel(_score, _treeView));
        delete m;
    }
};

} // namespace Ms

#endif // __STREEVIEW_H__
