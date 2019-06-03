#include <Common/Classification/Aircraft.hpp>
#include <Common/Classification/Airline.hpp>
#include <Common/Classification/Library.hpp>
#include <Common/Classification/Object.hpp>


#include <Common/Classification/AircraftModel.hpp>
#include <Common/Classification/AirlineModel.hpp>
#include <Common/Classification/CategoryModelDisplay.hpp>
#include <Common/Classification/EngineModel.hpp>
#include <Common/Classification/LibraryModel.hpp>
#include <Common/Classification/ObjectModel.hpp>

#include "Models.hpp"



void
Classification::Test::Models::aircraft ()
{
  /*
   * This needs to remain "over" mdl so its' livetime exceeds it.
   */
  bool signaled_reset;
  
  addAircraft ("XXXX");
  addAircraft ("YYYY");
  addAircraft ("ZZZZ");
  
  Classification::ModelPointer mdl (m_definitions->aircraftModel ());

  mdl->setData (mdl->index (0, mdl->column(Aircraft::NameProperty)), QVariant("Foo"));
  mdl->setData (mdl->index (0, mdl->column(Aircraft::WingspanProperty)), QVariant(7));
  mdl->setData (mdl->index (0, mdl->column(Aircraft::OMGWSProperty)), QVariant(8));
  mdl->setData (mdl->index (0, mdl->column(Aircraft::EngineProperty)), QVariant("Piston"));
  mdl->setData (mdl->index (0, mdl->column(Aircraft::MTOWProperty)), QVariant("99.123"));
  mdl->setData (mdl->index (0, mdl->column(Aircraft::FirstFlightProperty)), QVariant(1990));
  
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::NameProperty))) == "Foo");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::WingspanProperty))) == "7.000");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::OMGWSProperty))) == "8.000");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::EngineProperty))) == "Piston");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::MTOWProperty))) == "99.123");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::FirstFlightProperty))) == "1990");
  
  mdl->setData (mdl->index (0, mdl->column(Aircraft::EngineProperty)), QVariant("Magic Carpet"));
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::EngineProperty))) == "Piston");
  
  mdl->setData (mdl->index (0, mdl->column(Aircraft::IcaoProperty)), QVariant("AAAA"));
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::IcaoProperty))) == "XXXX");
  
  mdl->setData (mdl->index (0, mdl->column(Aircraft::IcaoProperty)), QVariant("AAAA"), Qt::UserRole);
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Aircraft::IcaoProperty)), Qt::UserRole) == "XXXX");
  
  /*
   * signals
   */
  connect (
      mdl.data(), &Classification::AircraftModel::modelReset,
      [&]()
      {
        signaled_reset = true;
      }
  );
  signaled_reset = false;
  addAircraft ("B757");
  QVERIFY (signaled_reset);
  
  signaled_reset = false;
  m_definitions->drop (m_definitions->aircraft ("B757"));
  QVERIFY (signaled_reset);
  
  signaled_reset = false;
  m_definitions->drop (m_definitions->aircraft ("B757"));
  QVERIFY (not signaled_reset);
  
  
  m_definitions->drop (m_definitions->aircraft ("XXXX"));
  
  /*
   * no stale pointer access
   */
  QVERIFY (mdl->data(mdl->index (0, 0), Qt::UserRole) == "YYYY");
  QVERIFY (mdl->data(mdl->index (1, 0), Qt::UserRole) == "ZZZZ");
  QVERIFY (mdl->data(mdl->index (2, 0), Qt::UserRole) == QVariant());
}


