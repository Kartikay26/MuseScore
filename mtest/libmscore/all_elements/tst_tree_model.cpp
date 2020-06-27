//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"

#define DIR QString("libmscore/all_elements/")

using namespace Ms;

//---------------------------------------------------------
//   TestTreeModel
///   Ensures that the tree model is consistent. Starting
///   from Score each element in the tree should have the
///   correct parent element, the one whose children list
///   it appears in. Also ensures that all the elements
///   scanned by scanElementsOld appear in the tree model.
//---------------------------------------------------------

class TestTreeModel : public QObject, public MTest
{
    Q_OBJECT

    MasterScore * score;

    void tstTree(QString file);
    void traverseTree(ScoreElement* element);

private slots:
    void initTestCase();
    void tstTreeElements() { tstTree("layout_elements.mscx"); }
    void tstTreeTablature() { tstTree("layout_elements_tab.mscx"); }
    void tstTreeMoonlight() { tstTree("moonlight.mscx"); }
    void tstTreeGoldberg() { tstTree("goldberg.mscx"); }

public:
    std::set<ScoreElement*> visitedElements;
};

QString elementToText(ScoreElement* element);

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTreeModel::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   tstTree
//---------------------------------------------------------

void scan(void*, Element*);

void TestTreeModel::tstTree(QString file)
{
    MasterScore* score = readScore(DIR + file);
    traverseTree(score);
    score->scanElementsOld(this, &scan, true);
}

void scan(void* data, Element* el)
{
    TestTreeModel* tstClass = static_cast<TestTreeModel*>(data);
    Q_UNUSED(data);
    // ensure el has been visited, if not print debugging info and exit
    if (!tstClass->visitedElements.count(el)) {
        qDebug() << "Element not visited! " << elementToText(el);
        qDebug() << "Parent in model: " << elementToText(el->treeParent());
        qDebug() << "Parent (in elements): " << elementToText(el->parent());
        qDebug() << "Heirarchy";
        ScoreElement* e = el;
        while (e->treeParent()) {
            qDebug() << elementToText(e);
            e = e->treeParent();
        }
    }
    Q_ASSERT(tstClass->visitedElements.count(el));
}

//---------------------------------------------------------
//   traverseTree
///   Checks whether parent element of current element is
///   correct, then recursively checks all children. Also
///   stores the element in visitedElements.
//---------------------------------------------------------

void TestTreeModel::traverseTree(ScoreElement* element)
{
    visitedElements.insert(element);

    for (ScoreElement* child : (*element)) {
        if (!child) {
            continue;
        }

        // if parent is not correct print some logging info and exit
        if (child->treeParent() != element) {
            qDebug() << "Element does not have correct parent!";
            qDebug() << "Element name: " << elementToText(child);
            qDebug() << "Parent in tree model: " << elementToText(child->treeParent());
            qDebug() << "Expected parent: " << elementToText(element);
            qDebug() << "Heirarchy";
            while (child->treeParent()) {
                qDebug() << elementToText(child);
                child = child->treeParent();
            }
        }
        QCOMPARE(child->treeParent(), element);

        // recursively apply to the rest of the tree
        traverseTree(child);
    }
}

//---------------------------------------------------------
//   elementToText
///   for printing debug info about any element
//---------------------------------------------------------

QString elementToText(ScoreElement* element)
{
    if (element == nullptr) {
        return "nullptr";
    }
    if (element->isElement()) {
        return toElement(element)->userName() + " (" + toElement(element)->accessibleInfo() + ")";
    }
    return element->userName();
}

QTEST_MAIN(TestTreeModel)
#include "tst_tree_model.moc"
