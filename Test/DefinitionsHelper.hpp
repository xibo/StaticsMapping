#pragma once

#include <QtCore/QObject>

#include <Common/Classification/Definitions.hpp>
#include <Common/Classification/Object.hpp>

namespace Classification
{
  namespace Test
  {
    struct DefinitionsHelper : QObject
    {
      Q_OBJECT
    
    public:
      static Hash hash (int i);
      static Object::PrimaryKey objkey (int i);
    
      
      AircraftPointer aircraft(QString) const;
      AirlinePointer airline(QString) const;
      ObjectPointer object (int) const;
      
      AircraftPointer addAircraft (QString);
      AirlinePointer addAirline (QString, QStringList =QStringList());
      AirportPointer addAirport (QString, double, double);
      LibraryPointer addLibrary (QString);
      ObjectPointer addObject (QString, QString, QString, int, bool);
  
    protected slots:
      virtual void init ();
      virtual void cleanup ();
    
    protected:
      DefinitionsPointer m_definitions;
    };
  }
}
