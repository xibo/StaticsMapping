#include <Obj8/Parser/LexerContext.hpp>
#include <Obj8/Parser/SyntaxError.hpp>
#include <Obj8/File.hpp>

#include "ObjFileInstaller.hpp"
#include "Worker.hpp"

Worker::Worker (
  Scanner *scanner,
  Classification::DefinitionsPointer definitions,
  QString src,
  int texture_limit
)
  : QRunnable ()
  , m_definitions (definitions)
  , m_src (src)
  , m_scanner (scanner)
  , m_texture_limit (texture_limit)
{
}


Worker::~Worker ()
{
}


void
Worker::run ()
{
  m_scanner->emitDiagnostic(this, "Starting");
  
  try {
    runImpl();
  } catch (const Obj8::Parser::SyntaxError &error) {
    m_scanner->emitDiagnostic(
      this,
      QString("Stopped reading at line %2, column %3.")
        .arg(error.info().line())
        .arg(error.info().column())
    );
  } catch (const std::exception &e) {
    m_scanner->emitDiagnostic(this, e.what());
  } catch (...) {
    m_scanner->emitDiagnostic(this, "Caught unstructured exception.");
  }
  
  return (m_scanner->finished(this));
}



void
Worker::runImpl()
{
  Obj8::File obj8 (m_src, content(), false);
  
  /*
   * size already matched by primary thread.
   */
  if (not m_scanner->couldMatch (obj8.size(), obj8.fileHash())) {
    throw (std::runtime_error("unknown primary hash"));
  }
  
  obj8.parse ();

  Classification::ObjectPointer metadata(
    m_scanner->match (obj8.size(), obj8.fileHash(), obj8.textureHash())
  );

  if (!metadata) {
    throw (std::runtime_error("unknown secondary hash"));
  }

  QString dst = m_scanner->found (metadata);
  m_scanner->emitDiagnostic(this, QString("Output goes to %1").arg(dst));

  ObjFileInstaller installer;
  installer.setFilename (m_src);
  installer.setTargetFilename (dst);
  installer.setRotation (metadata->rotate());
  installer.setTranslation (
    metadata->translate(0),
    metadata->translate(1),
    metadata->translate(2)
  );
  installer.setMaxTextureResolution (m_texture_limit);
  obj8.accept (&installer, true);

  QFile sink (dst);
  if (not sink.open (QFile::WriteOnly | QFile::Truncate)) {
    throw (std::runtime_error("failed to write output"));
  }
  sink.write (obj8.toString().toUtf8());
  sink.close ();
}



QString
Worker::src() const
{
  return (m_src);
}



QByteArray
Worker::content () const
{
  QFile file (m_src);
  QByteArray content;
  
  bool ok;
  ok = file.open (QFile::ReadOnly);
  if (not ok) {
    throw (std::runtime_error("Cannot read input"));
  }
  
  content = Obj8::File::read (file);
  
  return (content);
}