void
Classification::Test::Models::airline ()
{
  addAirline ("XXX");
  addAirline ("YYY");
  addAirline ("ZZZ");
  
  /*
   * These two bools need to remain "over" mdl so their livetime exceeds it.
   */
  bool signaled_structural_change, signaled_reset;
  
  Classification::AirlineModelPointer mdl (m_definitions->airlineModel ());

  //mdl->setData (mdl->index (0, Classification::AirlineModel::Area), QVariant("UUEE SEPA FAOR MMMX fictive"));
  mdl->setData (mdl->index (0, mdl->column(Airline::CommentProperty)), QVariant("Hello World\n\n"));
  mdl->setData (mdl->index (0, mdl->column(Airline::CeasedProperty)), QVariant(1999));
  mdl->setData (mdl->index (0, mdl->column(Airline::FoundedProperty)), QVariant(1990));
  mdl->setData (mdl->index (0, mdl->column(Airline::NameProperty)), QVariant("My Airline Instance"));
  mdl->setData (mdl->index (0, mdl->column(Airline::ParentProperty)), QVariant("YYY"));
  
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::CommentProperty))) == "Hello World\n\n");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::CeasedProperty))) == 1999);
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::FoundedProperty))) == 1990);
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::NameProperty))) == "My Airline Instance");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::ParentProperty))) == "YYY");
  
  /*
   * Reject non-integer input
   */
  mdl->setData (mdl->index (0, mdl->column(Airline::CeasedProperty)), QVariant("Hello"));
  mdl->setData (mdl->index (0, mdl->column(Airline::FoundedProperty)), QVariant("World"));
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::CeasedProperty))) == "1999");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::FoundedProperty))) == "1990");
  
  /*
   * No icao change via model.
   */
  mdl->setData (mdl->index (0, mdl->column(Airline::IcaoProperty)), QVariant("XXY"));
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::IcaoProperty))) == "XXX");
  

  /*
   * Reject poor parent.
   */
  mdl->setData (mdl->index (0, mdl->column(Airline::ParentProperty)), "ABC");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::ParentProperty))) == "YYY");
  
  /*
   * UserRole
   */
  QVERIFY (mdl->data(mdl->index (0, 0), Qt::UserRole) == mdl->data(mdl->index (0, mdl->column(Airline::IcaoProperty))));
  QVERIFY (mdl->data(mdl->index (0, 0), Qt::UserRole) == mdl->data(mdl->index (0, 1), Qt::UserRole));
  
  
  /*
   * signals
   */
  signaled_structural_change = false;
  signaled_reset = false;
  
  connect (
      mdl.data(), &Classification::AirlineModel::structuralChange,
      [&]()
      {
        signaled_structural_change = true;
      }
  );
  connect (
      mdl.data(), &Classification::AirlineModel::modelReset,
      [&]()
      {
        signaled_reset = true;
      }
  );
  
  mdl->setData (mdl->index (0, mdl->column(Airline::ParentProperty)), "ABC");
  QVERIFY (not signaled_reset);
  QVERIFY (not signaled_structural_change);
  
  mdl->setData (mdl->index (0, mdl->column(Airline::ParentProperty)), "ZZZ");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Airline::ParentProperty))) == "ZZZ");
  QVERIFY (not signaled_reset);
  QVERIFY (signaled_structural_change);
  
  /*
   * deleting airlines is a structural change.
   */
  signaled_reset = false;
  signaled_structural_change = false;
  m_definitions->drop (m_definitions->airline ("ZZZ"));
  QVERIFY (signaled_reset);
  QVERIFY (signaled_structural_change);
  
  /*
   * no stale pointer access
   * 
   *   XXX was referencing ZZZ and was deleted together with it.
   */
  QVERIFY (mdl->data(mdl->index (0, 0), Qt::UserRole) == "YYY");
  QVERIFY (mdl->data(mdl->index (1, 0), Qt::UserRole) == QVariant());
  QVERIFY (mdl->data(mdl->index (2, 0), Qt::UserRole) == QVariant());
}


void
Classification::Test::Models::library ()
{
  bool changed, reset;
  LibraryModelPointer mdl (m_definitions->libraryModel());
  
  changed = reset = false;
  
  connect (
      mdl.data(), &LibraryModel::dataChanged,
      [&]()
      {
        changed = true;
      }
  );
  connect (
      mdl.data(), &LibraryModel::modelReset,
      [&]()
      {
        reset = true;
      }
  );
  
  addLibrary("FooLib");
  QVERIFY (reset);
  QVERIFY (not changed);
  
  /*
   * change using the model
   */
  changed = reset = false;
  mdl->setData (mdl->index(0, mdl->column(Library::NamePropery)), "Hello World.");
  QVERIFY (not reset);
  QVERIFY (changed);
  
  /*
   * change outside the model
   */
  changed = reset = false;
  LibraryPointer lp = m_definitions->library(0);
  lp->setVersion("1.2.3");
  m_definitions->upsert (lp);
  QVERIFY (not reset);
  QVERIFY (changed);
  
  /*
   * set/get
   */
  mdl->setData (mdl->index(0, mdl->column(Library::NamePropery)), "NAME");
  mdl->setData (mdl->index(0, mdl->column(Library::VersionProperty)), "VERSION");
  mdl->setData (mdl->index(0, mdl->column(Library::UrlProperty)), "URL");
  QVERIFY (mdl->data(mdl->index(0, mdl->column(Library::NamePropery))) == "NAME");
  QVERIFY (mdl->data(mdl->index(0, mdl->column(Library::VersionProperty))) == "VERSION");
  QVERIFY (mdl->data(mdl->index(0, mdl->column(Library::UrlProperty))) == "URL");
  
  /*
   * reject primary Key write
   */
  changed = reset = false;  
  mdl->setData (mdl->index(0, mdl->column(Library::KeyProperty)), "new-key");
  QVERIFY (mdl->data(mdl->index(0, mdl->column(Library::KeyProperty))) == "FooLib");
  QVERIFY (not changed);
  QVERIFY (not reset);
}


