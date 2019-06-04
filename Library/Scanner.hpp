#pragma once

#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QThreadPool>

#include <Classification/Definitions.hpp>
#include <Classification/Object.hpp>

struct Worker;

struct Scanner : QThreadPool
{
  Q_OBJECT;
  
public:
  Scanner (QString, QString, int);
  ~Scanner ();
  
  QDir    dstDir() const;
  QDir    srcDir() const;
  
  bool    couldMatch (int filesize, const Hash &filehash);
  Classification::ObjectPointer match (int filesize, const Hash &filehash, const Hash &refshash);
  QString found (Classification::ObjectPointer);
  void    finished (Worker *);
  
  void    scan ();
  void    emitDiagnostic (Worker *, QString);
  bool    hasCompleted ();
  Classification::DefinitionsPointer result() const;
  
  
Q_SIGNALS:
  void completed ();
  void diagnostic (QString);
  
protected:
  void scanImpl (QDir);
  void spawnWorker (QString);
  void setScanCompleted (bool);
  
  QMutex            m_diagnostics_lock;
  QMutex            m_objs_lock;
  QMap<QString,int> m_objs;
  
  Classification::DefinitionsPointer m_all, m_found;
  
  QDir m_dst, m_src;
  int  m_texture_limit;
  
  QMutex m_outstanding_lock;
  int  m_outstanding;
  bool m_hash_updates;
  bool m_scan_completed;
};
