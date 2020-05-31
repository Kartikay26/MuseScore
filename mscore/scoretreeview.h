#ifndef __STREEVIEW_H__
#define __STREEVIEW_H__

#include <QtWidgets>
#include "musescore.h"
#include "libmscore/score.h"
#include "scoreitemmodel.h"

namespace Ms {

class ScoreTreeView
{
    QDockWidget* _dock;
    QTreeView* _treeView;
    Score* _score;

public:
    ScoreTreeView();
    ~ScoreTreeView();
    void setScore(Score* s);
};

} // namespace Ms

#endif // __STREEVIEW_H__
