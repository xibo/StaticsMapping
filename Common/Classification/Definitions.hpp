#pragma once

#include <QFile>
#include <QJsonObject>
#include <QMap>
#include <QSharedPointer>

#include <QtCore/QSet>

#include "Classification.hpp"
#include "Record.hpp"

namespace Classification
{
  template <typename Type>
  struct Table
  {
    typedef Record::PrimaryKey      Key;
    typedef QSharedPointer<Type>    ValuePointer;
    typedef QMap<Key, ValuePointer> KVTable;
    typedef QList<ValuePointer>     ForwardMapping;
    typedef QMap<ValuePointer,int>  ReverseMapping;
    typedef Type                    Value;
    
    struct Observer
    {
      Model *model;
      bool   structural_only;
    };
    
    Table (Definitions *);
    Table (const Table &, Definitions *);
    Table (QJsonArray, Definitions *);
    virtual ~Table ();
    
    QJsonArray   toJson () const;
    
    int          clear ();
    int          drop (ValuePointer);
    int          drop (Key, Key (Type::*)() const);
    void         upsert (ValuePointer);
    void         regenerate ();
    
    int          index (ValuePointer) const;
    ValuePointer value (Key) const;
    ValuePointer value (int) const;
    int          size () const;
    
    void         setDefinitions (Definitions *);
    void         link (Model *, bool);
    void         unlink (Model *);
    void         notifyObservers (int);
    
    
    KVTable        sorted;
    ForwardMapping forward;
    ReverseMapping reverse;
    
    QList<Observer>   observers;
    Definitions      *q;
  };
  
  struct Definitions
  {
    typedef QMap<QString, AircraftPointer> Aircrafts;
    typedef QMap<QString, AirlinePointer>  Airlines;
    typedef QMap<QString, ObjectPointer>   Objects;
    
    typedef QMultiMap<QString, ObjectPointer> AirportPopulation;
    typedef QMultiMap<QString, AirlinePointer> AirlinesAtAirports;
    
    Definitions ();
    Definitions (const Definitions &);
    Definitions (QJsonObject);
    ~Definitions ();
    
    QJsonObject toJson () const;
    
    void upsert (AircraftPointer);
    void upsert (AirlinePointer);
    void upsert (AirportPointer);
    void upsert (LibraryPointer);
    void upsert (ObjectPointer);
    
    int drop (AircraftPointer);
    int drop (AirlinePointer);
    int drop (LibraryPointer);
    int drop (ObjectPointer);
    
    int dropObjects ();
    
    AircraftPointer aircraft (QString) const;
    AircraftPointer aircraft (int) const;
    AirlinePointer  airline (QString) const;
    AirlinePointer  airline (int) const;
    AirportPointer  airport (QString) const;
    LibraryPointer  library (QString) const;
    LibraryPointer  library (int) const;
    ObjectPointer   object (QString) const;
    ObjectPointer   object (int) const;
    
    Aircrafts       aircrafts () const;
    Airlines        airlines () const;
    Objects         objects () const;
    
    int             numberOfAircrafts () const;
    int             numberOfAirlines () const;
    int             numberOfLibraries () const;
    int             numberOfObjects () const;
    
    int             indexOf (AirlinePointer) const;
    int             indexOf (ObjectPointer) const;
    
    bool checkConsistency () const;
    
    bool couldMatch (int filesize) const;
    bool couldMatch (Object::Hash filehash) const;
    ObjectPointer match (int filesize, Object::Hash obj8file, Object::Hash textures) const;
    
    void airportsFromJson (QJsonObject);
    void airportsFromFile (QString);
    AirlinesAtAirports airlinesAtAirports () const;
    AirportPopulation airportPopulation () const;
    
    bool              toFile() const;
    bool              toFile(QString filename) const;
    static DefinitionsPointer fromFile (QString =QString());
    
    AircraftModel *aircraftModel ();
    AirlineModel  *airlineModel ();
    LibraryModel  *libraryModel ();
    ObjectModel   *objectModel ();
    
    void unlinkModel (Model *);
    void invalidateModels (int, int);
    
  protected:
    static QJsonObject readJson (QString);
    
    typedef Table<Aircraft> AircraftTable;
    typedef Table<Airline>  AirlineTable;
    typedef Table<Airport>  AirportTable;
    typedef Table<Library>  LibraryTable;
    typedef Table<Object>   ObjectTable;
    
    AirportTable  m_airport_table;
    
    AircraftTable m_aircraft_table;
    AirlineTable  m_airline_table;
    LibraryTable  m_library_table;
    ObjectTable   m_object_table;
  };
}
