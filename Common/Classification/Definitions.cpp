#include <stdexcept>

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

#include <Common/DataPath.hpp>

#include "Aircraft.hpp"
#include "Airline.hpp"
#include "Airport.hpp"
#include "Definitions.hpp"
#include "Library.hpp"
#include "Object.hpp"

#include "AirlineModel.hpp"
#include "AircraftModel.hpp"
#include "LibraryModel.hpp"
#include "ObjectModel.hpp"



template <typename X>
Classification::Table<X>::Table (Definitions *q)
: sorted ()
, forward ()
, reverse ()
, observers ()
, q (q)
{
}



template <typename X>
Classification::Table<X>::Table (const Table &other, Definitions *q)
: sorted ()
, forward ()
, reverse ()
, observers ()
, q (q)
{
  /*
   * Deep copy.
   */
  Q_FOREACH (ValuePointer v, other.sorted) {
    sorted.insert (
        v->primaryKey (),
        ValuePointer (
            new X (*v)
        )
    );
  }
  regenerate ();
  
  Q_FOREACH (ValuePointer v, sorted) {
    v->setDefinitions (q);
  }
}



template <typename X>
Classification::Table<X>::Table (QJsonArray a, Definitions *q)
: sorted ()
, forward ()
, reverse ()
, observers ()
, q (q)
{
  for (QJsonArray::ConstIterator i=a.constBegin(); i!=a.constEnd(); ++i) {
    Q_ASSERT (i->isObject());
    ValuePointer x(new Value (q, i->toObject ()));
    
    /*
     * Do not upsert here, it takes linear time, and we do this a linear number
     * of times.
     */
#if 0
    //upsert (x, false);
#else
    sorted.insert (x->primaryKey(), x);
#endif
  }
  regenerate ();
}



template <typename X>
Classification::Table<X>::~Table ()
{
  Q_ASSERT (observers.isEmpty());
  
  q = Q_NULLPTR;
}



template <typename X>
QJsonArray
Classification::Table<X>::toJson () const
{
  QJsonArray retval;
  
  Q_FOREACH (ValuePointer v, sorted) {
    retval.append (v->toJson());
  }
  
  return (retval);
}



template<typename X>
int
Classification::Table<X>::clear ()
{
  int dropped = sorted.size ();
  
  sorted.clear ();
  regenerate ();
  notifyObservers (-1);
  
  return (dropped);
}



template<typename X>
int
Classification::Table<X>::drop (ValuePointer value)
{
  typename KVTable::Iterator i, e;
  int dropped;
  
  i = sorted.find (value->primaryKey());
  e = sorted.end ();
  
  if (i != e) {
    sorted.erase (i);
    dropped = 1;
    
    regenerate ();
    
    notifyObservers (-1);
  } else {
    dropped = 0;
  }
  
  return (dropped);
}



template<typename X>
int
Classification::Table<X>::drop (Key foreign, Key (X::*attribute)() const)
{
  QList<ValuePointer> matches;
  int retval;
  
  Q_FOREACH (ValuePointer v, sorted) {
    Value *ptr = v.data ();
    if (foreign == (ptr->*attribute)()) {
      matches.append (v);
    }
  }
  
  retval = 0;
  Q_FOREACH (ValuePointer v, matches) {
    retval += q->drop (v);
  }
  
  return (retval);
}



template <typename X>
void
Classification::Table<X>::upsert (
    ValuePointer value
)
{
  Key key = value->primaryKey ();
  int index = this->index (value);
  
  if (index == -1) {
    sorted.insert (key, value);
    regenerate ();
    value->setDefinitions (q);
  }
  
  notifyObservers (index);
}



template <typename X>
void
Classification::Table<X>::regenerate ()
{
  forward.clear ();
  reverse.clear ();
  
  forward.reserve (sorted.size());
  
  int i=0;
  Q_FOREACH (ValuePointer v, sorted) {
    forward.append (v);
    reverse.insert (v, i++);
  }
}



template <typename X>
int
Classification::Table<X>::index (ValuePointer v) const
{
  int retval;
  
  retval = reverse.value (v, -1);
  
  return (retval);
}



