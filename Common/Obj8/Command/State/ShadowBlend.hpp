#pragma once

#include <QtGlobal>

#include <Obj8/AbstractBlend.hpp>

namespace Obj8
{
  namespace Command
  {
    namespace State
    {
      struct ShadowBlend : AbstractBlend
      {
        ShadowBlend ();
        ShadowBlend (StringRef, Parser::LexerContext *);
        virtual ~ShadowBlend ();
        
        virtual RecordPointer instantiate (StringRef, Parser::LexerContext *) const Q_DECL_OVERRIDE;
        virtual String        name () const Q_DECL_OVERRIDE;
      };
    }
  }
}
