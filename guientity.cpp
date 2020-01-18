#include "guientity.h"

#include <QFontMetrics>

EntityTransform::EntityTransform(Qt3DCore::QEntity *parent):
    Qt3DCore::QEntity(parent)
{
    m_Transform = new Qt3DCore::QTransform;
    addComponent(m_Transform);
    QObject::connect(this, &QObject::destroyed,
                     [=]() { qDebug() << parent->objectName() << ":" << objectName() << "destroyed"; });
}

Qt3DCore::QTransform *EntityTransform::Transform() const { return m_Transform; }

void EntityTransform::addComponentToDeep(Qt3DCore::QComponent *comp)
{
    if(!comp) {qCritical() << __func__ << ": component is empty";  return; }

    addComponent(comp);
    for(auto n: childNodes())
    {
        auto e = qobject_cast<Qt3DCore::QEntity*>(n);
        if(e)
        {
            if(qobject_cast<EntityTransform*>(e)) addComponentToDeep(comp);
            else e->addComponent(comp);
        }
    }
}

EntityText::EntityText(Qt3DCore::QEntity *parent, 
                       const QString &text,
                       int size,
                       const QColor &color,
                       const QString &family,
                       int weight):
    EntityTransform(parent),
    m_width(0),
    m_height(0)
{
    setObjectName("EntityText");

    m_Text2DEntity = new Qt3DExtras::QText2DEntity;
    m_Font = QFont(family, size, weight);

    m_Text2DEntity->setFont(m_Font);
    m_Text2DEntity->setColor(color);

    setText(text);

    // обход бага, описано: https://forum.qt.io/topic/92944/qt3d-how-to-print-text-qtext2dentity/7
    m_Text2DEntity->setParent(this);
}

void EntityText::setText(const QString &text)
{
    m_Text2DEntity->setText(text);
    resize();
}

void EntityText::setTextWeight(int value)
{
    m_Font.setWeight(value);
    m_Text2DEntity->setFont(m_Font);
    resize();
}

void EntityText::resize()
{
    QFontMetrics fm(m_Font);
    auto rect = fm.boundingRect(m_Text2DEntity->text());

    m_width = rect.width();
    m_height = rect.height();

    m_Text2DEntity->setHeight(m_height);
    m_Text2DEntity->setWidth(m_width);
}

int EntityText::height() const { return m_height; }
int EntityText::width() const { return m_width; }