template <typename X>
typename Classification::Table<X>::ValuePointer
Classification::Table<X>::value (Key k) const
{
  return (sorted.value (k));
}



template <typename X>
typename Classification::Table<X>::ValuePointer
Classification::Table<X>::value (int idx) const
{
  return (forward.value (idx));
}



template <typename X>
int
Classification::Table<X>::size () const
{
  return (sorted.size());
}



template <typename X>
void
Classification::Table<X>::setDefinitions (Definitions *qptr)
{
  q = qptr;
  Q_FOREACH (ValuePointer v, sorted) {
    v->setDefinitions (qptr);
  }
  notifyObservers (-1);
}



template <typename X>
void
Classification::Table<X>::link (Model *m, bool r)
{
  Observer observer;
  
  observer.model = m;
  observer.structural_only = r;
  
  observers.append (observer);
}



template <typename X>
void
Classification::Table<X>::unlink (Model *m)
{
  for (typename QList<Observer>::Iterator i=observers.begin(); i!=observers.end(); /* no step */) {
    if (i->model == m) {
      i = observers.erase (i);
    } else {
      ++i;
    }
  }
}



template <typename X>
void
Classification::Table<X>::notifyObservers (int row)
{
  Q_FOREACH (Observer const& o, observers) {
    if ((row == -1) or (o.structural_only == false)) {
      o.model->notify (o.structural_only? -1 : row);
    }
  }
}



Classification::Definitions::Definitions ()
  : m_airport_table (this)
  , m_aircraft_table (this)
  , m_airline_table (this)
  , m_library_table (this)
  , m_object_table (this)
{
}



Classification::Definitions::Definitions (const Definitions &other)
  : m_airport_table (other.m_airport_table, this)
  , m_aircraft_table (other.m_aircraft_table, this)
  , m_airline_table (other.m_airline_table, this)
  , m_library_table (other.m_library_table, this)
  , m_object_table (other.m_object_table, this)
{
}



Classification::Definitions::Definitions (QJsonObject defs)
: m_airport_table (defs.value("airports").toArray(), this)
, m_aircraft_table (defs.value("aircrafts").toArray(), this)
, m_airline_table (defs.value("airlines").toArray(), this)
, m_library_table (defs.value("libraries").toArray(), this)
, m_object_table (defs.value("objects").toArray(), this)
{
  Q_ASSERT (checkConsistency());
}



Classification::Definitions::~Definitions ()
{
}



QJsonObject
Classification::Definitions::toJson () const
{
  QJsonObject obj;
  obj.insert ("aircrafts", m_aircraft_table.toJson());
  obj.insert ("airlines",  m_airline_table.toJson());
  obj.insert ("airports", m_airport_table.toJson());
  obj.insert ("libraries", m_library_table.toJson());
  obj.insert ("objects",   m_object_table.toJson());
  
  return (obj);
}



Classification::Definitions::AirlinesAtAirports
Classification::Definitions::airlinesAtAirports () const
{
  AirlinesAtAirports retval;
  
  Q_FOREACH (AirlinePointer airline, airlines()) {
    Q_FOREACH (QString airport_icao, airline->allHubsList()) {
      retval.insert(airport_icao, airline);
    }
  }
  
  return (retval);
}




Classification::Definitions::AirportPopulation
Classification::Definitions::airportPopulation () const
{
  AirportPopulation retval;

  Q_FOREACH (ObjectPointer object, objects()) {
    AirlinePointer airline = this->airline (object->livery());
    if (airline) {
      Q_FOREACH (Airport::PrimaryKey hub, airline->hubsList()) {
        
        /*
         * all defunctive inputs should have been removed already at this
         * point.
         */
        Q_ASSERT (airport (hub));
        
        retval.insert (hub, object);
      }
    }
  }

  return (retval);
}



void
Classification::Definitions::upsert (AircraftPointer a)
{
  m_aircraft_table.upsert (a);
}



void
Classification::Definitions::upsert (AirlinePointer a)
{
  m_airline_table.upsert (a);
}



void
Classification::Definitions::upsert (AirportPointer a)
{
  m_airport_table.upsert (a);
}



void
Classification::Definitions::upsert (LibraryPointer l)
{
  m_library_table.upsert (l);
}



