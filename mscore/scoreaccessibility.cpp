#include <QMainWindow>
#include <QWidget>
#include "scoreaccessibility.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/timesig.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/spanner.h"
#include "libmscore/sig.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/sym.h"
#include "inspectordockwidget.h"
#include "selectionwindow.h"
#include "playpanel.h"
#include "synthcontrol.h"
#include "mixer/mixer.h"
#include "drumroll.h"
#include "pianoroll/pianoroll.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreViewFaactory
//---------------------------------------------------------

QAccessibleInterface* AccessibleScoreView::ScoreViewFactory(const QString& classname, QObject* object)
{
    QAccessibleInterface* iface = 0;
    if (classname == QLatin1String("Ms::ScoreView") && object && object->isWidgetType()) {
        qDebug("Creating interface for ScoreView object");
        iface = static_cast<QAccessibleInterface*>(new AccessibleScoreView(static_cast<ScoreView*>(object)));
    }
    return iface;
}

//---------------------------------------------------------
//   AccessibleScoreView
//---------------------------------------------------------

int AccessibleScoreView::childCount() const
{
    return m_scoreView->score()->treeChildCount();
}

QAccessibleInterface* AccessibleScoreView::child(int index) const
{
    return new AccessibleScoreItem( toElement(m_scoreView->score()->treeChild(index)), m_scoreView );
}

QAccessibleInterface* AccessibleScoreView::parent() const
{
    return QAccessibleWidget::parent();
}

QRect AccessibleScoreView::rect() const
{
    return m_scoreView->rect();
}

QAccessible::Role AccessibleScoreView::role() const
{
    return QAccessible::Tree;
}

QString AccessibleScoreView::text(QAccessible::Text t) const
{
    switch (t) {
        case QAccessible::Name:
            return m_scoreView->score()->title();
        case QAccessible::Value:
        case QAccessible::Description:
            return m_scoreView->score()->accessibleInfo();
        default:
            return QString();
    }
}

//---------------------------------------------------------
//   AccessibleScoreItem
//---------------------------------------------------------

int AccessibleScoreItem::childCount() const
{
    return m_element->treeChildCount();
}

QAccessibleInterface* AccessibleScoreItem::child(int index) const
{
    return new AccessibleScoreItem( toElement(m_element->treeChild(index)), m_scoreView );
}

QAccessibleInterface* AccessibleScoreItem::childAt(int x, int y) const
{
    return new AccessibleScoreItem( m_scoreView->elementAt(m_scoreView->toLogical(QPoint(x, y))) , m_scoreView );
}

QAccessibleInterface* AccessibleScoreItem::parent() const
{
    // if (m_element->parent()->isScore()){
    //     return QAccessible::queryAccessibleInterface(mscore->currentScoreView());
    // }
    // return QAccessible::queryAccessibleInterface(m_element->parent());
    return nullptr;
}

QRect AccessibleScoreItem::rect() const
{
    return m_scoreView->toPhysical(m_element->bbox());
}

QAccessible::Role AccessibleScoreItem::role() const
{
    return QAccessible::TreeItem;
}

QString AccessibleScoreItem::text(QAccessible::Text t) const
{
    switch (t) {
        case QAccessible::Name:
            return m_element->userName();
        case QAccessible::Value:
        case QAccessible::Description:
            return m_element->accessibleInfo();
        default:
            return QString();
    }
}

//---------------------------------------------------------
//   ScoreAccessibility Stuff
//---------------------------------------------------------

ScoreAccessibility* ScoreAccessibility::inst = 0;

ScoreAccessibility::ScoreAccessibility(QMainWindow* mainWindow)
    : QObject(mainWindow)
{
    this->mainWindow = mainWindow;
    statusBarLabel = new QLabel(mainWindow->statusBar());
    mainWindow->statusBar()->addWidget(statusBarLabel);
}

void ScoreAccessibility::createInstance(QMainWindow* mainWindow)
{
    if (!inst) {
        inst = new ScoreAccessibility(mainWindow);
    }
}

ScoreAccessibility::~ScoreAccessibility()
{
}

void ScoreAccessibility::clearAccessibilityInfo()
{
    statusBarLabel->setText("");
    MuseScoreView* view = static_cast<MuseScore*>(mainWindow)->currentScoreView();
    if (view) {
        view->score()->setAccessibleInfo(tr("No selection"));
    }
    _oldBar = -1;
    _oldStaff = -1;
}

