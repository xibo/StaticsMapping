#include <Common/Classification/Aircraft.hpp>
#include <Common/Classification/Airline.hpp>
#include <Common/Classification/Airport.hpp>
#include <Common/Classification/Library.hpp>
#include <Common/Classification/Object.hpp>
#include <Common/CryptoHash.hpp>

#include "DefinitionsHelper.hpp"



Hash
Classification::Test::DefinitionsHelper::hash (int i)
{
  QByteArray ba ((const char *)(&i), sizeof(i));
  return (Hash(ba));
}



Classification::Object::PrimaryKey
Classification::Test::DefinitionsHelper::objkey (int i)
{
  Hash h(hash(i));
  return (Object (1, h, h).primaryKey());
}



Classification::AircraftPointer
Classification::Test::DefinitionsHelper::aircraft (QString icao) const
{
  return (m_definitions->aircraft(icao));
}



Classification::AirlinePointer
Classification::Test::DefinitionsHelper::airline (QString icao) const
{
  return (m_definitions->airline(icao));
}



Classification::ObjectPointer
Classification::Test::DefinitionsHelper::object (int i) const
{
  Object::PrimaryKey k = objkey (i);
  ObjectPointer retval = m_definitions->object (k);
  return (retval);
}



Classification::AircraftPointer
Classification::Test::DefinitionsHelper::addAircraft (QString icao)
{
  if (m_definitions->aircraft(icao).isNull()) {
    m_definitions->upsert (
        AircraftPointer (
            new Aircraft (icao)
        )
    );
  }
  return (aircraft(icao));
}



Classification::AirlinePointer
Classification::Test::DefinitionsHelper::addAirline (QString icao, QStringList hubs)
{
  if (m_definitions->airline(icao).isNull()) {
    m_definitions->upsert (
        AirlinePointer (
            new Airline (icao)
        )
    );
    airline(icao)->setHubsList(hubs);
  }
  return (airline(icao));
}



Classification::AirportPointer
Classification::Test::DefinitionsHelper::addAirport (QString icao, double lat, double lon)
{
  if (m_definitions->airport(icao).isNull()) {
    m_definitions->upsert (
        AirportPointer (
            new Airport (icao, icao, lat, lon)
        )
    );
  }
  return (m_definitions->airport(icao));
}



Classification::LibraryPointer
Classification::Test::DefinitionsHelper::addLibrary (QString key)
{
  if (m_definitions->library(key).isNull()) {
    m_definitions->upsert (
        LibraryPointer (
            new Library (key)
        )
    );
  }
  return (m_definitions->library(key));
}



Classification::ObjectPointer
Classification::Test::DefinitionsHelper::addObject (
    QString livery,
    QString aircraft,
    QString library,
    int id,
    bool auto_add
)
{
  if (auto_add) {
    addAircraft (aircraft);
    addAirline (livery, QStringList());
    addLibrary (library);
  }
  
  ObjectPointer obj (new Object (1, hash(id), hash(id)));
  obj->setLivery (livery);
  obj->setAircraft (aircraft);
  obj->setLibrary (library);
  obj->setPurpose (Object::Airliner);
  
  m_definitions->upsert (obj);
  
  return (object(id));
}



void
Classification::Test::DefinitionsHelper::init ()
{
  m_definitions.reset (new Definitions ());
}



void
Classification::Test::DefinitionsHelper::cleanup ()
{
  m_definitions.reset ();
}
