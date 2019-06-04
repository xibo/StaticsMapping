#include <stdexcept>

#include <QtCore/QMetaEnum>
#include <QVariant>

#include <Common/Common.hpp>
#include <Common/CryptoHash.hpp>

#include "Aircraft.hpp"
#include "Airline.hpp"
#include "Definitions.hpp"
#include "Library.hpp"
#include "Object.hpp"


const char *
Classification::Object::propertyByName (PropertyName p)
{
  switch (p) {
    case AircraftProperty: return ("aircraft");
    case LiveryProperty: return ("livery");
    case LibraryProperty: return ("library");
    case CommentProperty: return ("comment");
    case CompositehashProperty: return ("compositehash");
    case FilenameProperty: return ("filename");
    //case FilehashProperty: return ("filehash");
    case FilesizeProperty: return ("filesize");
    //case TexturehashProperty: return ("texturehash");
    case PurposeProperty: return ("purpose");
    case FictiveProperty: return ("fictive");
    case IntroductionProperty: return ("introduction");
    case RetirementProperty: return ("retirement");
    case RotateProperty: return ("rotate");
    case TranslationXProperty: return ("translateX");
    case TranslationYProperty: return ("translateY");
    case TranslationZProperty: return ("translateZ");
  }
  
  abort ();
}


Classification::Object::Object (Size filesize, Hash filehash, Hash refshash)
  : Record ()
  , m_aircraft ()
  , m_livery ()
  , m_library ()
  , m_comment ()
  , m_filename ()
  , m_filehash (filehash)
  , m_texturehash (refshash)
  , m_introduced (0)
  , m_retired (0)
  , m_size (filesize)
  , m_fictive (true)
  , m_purpose (GeneralAviation)
  , m_rotate (RotateNull)
{
}



Classification::Object::Object (const Object &o)
  : Record (o)
  , m_aircraft (o.m_aircraft)
  , m_livery (o.m_livery)
  , m_library (o.m_library)
  , m_comment (o.m_comment)
  , m_filename (o.m_filename)
  , m_filehash (o.m_filehash)
  , m_texturehash (o.m_texturehash)
  , m_introduced (o.m_introduced)
  , m_retired (o.m_retired)
  , m_size (o.m_size)
  , m_fictive (o.m_fictive)
  , m_purpose (o.m_purpose)
  , m_rotate (o.m_rotate)
{
}



Classification::Object::Object (Definitions *q, const QJsonObject &o)
  : Record (q, o)
  , m_aircraft (o.value("aircraft").toString())
  , m_livery (o.value("livery").toString())
  , m_library (o.value("library").toString())
  , m_comment (o.value("comment").toString())
  , m_filename (o.value("filename").toString())
  , m_filehash (o.value("filehash"))
  , m_texturehash (o.value("texturehash"))
  , m_introduced (o.value("introduced").toInt())
  , m_retired (o.value("retired").toInt())
  , m_size (o.value("size").toInt())
  , m_fictive (o.value("fictive").toBool(true))
  , m_purpose (stringToEnum<Purpose>(o.value("category"), GeneralAviation))
  , m_rotate (stringToEnum<Rotate>(o.value("rotate"), RotateNull))
{
  if (o.contains("translate")) {
    QString translate = o.value("translate").toString();
    
    Obj8::Parser::LexerContext lc;
    
    setTranslate (Obj8::Parser::Number (translate, &lc), 0);
    setTranslate (Obj8::Parser::Number (translate, &lc), 1);
    setTranslate (Obj8::Parser::Number (translate, &lc), 2);
  }
}



Classification::Object::~Object ()
{
}



QJsonObject
Classification::Object::toJson () const
{
  QJsonObject obj = Record::toJson ();

  obj.insert ("size", m_size);
  obj.insert ("texturehash", m_texturehash.toJson());
  obj.insert ("filename", m_filename);
  obj.insert ("filehash", m_filehash.toJson());
  obj.insert ("livery", m_livery);
  obj.insert ("aircraft", m_aircraft);
  obj.insert ("library", m_library);
  obj.insert ("comment", m_comment);
  obj.insert ("introduced", m_introduced);
  obj.insert ("retired", m_retired);
  obj.insert ("fictive", m_fictive);
  obj.insert ("category", enumToString<Purpose>(m_purpose));
  obj.insert ("rotate", enumToString<Rotate>(m_rotate));
      
  obj.insert (
      "translate",
      QString("%1 %2 %3")
          .arg(m_translate_x.value())
          .arg(m_translate_y.value())
          .arg(m_translate_z.value()));
  
  return (obj);
}



const char *
Classification::Object::primaryKeyProperty () const
{
  return (propertyByName (CompositehashProperty));
}


bool
Classification::Object::verifyPrimaryKey (PrimaryKey key) const
{
  QByteArray hex(key.toLower().toUtf8());
  QByteArray bin(QByteArray::fromHex(hex));
  
  if (bin.isEmpty()) {
    return false;
  }
  
  return (bin.toHex().toLower() == hex);
}



QString
Classification::Object::aircraft () const
{
  return (m_aircraft);
}



void
Classification::Object::setAircraft (QString a)
{
  if (m_q and not m_q->aircraft(a)) {
    throw (std::runtime_error("Invalid Aircraft"));
  }
  
  m_aircraft = a;
}



QString
Classification::Object::livery () const
{
  return (m_livery);
}



void
Classification::Object::setLivery (QString o)
{
  if (m_q and not m_q->airline(o)) {
    throw (std::runtime_error("Invalid livery"));
  }
  
  m_livery = o;
}



