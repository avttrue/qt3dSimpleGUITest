#ifndef GUIENTITY_H
#define GUIENTITY_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QText2DEntity>

const QColor FONT_COLOR = Qt::white;
const QFont FONT = QFont("monospace", 30, QFont::Bold);

class EntityTransform : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    EntityTransform(Qt3DCore::QEntity *parent);
    Qt3DCore::QTransform *Transform() const;
    float RealWidth() const;
    float RealHeight() const;
    QRectF Rect() const;

protected:
    Qt3DCore::QTransform* m_Transform;
    QSizeF m_Size;
    QRectF m_Rect;
    float m_RealWidth;
    float m_RealHeight;
};

class Entity3DText : public EntityTransform
{
    Q_OBJECT

public:
    Entity3DText(Qt3DCore::QEntity *parent, const QSizeF& size, const QFont& font = FONT);
    void write(const QString& text, const QColor &color = FONT_COLOR);

protected:
    void resize();

private:
    QFont m_Font;
    int m_LoadingStatus;
    float m_FontMetricWH;

Q_SIGNALS:
        void signalWrited();

};

class EntityButton : public Entity3DText
{
    Q_OBJECT

public:
    EntityButton(Qt3DCore::QEntity *parent,
                 const QSizeF& size,
                 const QColor &color,
                 const QFont& font = FONT);
};

#endif // GUIENTITY_H
