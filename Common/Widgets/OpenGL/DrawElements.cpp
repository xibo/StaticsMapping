#include "DrawElements.hpp"

namespace OpenGL
{
  DrawElements::DrawElements(QVector<int> indices, GLenum mode)
  : m_indices(indices)
  , m_mode(mode)
  {
  }
  
  DrawElements::~DrawElements()
  {
  }
  
  void
  DrawElements::draw(OpenGL::ShaderPointer shader)
  {
    Q_UNUSED(shader);
    
    glDrawElements(m_mode, m_indices.size(), GL_UNSIGNED_INT, &m_indices[0]);
  }
}