#pragma once

#include <QtGlobal>

#include <Obj8/Parameter/Number.hpp>
#include <Obj8/Parameter/Word.hpp>
#include <Obj8/Record.hpp>

namespace Obj8
{
  namespace Command
  {
    namespace Animation
    {
      struct RotationBegin : Record
      {
        RotationBegin ();
        RotationBegin (StringRef, Parser::LexerContext *);
        virtual ~RotationBegin ();
        
        virtual void          accept (AbstractVisitor *, bool) Q_DECL_OVERRIDE;
        virtual RecordPointer instantiate (StringRef, Parser::LexerContext *) const Q_DECL_OVERRIDE;
        virtual String        name () const Q_DECL_OVERRIDE;
        virtual String        toString () const Q_DECL_OVERRIDE;
      
      protected:
        Parameter::Number m_x, m_y, m_z;
        Parameter::Word   m_dataref;
      };
    }
  }
}
