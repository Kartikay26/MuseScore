#include "xml.h"
#include "property.h"

namespace Ms {

//---------------------------------------------------------
//   treeWrite
///  This will write the score by traversing the tree
//---------------------------------------------------------

void treeWrite(XmlWriter& xml, ScoreElement* element)
{
    xml.stag(element->name());

    for (int i = 0; i < int(Pid::END); i++) {
        element->writeProperty(xml, Pid(i));
    }

    for (int i = 0; i < element->treeChildCount(); i++) {
        ScoreElement* child = element->treeChild(i);
        treeWrite(xml, child);
    }
    
    xml.etag();
}

} // namespace Ms
