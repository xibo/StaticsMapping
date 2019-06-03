#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QDate>
#include <QtCore/QJsonObject>

#include <CryptoHash.hpp>
#include "Classification.hpp"
#include "Constants.hpp"
#include "Record.hpp"

#include "Obj8/Parser/Number.hpp"

namespace Classification
{
  struct Object : Record
  {
    Q_OBJECT
    
  public:
    enum PurposeBits {
      GeneralAviationBit,
      AirlinerBit,
      FreighterBit,
      
      AttackBit,
      BomberBit,
      EWBit,
      FighterBit,
      MPBit,
      AEWBit,
      ReconBit,
      SurveillanceBit,
      TankerBit,
      TrainerBit,
      TransportBit,
    };
    
    enum Purpose {
      GeneralAviation   = 1 << GeneralAviationBit,
      Airliner          = 1 << AirlinerBit,
      Freighter         = 1 << FreighterBit,
      
      Attack            = 1 << AttackBit,
      Bomber            = 1 << BomberBit,
      ElectronicWarfare = 1 << EWBit,
      Fighter           = 1 << FighterBit,
      MaritimePatrol    = 1 << MPBit,
      AEW               = 1 << AEWBit,
      Reconnaisance     = 1 << ReconBit,
      Surveillance      = 1 << SurveillanceBit,
      Tanker            = 1 << TankerBit,
      Trainer           = 1 << TrainerBit,
      MilitaryTransport = 1 << TransportBit,
      
      MultiPurpose      = Airliner | Freighter
    };
    
    Q_ENUM(Purpose)
    Q_DECLARE_FLAGS(Purposes, Purpose)
    Q_FLAG(Purposes)
    
    enum Rotate {
      RotateNull,
      RotateCCW90,
      RotateCCW180,
      RotateCCW270
    };
    Q_ENUM(Rotate)
    
    Q_PROPERTY (QString  aircraft      MEMBER m_aircraft WRITE setAircraft)
    Q_PROPERTY (QString  livery        MEMBER m_livery WRITE setLivery)
    Q_PROPERTY (QString  library       MEMBER m_library WRITE setLibrary)
    Q_PROPERTY (QString  comment       MEMBER m_comment)
    Q_PROPERTY (QString  compositehash READ compositeHash)
    Q_PROPERTY (QString  filename      READ fileName WRITE setFileName)
    // Q_PROPERTY (QString  filehash      READ fileHash)
    Q_PROPERTY (int      filesize      READ fileSize)
    // Q_PROPERTY (QString  texturehash   READ textureHash)
    Q_PROPERTY (Purposes purpose       READ purpose WRITE setPurpose)
    Q_PROPERTY (bool     fictive       MEMBER m_fictive)
    Q_PROPERTY (int      introduction  MEMBER m_introduced)
    Q_PROPERTY (int      retirement    MEMBER m_retired)
    Q_PROPERTY (Rotate   rotate        READ rotate WRITE setRotate)
    Q_PROPERTY (QString  translateX    READ translateX WRITE setTranslateX)
    Q_PROPERTY (QString  translateY    READ translateY WRITE setTranslateY)
    Q_PROPERTY (QString  translateZ    READ translateZ WRITE setTranslateZ)
    
    enum PropertyName {
      AircraftProperty,
      LiveryProperty,
      LibraryProperty,
      CommentProperty,
      CompositehashProperty,
      FilenameProperty,
      //FilehashProperty,
      FilesizeProperty,
      //TexturehashProperty,
      PurposeProperty,
      FictiveProperty,
      IntroductionProperty,
      RetirementProperty,
      RotateProperty,
      TranslationXProperty,
      TranslationYProperty,
      TranslationZProperty
    };
    static const char *propertyByName (PropertyName);
    
    typedef int     Size;
    
    Object (const Object &);
    Object (Size filesize=-1, Hash filehash=Hash(), Hash refshash=Hash());
    Object (Definitions *, const QJsonObject &);
    virtual ~Object ();
    
    virtual QJsonObject toJson () const Q_DECL_OVERRIDE;
    
    virtual const char *primaryKeyProperty () const Q_DECL_OVERRIDE;
    virtual bool verifyPrimaryKey (PrimaryKey) const Q_DECL_OVERRIDE;
    
    QString aircraft () const;
    void setAircraft (QString);
    
    QString livery () const;
    void setLivery (QString);
    
    QString library () const;
    void setLibrary (QString);
    
    QString comment () const;
    void setComment (QString);
    
    QString fileName () const;
    void setFileName (QString);
  
    Hash fileHash () const;
    void setFileHash (Hash);
  
    Size fileSize () const;
    void setFileSize (Size);
    
    Hash textureHash () const;
    void setTextureHash (Hash);
    
    QString compositeHash () const;
    static QString compositeHash (int filesize, Hash filehash, Hash refshash);
    
    int introduced () const;
    void setIntroduced (int);
    
    int retired () const;
    void setRetired (int);
    
    bool fictive () const;
    void setFictive (bool);
    
    Purposes purpose () const;
    void     setPurpose (Purposes);
    
    Rotate rotate () const;
    void   setRotate (Rotate);
    
    Obj8::Parser::Number translate (size_t) const;
    void setTranslate (Obj8::Parser::Number, size_t);
    
    void    setTranslateX (QString);
    void    setTranslateY (QString);
    void    setTranslateZ (QString);
    QString translateX () const;
    QString translateY () const;
    QString translateZ () const;
    
    QString name () const;
    
  protected:
    virtual void fixReferences () Q_DECL_OVERRIDE;
    
    
  protected:
    QString m_aircraft;
    QString m_livery;
    QString m_library;
    QString m_comment;
    QString m_filename;
    Hash    m_filehash;
    Hash    m_texturehash;
    int     m_introduced;
    int     m_retired;
    Size    m_size;
    bool    m_fictive;
    Purposes m_purpose;
    Rotate  m_rotate;
    Obj8::Parser::Number m_translate_x, m_translate_y, m_translate_z;
  };
}

