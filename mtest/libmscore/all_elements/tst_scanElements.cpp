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
    void tstScanElementsMoonlight() { tstScanElements("moonlight.mscx"); }
    // void tstScanElementsGoldberg() { tstScanElements("goldberg.mscx"); }

public:
    std::set<ScoreElement*> elementsOld;
    std::set<ScoreElement*> elementsNew;
};

QString elementToText(ScoreElement* element);
void scanOld(void*, Element*);  // for use with scanElementsOld
void scanNew(void*, Element*);  // for use with scanElements
void compareOldNew();

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
    elementsOld.clear();
    elementsNew.clear();
    MasterScore* score = readScore(DIR + file);
    score->scanElementsOld(this, &scanOld, true);
    score->scanElements(this, &scanNew, true);
    compareOldNew();
    score->scanElementsOld(this, &scanOld, false);
    score->scanElements(this, &scanNew, false);
    compareOldNew();
}

void scanOld(void* data, Element* el)
{
    TestScanElements* test = static_cast<TestScanElements*>(data);
    test->elementsOld.insert(el);
}

void scanNew(void* data, Element* el)
{
    TestScanElements* test = static_cast<TestScanElements*>(data);
    test->elementsNew.insert(el);
}

void TestScanElements::compareOldNew()
{
    if (elementsOld != elementsNew) {
        qDebug() << "Extra in elementsOld: ";
        for (auto el : elementsOld) {
            if (!elementsNew.count(el)) {
                qDebug() << elementToText(el);
            }
        }
        qDebug() << "Extra in elementsNew: ";
        for (auto el : elementsNew) {
            if (!elementsOld.count(el)) {
                qDebug() << elementToText(el);
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
    if (element->isElement()) {
        return toElement(element)->userName() + " (" + toElement(element)->accessibleInfo() + ")";
    }
    return element->userName();
}

QTEST_MAIN(TestScanElements)
#include "tst_scanElements.moc"
