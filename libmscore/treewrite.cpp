#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   treeWrite
///  This will write the score by traversing the tree
//---------------------------------------------------------

void treeWrite(XmlWriter& xml, ScoreElement* element)
{
    xml.stag(element->name());

    for (int i = 0; i < element->treeChildCount(); i++) {
        ScoreElement* child = element->treeChild(i);
        treeWrite(xml, child);
    }
    
    xml.etag();
}

} // namespace Ms
