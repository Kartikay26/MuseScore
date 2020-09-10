#ifndef __SCORE_ACCESSIBILITY__
#define __SCORE_ACCESSIBILITY__

#include <QStatusBar>
#include <QAccessible>
#include <QAccessibleWidget>
#include "scoreview.h"

namespace  Ms {
//---------------------------------------------------------
//   AccessibleScoreView
//---------------------------------------------------------

class AccessibleScoreView : public QAccessibleWidget
{
    ScoreView* m_scoreView;
public:

    AccessibleScoreView(ScoreView* scoreView) : m_scoreView(scoreView), QAccessibleWidget(scoreView) {};

    static QAccessibleInterface* ScoreViewFactory(const QString& classname, QObject* object);
    
    int childCount() const override;
    QAccessibleInterface* child(int index) const override;
    QAccessibleInterface* parent() const override;
    QRect rect() const override;
    QAccessible::Role role() const override;
    QString text(QAccessible::Text t) const override;
};

class AccessibleScoreItem : public QAccessibleInterface
{
    Element* m_element;
    ScoreView* m_scoreView;
public:

    AccessibleScoreItem(Element* el, ScoreView* sv) : m_element(el), m_scoreView(sv) {};
    
    int childCount() const override;
    QAccessibleInterface* child(int index) const override;
    QAccessibleInterface* parent() const override;
    QRect rect() const override;
    QAccessible::Role role() const override;
    QString text(QAccessible::Text t) const override;
    QAccessibleInterface* childAt(int x, int y) const override;

    bool isValid(void) const override { return true; };
    QObject* object(void) const override { return nullptr; }
    int indexOfChild(const QAccessibleInterface* child) const override { if (auto c = dynamic_cast<const AccessibleScoreItem*>(child)) return m_element->treeChildIdx(c->m_element); else return -1; }
    void setText(QAccessible::Text,const QString &) override {}
    QAccessible::State state(void) const override { return QAccessible::State(); }
};


//---------------------------------------------------------
//   ScoreAccessibility
//---------------------------------------------------------

class ScoreAccessibility : public QObject
{
    Q_OBJECT

    static ScoreAccessibility* inst;
    QMainWindow* mainWindow;
    QLabel* statusBarLabel;
    ScoreAccessibility(QMainWindow* statusBar);
    std::pair<int, float> barbeat(Element* e);
    int _oldStaff = -1;
    int _oldBar = -1;

public:
    ~ScoreAccessibility();
    void updateAccessibilityInfo();
    void clearAccessibilityInfo();
    static void createInstance(QMainWindow* statusBar);
    static ScoreAccessibility* instance();
    void currentInfoChanged();
    static void makeReadable(QString&);

private slots:
    void updateAccessibility();
};

}

#endif
