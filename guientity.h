#ifndef GUIENTITY_H
#define GUIENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QText2DEntity>

class EntityTransform : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    EntityTransform(Qt3DCore::QEntity *parent);
    Qt3DCore::QTransform *Transform() const;

protected:
    Qt3DCore::QTransform* m_Transform;
};

//TODO: EntityText есть проблема с динамическим удалением объекта со сцены
class EntityText : public EntityTransform
{
public:
    EntityText(Qt3DCore::QEntity *parent,
               const QString& text,
               int size,
               const QColor &color = Qt::white,
               const QString &family = "monospace",
               int weight = QFont::Bold);
    void setText(const QString& text);
    void setTextWeight(int value);
    int Width() const;
    int Height() const;

protected:
    void resize();

private:
    QFont m_Font;
    Qt3DExtras::QText2DEntity* m_Text2DEntity;
    int m_Width;
    int m_Height;
};

class Entity3DText : public EntityTransform
{
    Q_OBJECT

public:
    Entity3DText(Qt3DCore::QEntity *parent,
                 const QString& text,
                 const QSizeF& size,
                 const QFont& font,
                 const QColor &color);
    Entity3DText(Qt3DCore::QEntity *parent,
                 const QString& text,
                 const QSizeF& size,
                 const QColor &color = Qt::white,
                 int pointSize = 20,
                 const QString &family = "monospace",
                 int weight = QFont::Bold,
                 bool italic = false);
    float RealWidth() const;
    float RealHeight() const;
    void resize(const QSizeF& size);
    QRectF getRect() const;

private:
    void init(const QString& text,
              const QSizeF& size,
              const QFont& font,
              const QColor &color);
    QFont m_Font;
    float m_RealWidth;
    float m_RealHeight;    
    QRectF m_Rect;
    int m_LoadingStatus;
    float m_FontMetricWH;

Q_SIGNALS:
        void signalLoaded();

};


#endif // GUIENTITY_H
