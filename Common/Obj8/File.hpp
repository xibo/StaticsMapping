#pragma once

#include <QByteArray>
#include <QFile>

#include <CryptoHash.hpp>

#include "Header.hpp"
#include "Obj8.hpp"
#include "RecordSequence.hpp"

namespace Obj8
{
  struct File
  {
    File ();
    File (QString, bool);
    File (QFile &, bool);
    File (QString, QByteArray, bool);
    ~File ();
    
    void    accept (AbstractVisitor *, bool);
    void    parse ();
    String  toString () const;
    
    Hash    fileHash () const;
    Hash    textureHash () const;
    int     size () const;
    
    QString    basename () const;
    QString    filename () const;
    
    static QByteArray read (QFile &file);
    static QByteArray read (QString path);
    
  protected:
    QByteArray     m_raw;
    Hash           m_hash_file;
    Hash           m_hash_texture;
    QString        m_path, m_basename;
    Header         m_header;
    RecordSequence m_content;
  };
}
