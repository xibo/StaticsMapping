#include <Common/Widgets/Application.hpp>
#include <Common/Widgets/OpenGL/Screen.hpp>

#include "ObjPreview.hpp"




#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtWidgets/QMessageBox>
#include <Common/Obj8/File.hpp>
#include <Common/DataPath.hpp>
#include <Common/Widgets/OpenGL/Obj8Visitor.hpp>

OpenGL::ModelPointer
loadObjFile (QString path)
{
  OpenGL::ModelPointer retval;
  
  QFile f(path);
  if (not f.open (QFile::ReadOnly)) {
    QMessageBox::critical (
      Q_NULLPTR,
      "Fatal",
      QString("Cannot read %1")
      .arg(path)
    );
    return (retval);
  }
  
  try {
    retval.reset(new OpenGL::Model);
    
    Obj8::File obj8 (f, true);
    QSharedPointer<OpenGL::Obj8Visitor> model(new OpenGL::Obj8Visitor(retval, path));
    
    obj8.accept (model.data(), false);
    
  } catch (const Obj8::Parser::SyntaxError &error) {
    
    Obj8::Parser::LexerContext lc = error.info ();
    
    QMessageBox::critical (
      Q_NULLPTR,
      "Fatal",
      QString("Error at %1:%2.")
      .arg(lc.line())
      .arg(lc.column())
    );
  }
  
  return (retval);
}

int
main (int argscnt, char **args)
{
  Widgets::Application app(argscnt, args);
  
  OpenGL::Screen screen;
  screen.show();
  
  for(int i=1; i!=argscnt; ++i) {
    QVector3D offset((i-1)*50, 0, 0);
    OpenGL::ModelPointer model(loadObjFile(args[i]));
    model->setPosition(offset);
    screen.addModel(model);
  }
  
  return (app.exec());
}
