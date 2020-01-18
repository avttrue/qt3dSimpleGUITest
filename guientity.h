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

    /*!
     * \brief addComponentToDeep - предназначено для регистрации EntityTransform в Qt3DRender::QLayer.
     * Когда Qt3DRender::QLayer.setRecursive(true) не срабатывает.
     */
    void addComponentToDeep(Qt3DCore::QComponent *comp);

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
    int width() const;
    int height() const;

protected:
    void resize();

private:
    QFont m_Font;
    Qt3DExtras::QText2DEntity* m_Text2DEntity;
    int m_width;
    int m_height;
};


#endif // GUIENTITY_H