void
Classification::Test::Models::object ()
{
  addObject ("AAL", "B731", "mylib", 0, true);
  addObject ("AAL", "B731", "mylib", 1, true);
  addObject ("AAL", "B732", "mylib", 2, true);
  addObject ("AFL", "B733", "mylib", 3, true);
  addLibrary ("otherlib");
  
  /*
   * This needs to remain "over" mdl so its' livetime exceeds it.
   */
  bool signaled_reset;
  
  Classification::ModelPointer mdl (m_definitions->objectModel ());
  
  mdl->setData (mdl->index (0, mdl->column(Object::FilenameProperty)), QVariant("magic-carpet.txt"));
  mdl->setData (mdl->index (0, mdl->column(Object::AircraftProperty)), QVariant("B733"));
  mdl->setData (mdl->index (0, mdl->column(Object::LiveryProperty)), QVariant("AFL"));
  mdl->setData (mdl->index (0, mdl->column(Object::LibraryProperty)), QVariant("otherlib"));
  mdl->setData (mdl->index (0, mdl->column(Object::CommentProperty)), QVariant("☆★＊★☆"));
  mdl->setData (mdl->index (0, mdl->column(Object::IntroductionProperty)), QVariant("123"));
  mdl->setData (mdl->index (0, mdl->column(Object::RetirementProperty)), QVariant("3456"));
  mdl->setData (mdl->index (0, mdl->column(Object::PurposeProperty)), QVariant("MultiPurpose"));
  mdl->setData (mdl->index (0, mdl->column(Object::FictiveProperty)), QVariant("false"));
  mdl->setData (mdl->index (0, mdl->column(Object::TranslationXProperty)), QVariant("001.0"));
  mdl->setData (mdl->index (0, mdl->column(Object::TranslationYProperty)), QVariant("+1.3"));
  mdl->setData (mdl->index (0, mdl->column(Object::TranslationZProperty)), QVariant("  -12.08"));
  
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::FilenameProperty))) == "magic-carpet.txt");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::AircraftProperty))) == "B733");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::LiveryProperty))) == "AFL");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::LibraryProperty))) == "otherlib");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::CommentProperty))) == "☆★＊★☆");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::IntroductionProperty))) == "123");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::RetirementProperty))) == "3456");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::PurposeProperty))) == "MultiPurpose");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::FictiveProperty))) == "false");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::TranslationXProperty))) == "001.0");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::TranslationYProperty))) == "+1.3");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::TranslationZProperty))) == "  -12.08");
  
  /*
   * Reject poor foreign keys.
   */
  mdl->setData (mdl->index (0, mdl->column(Object::AircraftProperty)), QVariant("B775"));
  mdl->setData (mdl->index (0, mdl->column(Object::LiveryProperty)), QVariant("SBI"));
  mdl->setData (mdl->index (0, mdl->column(Object::LibraryProperty)), QVariant("yetanotherlib"));
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::AircraftProperty))) == "B733");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::LiveryProperty))) == "AFL");
  QVERIFY (mdl->data(mdl->index (0, mdl->column(Object::LibraryProperty))) == "otherlib");
  
  /*
   * No primary key writes via model.
   */
  mdl->setData (mdl->index (0, mdl->column(Object::CompositehashProperty)), QVariant(objkey(4)));
  QCOMPARE (mdl->data(mdl->index (0, mdl->column(Object::CompositehashProperty))).toString(), objkey(0));
  /*
   * No writes to elements making up the primary key, either
   */
  mdl->setData (mdl->index (0, mdl->column(Object::FilesizeProperty)), 12);
  QCOMPARE (mdl->data(mdl->index (0, mdl->column(Object::FilesizeProperty))).toInt(), 1);
  
  
  /*
   * Be triggered on deep removal via foreign key purge.
   * 
   *   The objkey(3) plane is a 733 and will also be dropped. Nevertheless
   *   the signal is triggered exactly once despite multiple records having
   *   been removed.
   */
  
  connect (
    mdl.data(), &Classification::ObjectModel::modelReset,
    [&signaled_reset] ()
    {
      signaled_reset = true;
    }
  );
  
  signaled_reset = false;
  m_definitions->drop (m_definitions->aircraft ("B733"));
  QVERIFY (signaled_reset);
  
  QVERIFY (mdl->data(mdl->index (0, 0), Qt::UserRole) == objkey(1));
  QVERIFY (mdl->data(mdl->index (1, 0), Qt::UserRole) == objkey(2));
  QVERIFY (mdl->data(mdl->index (2, 0), Qt::UserRole) == QVariant());
  QVERIFY (mdl->data(mdl->index (3, 0), Qt::UserRole) == QVariant());
}



void
Classification::Test::Models::unlink ()
{
  QSharedPointer<Model> array[] = {
    QSharedPointer<Model> (m_definitions->aircraftModel()),
    QSharedPointer<Model> (m_definitions->airlineModel()),
    QSharedPointer<Model> (m_definitions->libraryModel()),
    QSharedPointer<Model> (m_definitions->objectModel()),
    /*
     * many
     */
    QSharedPointer<Model> (m_definitions->objectModel()),
    QSharedPointer<Model> (m_definitions->objectModel()),
    QSharedPointer<Model> (m_definitions->objectModel())
  };
}

QTEST_MAIN (Classification::Test::Models);
