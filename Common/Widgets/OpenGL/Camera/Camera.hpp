#pragma once

#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtCore/QSize>

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>

#include <OpenGL/Object.hpp>

namespace OpenGL
{
  struct Camera : Object
  {
    typedef float Zoom;
    typedef int ZoomStage;
    typedef QList<float> ZoomStages;
    
    Camera(QString name="");
    virtual ~Camera();
    
    QString name() const;
    
    virtual bool wireframe() const;
   
    void move(QVector3D delta) Q_DECL_OVERRIDE;
    void setPosition(QVector3D position) Q_DECL_OVERRIDE;
    void setOrientation(float roll, float pitch, float yaw) Q_DECL_OVERRIDE;
   
    QSize screenDimensions() const;
    void setScreenDimensions(QSize size);
    
    float fovy() const;
    void setFovy(float fovy);
   
    bool ortho() const;
    void setOrtho(bool ortho);
   
    virtual QMatrix4x4 projection() const;
    
    virtual Zoom zoom() const;
    virtual void zoom(bool in);
    
  protected:
    ZoomStages m_zoom_stages;
    ZoomStage  m_zoom_active;
    
    QSize m_screen;
    QString m_name;
    
    float m_fovy;
    bool m_ortho;
  };
  
  typedef QSharedPointer<Camera> CameraPointer;
}