void
Classification::Definitions::upsert (ObjectPointer o)
{
  m_object_table.upsert (o);
}



int
Classification::Definitions::drop (AircraftPointer ptr)
{
  int droped;
  
  if (ptr) {
    droped  = m_object_table.drop (ptr->icao(), &Object::aircraft);
    droped += m_aircraft_table.drop (ptr);
  } else {
    droped = 0;
  }
  
  return (droped);
}



int
Classification::Definitions::drop (AirlinePointer ptr)
{
  int droped;
  
  if (ptr) {
    /*
     * unset own parent to prevent infinite recursion
     */
    ptr->setParent (Airline::PrimaryKey());
  
    droped  = m_airline_table.drop (ptr->icao(), &Airline::parent);
    droped += m_object_table.drop (ptr->icao(), &Object::livery);
    droped += m_airline_table.drop (ptr);
  } else {
    droped = 0;
  }
  
  return (droped);
}



int
Classification::Definitions::drop (LibraryPointer ptr)
{
  int droped;
  
  if (ptr) {
    droped  = m_object_table.drop (ptr->primaryKey(), &Object::library);
    droped += m_library_table.drop (ptr);
  } else {
    droped = 0;
  }
  
  return (droped);
}



int
Classification::Definitions::drop (ObjectPointer ptr)
{
  int droped;
  
  if (ptr) {
    droped = m_object_table.drop (ptr);
  } else {
    droped = 0;
  }
  
  return (droped);
}



int
Classification::Definitions::dropObjects ()
{
  return (m_object_table.clear ());
}



Classification::AircraftPointer
Classification::Definitions::aircraft (QString icao) const
{
  return (m_aircraft_table.value (icao));
}



Classification::AircraftPointer
Classification::Definitions::aircraft (int index) const
{
  return (m_aircraft_table.value (index));
}



Classification::AirlinePointer
Classification::Definitions::airline (QString icao) const
{
  return (m_airline_table.value (icao));
}



Classification::AirlinePointer
Classification::Definitions::airline (int index) const
{
  return (m_airline_table.value (index));
}



Classification::AirportPointer
Classification::Definitions::airport (QString icao) const
{
  return (m_airport_table.value (icao));
}



Classification::LibraryPointer
Classification::Definitions::library (QString key) const
{
  return (m_library_table.value (key));
}



Classification::LibraryPointer
Classification::Definitions::library (int index) const
{
  return (m_library_table.value (index));
}



Classification::ObjectPointer
Classification::Definitions::object (QString hash) const
{
  return (m_object_table.value (hash));
}



Classification::ObjectPointer
Classification::Definitions::object (int index) const
{
  return (m_object_table.value (index));
}



Classification::Definitions::Aircrafts
Classification::Definitions::aircrafts () const
{ 
  return (m_aircraft_table.sorted);
}



Classification::Definitions::Airlines
Classification::Definitions::airlines () const
{
  return (m_airline_table.sorted);
}



Classification::Definitions::Objects
Classification::Definitions::objects () const
{
  return (m_object_table.sorted);
}



int
Classification::Definitions::numberOfAircrafts () const
{
  return (m_aircraft_table.size());
}



int
Classification::Definitions::numberOfAirlines () const
{
  return (m_airline_table.size());
}



int
Classification::Definitions::numberOfLibraries () const
{
  return (m_library_table.size());
}



int
Classification::Definitions::numberOfObjects () const
{
  return (m_object_table.size());
}



int
Classification::Definitions::indexOf (AirlinePointer ptr) const
{
  return (m_airline_table.index (ptr));
}



int
Classification::Definitions::indexOf (ObjectPointer ptr) const
{
  return (m_object_table.index (ptr));
}



