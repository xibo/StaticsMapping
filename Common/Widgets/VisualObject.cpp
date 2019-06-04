#include <Common/Classification/Aircraft.hpp>
#include <Common/Classification/Airline.hpp>
#include <Common/Classification/Library.hpp>

#include "VisualObject.hpp"

namespace Widgets
{
  VisualObject::VisualObject(
    Classification::DefinitionsPointer definitions,
    QString path
  )
  : file(new Obj8::File(path, true))
  , model(new VisualModel(path))
  , data()
  {
    data = definitions->match(file->size(), file->fileHash(), file->textureHash());
    if (data) {
      data->addFileHash(file->fileHash());
      data->addTextureHash(file->textureHash());
    } else {
      data.reset(new Classification::Object(file->size(), file->fileHash(), file->textureHash()));
      data->setFileName(file->basename());
    }
    
    file->accept(model.data(), false);
  }
  
  
  
  VisualObject::~VisualObject ()
  {
  }
}
