#pragma once

#include <Common/Obj8/File.hpp>

#include "Scene.hpp" 

namespace OpenGL
{
  struct Obj8Scene : Scene
  {
    Obj8Scene(bool add_grid=true);
    ~Obj8Scene() Q_DECL_OVERRIDE;
    
    ModelWeakPointer insertModel(Obj8::File *file);
    ModelWeakPointer insertModel(QString path);
    using Scene::insertModel;
  };
  
  typedef QSharedPointer<Obj8Scene> Obj8ScenePointer;
}