//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "element.h"
#include "score.h"
#include "scoreElement.h"
#include "staff.h"
#include "measure.h"

namespace Ms {
//---------------------------------------------------------
//   writable
///   Check if property can be written
//---------------------------------------------------------

P_TYPE unwritable[] = {
    P_TYPE::POINT_MM, P_TYPE::SIZE_MM, P_TYPE::BEAM_MODE,
    P_TYPE::TEMPO,    P_TYPE::GROUPS,  P_TYPE::INT_LIST,
};

bool writable(Pid p)
{
    for (P_TYPE x : unwritable) {
        if (propertyType(p) == x) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------
//   writeAllProperties
//---------------------------------------------------------

static void writeAllProperties(XmlWriter& xml, Element* e)
{
    for (int p = 0; p < int(Pid::END); p++) {
        if (!writable(Pid(p))) {
            continue;
        }
        if (e->getProperty(Pid(p)).isValid()) {
            e->writeProperty(xml, Pid(p));
        }
    }
}

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

void ScoreElement::treeWrite(XmlWriter& xml)
{
    xml.stag(this);
    for (ScoreElement* ch : *this) {
        ch->treeWrite(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

void Element::treeWrite(XmlWriter& xml)
{
    if (generated()) {
        return;
    }
    xml.stag(this);
    writeAllProperties(xml, this);
    for (ScoreElement* ch : *this) {
        ch->treeWrite(xml);
    }
    xml.etag();
}

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

void Score::treeWrite(XmlWriter& xml)
{
    xml.header();
    xml.stag("museScore version=\"3.01\"");
    xml.stag(this);
    // write all measures in staff 1 first, then staff 2, etc.
    for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        xml.stag(staff(staffIdx), QString("id=\"%1\"").arg(staffIdx + 1));
        for (MeasureBase* m = measures()->first(); m != measures()->last(); m = m->next()) {
            if (m->isMeasure()) {
                toMeasure(m)->treeWriteStaff(xml, staffIdx);
            } else {
                // non-measure things (like boxes) to be written only once in staff 1
                if (staffIdx == 0) {
                    m->treeWrite(xml);
                }
            }
        }
        xml.etag();
    }
    xml.etag(); // score
    xml.etag(); // musescore
}

//---------------------------------------------------------
//   Measure::treeWriteStaff
///   Writes one staff in one measure
//---------------------------------------------------------

void Measure::treeWriteStaff(XmlWriter& xml, int staffIdx)
{
    xml.stag(this);
    for (Element* e : el()) {
        e->treeWrite(xml);
    }
    // write voice 1 first, then voice 2, .. upto VOICES
    for (int voice = 0; voice < VOICES; voice++) {
        xml.stag("voice");
        int track = staffIdx * VOICES + voice;
        for (const Segment& s : segments()) {
            Element* e = s.element(track);
            if (e) {
                e->treeWrite(xml);
            }
        }
        xml.etag(); // voice
    }
    xml.etag(); // measure
}
} // namespace Ms
