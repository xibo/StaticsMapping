#pragma once

#include <Common/CryptoHash.hpp>

#include "AbstractVisitor.hpp"

namespace Obj8
{
  struct TextureHashingVisitor : AbstractVisitor
  {
    TextureHashingVisitor (QString);
    virtual ~TextureHashingVisitor ();
    
    virtual void visit (Global::Texture *) Q_DECL_OVERRIDE;
    virtual void visit (Global::TextureDraped *) Q_DECL_OVERRIDE;
    virtual void visit (Global::TextureLit *) Q_DECL_OVERRIDE;
    virtual void visit (Global::TextureNormal *) Q_DECL_OVERRIDE;
    
    virtual void reset () Q_DECL_OVERRIDE;
    virtual bool satisfied (StringRef) Q_DECL_OVERRIDE;
    
    int     texturesHashed () const;
    Hash    result () const;
  
  protected:
    void hashTexture (QString);
    
    Hash m_hash;
    int m_processed_textures;
  };
}