QString
Classification::Object::library () const
{
  return (m_library);
}



void
Classification::Object::setLibrary (QString o)
{
  if (m_q and not m_q->library(o)) {
    throw (std::runtime_error("Invalid library"));
  }
  
  m_library = o;
}



QString
Classification::Object::comment () const
{
  return (m_comment);
}



void
Classification::Object::setComment (QString comment)
{
  m_comment = comment;
}



QString
Classification::Object::fileName () const
{
  return (m_filename);
}



void
Classification::Object::setFileName (QString filename)
{
  m_filename = filename;
}



Hash
Classification::Object::fileHash () const
{
  return (m_filehash);
}


bool
Classification::Object::addFileHash(Hash hash)
{
  return m_filehash.addResult(hash);
}


void
Classification::Object::setFileHash (Hash s)
{
  if (s.hasResult(Hash::keyMethod())) {
    m_filehash = s;
  } else {
    throw (std::runtime_error("poor hash"));
  }
}


Hash
Classification::Object::textureHash () const
{
  return (m_texturehash);
}


bool
Classification::Object::addTextureHash (Hash hash)
{
  return (m_texturehash.addResult(hash));
}


void
Classification::Object::setTextureHash (Hash s)
{
  if (s.hasResult(Hash::keyMethod())) {
    m_texturehash = s;
  } else {
    throw (std::runtime_error("poor hash"));
  }
}



QString
Classification::Object::compositeHash () const
{
  return (compositeHash (m_size, m_filehash, m_texturehash));
}



QString
Classification::Object::compositeHash (int filesize, Hash filehash, Hash refshash)
{
  
  QString retval = QString (
      "%1-%2-%3")
      .arg(QString::number(filesize, 16).rightJustified(8, '0'))
      .arg(filehash.resultString(Hash::keyMethod()))
      .arg(refshash.resultString(Hash::keyMethod()));
  return (retval);
}



Classification::Object::Size
Classification::Object::fileSize () const
{
  return (m_size);
}



void
Classification::Object::setFileSize (Size s)
{
  bool acceptable = (s >= 1);
  if (not acceptable) {
    throw (std::runtime_error("poor size"));
  }
  
  m_size = s;
}



int
Classification::Object::introduced () const
{
  return (m_introduced);
}



void
Classification::Object::setIntroduced (int introduced)
{
  m_introduced = introduced;
}



int
Classification::Object::retired () const
{
  return (m_retired);
}



void
Classification::Object::setRetired (int retired)
{
  m_retired = retired;
}



bool
Classification::Object::fictive () const
{
  return (m_fictive);
}



void
Classification::Object::setFictive (bool fictive)
{
  m_fictive = fictive;
}



Classification::Object::Purposes
Classification::Object::purpose () const
{
  return (m_purpose);
}



void
Classification::Object::setPurpose (Purposes p)
{
  m_purpose = p;
}



void
Classification::Object::setRotate (Rotate rotate)
{
  m_rotate = rotate;
}



Classification::Object::Rotate
Classification::Object::rotate () const
{
  return (m_rotate);
}



Obj8::Parser::Number
Classification::Object::translate (size_t i) const
{
  Q_ASSERT (i < 3);
  
  if (i == 0){
    return (m_translate_x);
  } else if (i == 1) {
    return (m_translate_y);
  } else {
    return (m_translate_z);
  }
}



void
Classification::Object::setTranslate (Obj8::Parser::Number num, size_t i)
{
  Q_ASSERT (i < 3);
  
  if (i == 0){
    m_translate_x = num;
  } else if (i == 1) {
    m_translate_y = num;
  } else {
    m_translate_z = num;
  }
}



void
Classification::Object::setTranslateX (QString s)
{
  setTranslate (Obj8::Parser::Number (s), 0);
}



void
Classification::Object::setTranslateY (QString s)
{
  setTranslate (Obj8::Parser::Number (s), 1);
}



void
Classification::Object::setTranslateZ (QString s)
{
  setTranslate (Obj8::Parser::Number (s), 2);
}


QString
Classification::Object::translateX () const
{
  return (m_translate_x.toString());
}


QString
Classification::Object::translateY () const
{
  return (m_translate_y.toString());
}


QString
Classification::Object::translateZ () const
{
  return (m_translate_z.toString());
}



QString
Classification::Object::name () const
{
  return (aircraft() + "-" + livery());
}


void
Classification::Object::fixReferences ()
{
  Q_CHECK_PTR (m_q);
  
  if (not m_q->aircraft(m_aircraft)) {
    if (m_q->numberOfAircrafts() != 0) {
      m_aircraft = m_q->aircraft(0)->primaryKey ();
    } else {
      m_aircraft.clear();
    }
    qDebug("Reset aircraft of \'%s\'", qUtf8Printable(primaryKey()));
  }
  
  if (not m_q->airline(m_livery)) {
    if (m_q->numberOfAirlines() != 0) {
      m_livery = m_q->airline(0)->primaryKey ();
    } else {
      m_livery.clear();
    }
    qDebug("Reset airline of \'%s\'", qUtf8Printable(primaryKey()));
  }
  
  if (not m_q->library(m_library)) {
    if (m_q->numberOfLibraries() != 0) {
      m_library = m_q->library(0)->primaryKey ();
    } else {
      m_library.clear();
    }
    qDebug("Reset library of \'%s\'", qUtf8Printable(primaryKey()));
  }
}
