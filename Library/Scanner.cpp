#include <QtCore/QFileInfo>

#include <Classification/Object.hpp>

#include "Scanner.hpp"
#include "Worker.hpp"

Scanner::Scanner (QString dst, QString src, int max_texture_resolution)
  : QThreadPool ()
  , m_diagnostics_lock (QMutex::Recursive)
  , m_objs_lock ()
  , m_objs ()
  , m_all (Classification::Definitions::fromFile ())
  , m_found (new Classification::Definitions(*m_all))
  , m_dst (dst)
  , m_src (src)
  , m_texture_limit (max_texture_resolution)
  
  , m_outstanding_lock (QMutex::Recursive)
  , m_outstanding (0)
  , m_hash_updates (false)
  , m_scan_completed (false)
{
  if (not m_all) {
    qFatal("can't load definitions.");
  }
  
  QFileInfo info (dst);
  if (not info.isDir()) {
    throw "not a directory";
  }
  
  setExpiryTimeout (-1);
}



Scanner::~Scanner ()
{
  QThreadPool::waitForDone();
  
  if (m_hash_updates) {
    m_all->toFile();
  }
}



QDir
Scanner::dstDir () const
{
  return (m_dst);
}



QDir
Scanner::srcDir () const
{
  return (m_src);
}



bool
Scanner::couldMatch(int filesize, const Hash &filehash)
{
  QMutexLocker locker (&m_objs_lock);
  return m_all->couldMatch(filesize, filehash);
}


Classification::ObjectPointer
Scanner::match(int filesize, const Hash &filehash, const Hash &refshash)
{
  QMutexLocker locker (&m_objs_lock);
  Classification::ObjectPointer retval(m_all->match(filesize, filehash, refshash));
  if (retval) {
    m_hash_updates |= retval->addFileHash(filehash);
    m_hash_updates |= retval->addTextureHash(refshash);
  }
  return (retval);
}


QString
Scanner::found (Classification::ObjectPointer obj)
{
  Classification::ObjectPointer copy (new Classification::Object(*obj));
  QString objname, basename;
  int n;
  
  objname = obj->name ();
  
  QMutexLocker locker (&m_objs_lock);
  
  n = m_objs.value (objname, 0);
  m_objs[objname] = n+1;
  
  basename = QString("%1-%2.obj").arg(objname).arg(n);
  
  copy->setFileName (basename);
  m_found->upsert (copy);
  
  locker.unlock();
  
  return (m_dst.absoluteFilePath(basename));
}



void
Scanner::finished (Worker *worker)
{
  emitDiagnostic (worker, "Finished");
  
  QMutexLocker locker(&m_outstanding_lock);
  --m_outstanding;
  locker.unlock();
  
  if (hasCompleted()) {
    QString path = m_dst.absoluteFilePath("found.json");
    emitDiagnostic(Q_NULLPTR, QString("All workers finished. Writing %1").arg(path));
    m_found->toFile(path);
    emit completed();
  }
}



void
Scanner::emitDiagnostic (Worker *worker, QString string)
{
  QString name;
  if (worker != Q_NULLPTR) {
    name = worker->src();
  } else {
    name = "Master";
  }
  
  QString message = QString("[%1] %2").arg(name).arg(string);
  
  QMutexLocker locker(&m_diagnostics_lock);
  emit diagnostic(message);
  locker.unlock();
}



void
Scanner::scan ()
{
  m_found->dropObjects ();
  
  Q_FOREACH (Classification::AircraftPointer a, m_all->aircrafts()) {
    m_found->upsert (a);
  }
  Q_FOREACH (Classification::AirlinePointer a, m_all->airlines()) {
    m_found->upsert (a);
  }
  for (int i=0; i!=m_all->numberOfLibraries(); ++i) {
    m_found->upsert (m_all->library(i));
  }
  
  setScanCompleted(false);
  scanImpl (m_src);
  setScanCompleted(true);
}


void
Scanner::scanImpl (QDir dir)
{
  QDir::Filters flags = QDir::AllEntries | QDir::NoDotAndDotDot;
  QDir::SortFlags sort = QDir::Name;
  
  emitDiagnostic(Q_NULLPTR, QString("Recursing into %1").arg(dir.path()));
  
  Q_FOREACH (const QFileInfo &i, dir.entryInfoList (flags, sort)) {
    
    QString subpath = i.canonicalFilePath ();
    
    if (i.isDir ()) {
      scanImpl (subpath);
    }
    
    if (i.isFile() ) {
      if (i.isReadable()) {
        if (m_all->couldMatch(i.size())) {
          spawnWorker (subpath);
        }
      }
    }
  }
}



void
Scanner::spawnWorker (QString subpath)
{
  emitDiagnostic (Q_NULLPTR, QString("Starting worker for %1").arg(subpath));
  
  QMutexLocker locker(&m_outstanding_lock);
  ++m_outstanding;
  locker.unlock();
  
  QThreadPool::start(new Worker(this, m_all, subpath, m_texture_limit));
}



void
Scanner::setScanCompleted (bool completed)
{
  QMutexLocker locker(&m_outstanding_lock);
  m_scan_completed = completed;
  locker.unlock();
}




bool
Scanner::hasCompleted ()
{
  bool retval;
  
  QMutexLocker locker(&m_outstanding_lock);
  retval = (m_outstanding == 0) && m_scan_completed;
  locker.unlock();
  
  return (retval);
}



Classification::DefinitionsPointer
Scanner::result () const
{
  return (m_found);
}

