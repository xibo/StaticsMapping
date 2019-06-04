#include <iostream>

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>

#include <Classification/Definitions.hpp>
#include <Classification/Object.hpp>
#include <Obj8/File.hpp>

int
main (int argscnt, char **args)
{
  QCoreApplication app (argscnt, args);
  QCommandLineParser parser;
  parser.addOption(QCommandLineOption("verbose"));
  parser.addHelpOption();
  parser.process(app);
  
  Classification::DefinitionsPointer definitions(
      Classification::Definitions::fromFile ()
  );
  
  Q_FOREACH (QString filename, parser.positionalArguments()) {
    QFile qf (filename);
    if (not qf.open (QFile::ReadOnly)) {
      qCritical ("Failed to open %s", qUtf8Printable(filename));
      continue;
    }
    
    if (definitions->couldMatch (qf.size())) {
      Obj8::File file (qf, false);
      if (definitions->couldMatch (file.size(), file.fileHash())) {
        try {
          file.parse ();
          Classification::ObjectPointer obj;
          obj = definitions->match (file.size(), file.fileHash(), file.textureHash());
          
          if (parser.isSet("verbose")) {
            if (obj) {
              std::cout << "Known ";
            } else {
              std::cout << "Unknown ";
            }
            std::cout
              << "<"
              << qUtf8Printable(filename)
              << "> hash <"
              << qUtf8Printable (
                  Classification::Object (
                    file.size(), file.fileHash(), file.textureHash()
                  )
                  .primaryKey()
                )
              << ">"
              << std::endl;
          } else {
            if (obj) {
              std::cout << qUtf8Printable(filename) << std::endl;
            }
          }
          
        } catch (...) {
          qCritical ("size and primary hash match for %s.", qUtf8Printable(filename));
        }
      }
    }
  }
  
  Q_UNUSED (app);
  return (EXIT_SUCCESS);
}