void ScoreAccessibility::currentInfoChanged()
{
    ScoreView* scoreView =  static_cast<MuseScore*>(mainWindow)->currentScoreView();
    Score* score = scoreView->score();
    int oldStaff = _oldStaff;
    int oldBar = _oldBar;
    _oldStaff = -1;
    _oldBar = -1;
    QString oldStatus = statusBarLabel->text();
    QString oldScreenReaderInfo = score->accessibleInfo();
    clearAccessibilityInfo();
    if (score->selection().isSingle()) {
        Element* e = score->selection().element();
        if (!e) {
            return;
        }
        Element* el = e->isSpannerSegment() ? static_cast<SpannerSegment*>(e)->spanner() : e;
        QString barsAndBeats = "";
        QString optimizedBarsAndBeats = "";
        if (el->isSpanner()) {
            Spanner* s = static_cast<Spanner*>(el);
            std::pair<int, float> bar_beat = barbeat(s->startSegment());
            barsAndBeats += tr("Start Measure: %1; Start Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(
                                                                                                                 bar_beat
                                                                                                                 .second));
            Segment* seg = s->endSegment();
            if (!seg) {
                seg = score->lastSegment()->prev1MM(SegmentType::ChordRest);
            }

            if (seg->tick() != score->lastSegment()->prev1MM(SegmentType::ChordRest)->tick()
                && s->type() != ElementType::SLUR
                && s->type() != ElementType::TIE) {
                seg = seg->prev1MM(SegmentType::ChordRest);
            }

            bar_beat = barbeat(seg);
            barsAndBeats += "; " + tr("End Measure: %1; End Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(
                                                                                                                    bar_beat
                                                                                                                    .
                                                                                                                    second));
            optimizedBarsAndBeats = barsAndBeats;
        } else {
            std::pair<int, float> bar_beat = barbeat(el);
            if (bar_beat.first) {
                _oldBar = bar_beat.first;
                barsAndBeats += " " + tr("Measure: %1").arg(QString::number(bar_beat.first));
                if (bar_beat.first != oldBar) {
                    optimizedBarsAndBeats += " " + tr("Measure: %1").arg(QString::number(bar_beat.first));
                }
                if (bar_beat.second) {
                    barsAndBeats += "; " + tr("Beat: %1").arg(QString::number(bar_beat.second));
                    optimizedBarsAndBeats += "; " + tr("Beat: %1").arg(QString::number(bar_beat.second));
                }
            }
        }

        QString rez = e->accessibleInfo();
        if (!barsAndBeats.isEmpty()) {
            rez += "; " + barsAndBeats;
        } else {
            oldScreenReaderInfo.clear();        // force regeneration for elements with no barbeat info - see below
        }
        QString staff = "";
        QString optimizedStaff = "";
        if (e->staffIdx() + 1) {
            _oldStaff = e->staffIdx();
            staff = tr("Staff: %1").arg(QString::number(e->staffIdx() + 1));
            QString staffName = e->staff()->part()->longName(e->tick());
            if (staffName.isEmpty()) {
                staffName = e->staff()->partName();
            }
            if (staffName.isEmpty()) {
                staffName = tr("Unnamed");            // for screenreader only
                rez = QString("%1; %2").arg(rez).arg(staff);
            } else {
                rez = QString("%1; %2 (%3)").arg(rez).arg(staff).arg(staffName);
            }
            if (e->staffIdx() != oldStaff) {
                optimizedStaff = QString("%1 (%2)").arg(staff).arg(staffName);
            }
        }

        statusBarLabel->setText(rez);

        if (scoreView->mscoreState() & STATE_ALLTEXTUAL_EDIT) {
            // Don't say element name during text editing.
            score->setAccessibleInfo("");
            return;
        }

        QString screenReaderRez;
        QString newScreenReaderInfo = e->screenReaderInfo();
        if (rez != oldStatus || newScreenReaderInfo != oldScreenReaderInfo || oldScreenReaderInfo.isEmpty()) {
            // status has changed since last call, or there is no existing screenreader info
            //
            // build new screenreader info
            screenReaderRez = QString("%1%2 %3 %4").arg(newScreenReaderInfo).arg(optimizedBarsAndBeats).arg(
                optimizedStaff).arg(e->accessibleExtraInfo());
            makeReadable(screenReaderRez);
        } else {
            // status has not changed since last call, and there is existing screenreader info
            //
            // if this function is called twice within the same command,
            // then status does not change between calls,
            // but the second call may result in too much information being optimized away for screenreader
            // so in these cases, let the previous screenreader info stand
            // (this is relevant only for elements with bar and beat info)
            screenReaderRez = oldScreenReaderInfo;
        }
        score->setAccessibleInfo(screenReaderRez);
    } else if (score->selection().isRange()) {
        QString barsAndBeats = "";
        std::pair<int, float> bar_beat;

        bar_beat = barbeat(score->selection().startSegment());
        barsAndBeats += " " + tr("Start Measure: %1; Start Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(
                                                                                                                   bar_beat
                                                                                                                   .
                                                                                                                   second));
        Segment* endSegment = score->selection().endSegment();

        if (!endSegment) {
            endSegment = score->lastSegment();
        } else {
            endSegment = endSegment->prev1MM();
        }

        bar_beat = barbeat(endSegment);
        barsAndBeats += " " + tr("End Measure: %1; End Beat: %2").arg(QString::number(bar_beat.first)).arg(QString::number(
                                                                                                               bar_beat.
                                                                                                               second));
        statusBarLabel->setText(tr("Range Selection") + barsAndBeats);
        score->setAccessibleInfo(tr("Range Selection") + barsAndBeats);
    } else if (score->selection().isList()) {
        statusBarLabel->setText(tr("List Selection"));
        score->setAccessibleInfo(tr("List Selection"));
    }
}