bool
Classification::Definitions::checkConsistency () const
{
  Q_FOREACH (ObjectPointer o, objects()) {
    if (not aircraft (o->aircraft())) {
      qCritical ("Stale aircraft: %s of %s", qUtf8Printable(o->aircraft()), qUtf8Printable(o->primaryKey()));
      return (false);
    }
    
    if (not airline (o->livery())) {
      qCritical ("Stale livery: %s of %s", qUtf8Printable(o->livery()), qUtf8Printable(o->primaryKey()));
      return (false);
    }
    
    if (not library (o->library())) {
      qCritical ("Stale library: %s of %s", qUtf8Printable(o->library()), qUtf8Printable(o->primaryKey()));
      return (false);
    }
  }
  
  Q_FOREACH (AirlinePointer a, airlines()) {
    QString owner = a->parent ();
    if (owner.isEmpty()) {
      continue;
    }
    
    if (not airline (owner)) {
      return (false);
    }
  }
  
  return (true);
}



bool
Classification::Definitions::couldMatch (int size) const
{
  Q_FOREACH (ObjectPointer p, m_object_table.sorted) {
    if (p->fileSize() == size) {
      return (true);
    }
  }
  
  return (false);
}



bool
Classification::Definitions::couldMatch (Object::Hash filehash) const
{
  QString str = filehash.toLower ();
  
  Q_FOREACH (ObjectPointer p, m_object_table.sorted) {
    if (p->fileHash() == str) {
      return (true);
    }
  }
  
  return (false);
}



Classification::ObjectPointer 
Classification::Definitions::match (int filesize, Object::Hash obj8file, Object::Hash textures) const
{
  QString composite = Object::compositeHash (filesize, obj8file, textures);
  
  return (object(composite));
}



bool
Classification::Definitions::toFile () const
{
  return (toFile(DataPath::existingPath("data.json")));
}



bool
Classification::Definitions::toFile (QString filename) const
{
  Q_ASSERT(!filename.isNull());
  if (filename.isNull()) {
    filename = DataPath::existingPath("data.json");
  }
  
  QFile file (filename);
  bool ok;
  
  ok = file.open (QFile::WriteOnly | QFile::Truncate);
  
  if (ok) {
    QJsonDocument newdoc;
    newdoc.setObject (toJson ());
    file.write (newdoc.toJson ());
    file.close ();
  }
  
  return (ok);
}



Classification::DefinitionsPointer
Classification::Definitions::fromFile (QString filename)
{
  DefinitionsPointer retval;
  
  if (filename.isNull())
    filename = DataPath::existingPath("data.json");
  
  QJsonObject defs(readJson(filename));
  
  if (defs.size() != 0) {
    retval.reset (new Definitions (defs));
  }
  
  return (retval);
}



Classification::AircraftModel *
Classification::Definitions::aircraftModel ()
{
  AircraftModel *retval;
  
  retval = new AircraftModel (this);
  m_aircraft_table.link (retval, false);
  m_object_table.link (retval, true);
  
  return (retval);
}



Classification::AirlineModel *
Classification::Definitions::airlineModel ()
{
  AirlineModel *retval;
  
  retval = new AirlineModel (this);
  m_airline_table.link (retval, false);
  m_object_table.link (retval, true);
  
  return (retval);
}



Classification::LibraryModel *
Classification::Definitions::libraryModel ()
{
  LibraryModel *retval;
  
  retval = new LibraryModel (this);
  m_library_table.link (retval, false);
  m_object_table.link (retval, true);
  
  return (retval);
}



Classification::ObjectModel *
Classification::Definitions::objectModel ()
{
  ObjectModel *retval;
  
  retval = new ObjectModel (this);
  m_object_table.link (retval, false);
  
  return (retval);
}



void
Classification::Definitions::unlinkModel (Model *model)
{
  m_aircraft_table.unlink (model);
  m_airline_table.unlink (model);
  m_airport_table.unlink (model);
  m_library_table.unlink (model);
  m_object_table.unlink (model);
}



QJsonObject
Classification::Definitions::readJson (QString filename)
{
  QFile file (filename);
  bool ok;
  
  QJsonParseError error;
  QByteArray data;
  QJsonDocument doc;
  
  ok = file.open (QFile::ReadOnly);
  if (ok) {
    data = file.readAll ();
    doc = QJsonDocument::fromJson (data, &error);
    file.close ();
    
    if (error.error != QJsonParseError::NoError) {
      throw (std::runtime_error(error.errorString().toStdString()));
    }
  } else {
    throw (std::runtime_error(QString("Cannot open \'%1\'").arg(filename).toStdString()));
  }
  
  return (doc.object ());
}
