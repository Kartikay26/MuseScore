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
#include "libmscore/page.h"

#define DIR QString("libmscore/all_elements/")

using namespace Ms;

//---------------------------------------------------------
//   TestScanElements
///   Ensures that the new scanElements function finds
///   exactly the same elements as the old one.
///
///   TODO: To be removed along with scanElementsOld once
///         the test passes
//---------------------------------------------------------

class TestScanElements : public QObject, public MTest
{
    Q_OBJECT

    MasterScore * score;

    void tstScanElements(QString file);
    void compareOldNew();

private slots:
    void initTestCase();
    void tstScanElementsElements() { tstScanElements("layout_elements.mscx"); }
    void tstScanElementsTablature() { tstScanElements("layout_elements_tab.mscx"); }
    // void tstScanElementsMoonlight() { tstScanElements("moonlight.mscx"); }
    // void tstScanElementsGoldberg() { tstScanElements("goldberg.mscx"); }
    // void tstScanElementsRepeats() { tstScanElements("../unrollrepeats/clef-key-ts-test.mscx"); }

public:
    std::map<ScoreElement*, int> elementsOld;
    std::map<ScoreElement*, int> elementsNew;
    std::set<ScoreElement*> allElements;
};

QString elementToText(ScoreElement* element);
QString elementHeirarchy(ScoreElement* element);
void scanOld(void*, Element*);  // for use with scanElementsOld
void scanNew(void*, Element*);  // for use with scanElements

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestScanElements::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   tstScanElements
//---------------------------------------------------------

void TestScanElements::tstScanElements(QString file)
{
    MasterScore* score = readScore(DIR + file);
    elementsOld.clear();
    elementsNew.clear();
    score->scanElementsOld(this, &scanOld, true);
    score->scanElements(this, &scanNew, true);
    compareOldNew();
    elementsOld.clear();
    elementsNew.clear();
    score->scanElementsOld(this, &scanOld, false);
    score->scanElements(this, &scanNew, false);
    compareOldNew();
}

void scanOld(void* data, Element* el)
{
    TestScanElements* test = static_cast<TestScanElements*>(data);
    test->elementsOld[el]++;
    test->allElements.insert(el);
}

void scanNew(void* data, Element* el)
{
    TestScanElements* test = static_cast<TestScanElements*>(data);
    test->elementsNew[el]++;
    test->allElements.insert(el);
}

void TestScanElements::compareOldNew()
{
    if (elementsOld != elementsNew) {
        qDebug() << "Element counts don't match for: ";
        for (auto el : allElements) {
            int oldCnt = elementsOld[el];
            int newCnt = elementsNew[el];
            if (oldCnt != newCnt) {
                qDebug() << elementToText(el) << QString(" (oldCnt: %1, newCnt: %2)").arg(oldCnt).arg(newCnt);
                // qDebug() << "Heirarchy: " << elementHeirarchy(el);
            }
        }
    }
    QVERIFY(elementsOld == elementsNew);
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
    if (element->isPage()) {
        return QString("Page ") + QString::number(toPage(element)->no());
    }
    if (element->isElement()) {
        return toElement(element)->userName() + " (" + toElement(element)->accessibleInfo() + ")";
    }
    return element->userName();
}

QString elementHeirarchy(ScoreElement* element)
{
    QString heirarchy = elementToText(element);
    while (element->treeParent()) {
        heirarchy += " < " + elementToText(element->treeParent());
        element = element->treeParent();
    }
    return heirarchy;
}

QTEST_MAIN(TestScanElements)
#include "tst_scanElements.moc"