ScoreAccessibility* ScoreAccessibility::instance()
{
    return inst;
}

void ScoreAccessibility::updateAccessibilityInfo()
{
    ScoreView* w = static_cast<MuseScore*>(mainWindow)->currentScoreView();
    if (!w) {
        return;
    }

    currentInfoChanged();

    //getInspector->isAncestorOf is used so that inspector and search dialog don't loose focus
    //when this method is called
    //TODO: create a class to manage focus and replace this massive if
    QWidget* focusWidget = qApp->focusWidget();
    if ((focusWidget != w)
        && !(mscore->inspector() && mscore->inspector()->isAncestorOf(focusWidget))
        && !(mscore->searchDialog() && mscore->searchDialog()->isAncestorOf(focusWidget))
        && !(mscore->getSelectionWindow() && mscore->getSelectionWindow()->isAncestorOf(focusWidget))
        && !(mscore->getPlayPanel() && mscore->getPlayPanel()->isAncestorOf(focusWidget))
        && !(mscore->getSynthControl() && mscore->getSynthControl()->isAncestorOf(focusWidget))
        && !(mscore->getMixer() && mscore->getMixer()->isAncestorOf(focusWidget))
        && !(mscore->searchDialog() && mscore->searchDialog()->isAncestorOf(focusWidget))
        && !(mscore->getDrumrollEditor() && mscore->getDrumrollEditor()->isAncestorOf(focusWidget))
        && !(mscore->getPianorollEditor() && mscore->getPianorollEditor()->isAncestorOf(focusWidget))) {
        mscore->activateWindow();
        w->setFocus();
    }
#if 0
    else if (focusWidget == w) {
        w->clearFocus();
        w->setFocus();
    }
#endif

    // Try to send message to the screen reader. Note that NVDA will
    // ignore the message if it is the same as the previous message.
    updateAccessibility();

#if defined(Q_OS_WIN)
    // HACK: send the message again after a short delay to force NVDA
    // to read it even if it is the same as before. This is useful when
    // cursoring through a word with repeated characters, such as "food".
    // Without this hack NVDA would say "f", "o", *silence*, "d".
    QTimer::singleShot(0, this, &ScoreAccessibility::updateAccessibility);
#endif
}

void ScoreAccessibility::updateAccessibility()
{
    ScoreView* w = static_cast<MuseScore*>(mainWindow)->currentScoreView();
    if (!w) {
        return;
    }

    QObject* obj = static_cast<QObject*>(w);
    // QAccessibleValueChangeEvent vcev(obj, w->score()->accessibleInfo());
    // QAccessible::updateAccessibility(&vcev);
    // TODO:
    // some screenreaders may respond better to other events
    // the version of Qt used may also be relevant, and platform too
    //QAccessibleEvent ev1(obj, QAccessible::NameChanged);
    //QAccessible::updateAccessibility(&ev1);
    //QAccessibleEvent ev2(obj, QAccessible::DescriptionChanged);
    //QAccessible::updateAccessibility(&ev2);
}

std::pair<int, float> ScoreAccessibility::barbeat(Element* e)
{
    if (!e) {
        return std::pair<int, float>(0, 0.0F);
    }

    int bar = 0;
    int beat = 0;
    int ticks = 0;
    TimeSigMap* tsm = e->score()->sigmap();
    Element* p = e;
    int ticksB = ticks_beat(tsm->timesig(0).timesig().denominator());
    while (p && p->type() != ElementType::SEGMENT && p->type() != ElementType::MEASURE) {
        p = p->parent();
    }

    if (!p) {
        return std::pair<int, float>(0, 0.0F);
    } else if (p->type() == ElementType::SEGMENT) {
        Segment* seg = static_cast<Segment*>(p);
        tsm->tickValues(seg->tick().ticks(), &bar, &beat, &ticks);
        ticksB = ticks_beat(tsm->timesig(seg->tick().ticks()).timesig().denominator());
    } else if (p->type() == ElementType::MEASURE) {
        Measure* m = static_cast<Measure*>(p);
        bar = m->no();
        beat = -1;
        ticks = 0;
    }
    return std::pair<int,float>(bar + 1, beat + 1 + ticks / static_cast<float>(ticksB));
}

void ScoreAccessibility::makeReadable(QString& s)
{
    static std::vector<std::pair<QString, QString> > unicodeReplacements {
        { "♭", " " + tr("flat") },
        { "♮", " " + tr("natural") },
        { "♯", " " + tr("sharp") },
        { "𝄫", " " + tr("double flat") },
        { "𝄪", " " + tr("double sharp") },
    };

    if (!QAccessible::isActive()) {
        return;
    }
    for (auto const& r : unicodeReplacements) {
        s.replace(r.first, r.second);
    }
    ScoreFont* sf = gscore->scoreFont();
    for (auto id : Sym::commonScoreSymbols) {
        if (id == SymId::space) {
            continue;                     // don't read "space"
        }
        QString src = sf->toString(id);
        QString replacement = Sym::id2userName(id);
        s.replace(src, replacement);
    }
}

}
