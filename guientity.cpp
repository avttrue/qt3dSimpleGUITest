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

EntityText::EntityText(Qt3DCore::QEntity *parent, 
                       const QString &text,
                       int size,
                       const QColor &color,
                       const QString &family,
                       int weight):
    EntityTransform(parent),
    m_Width(0),
    m_Height(0)
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

    m_Width = rect.width();
    m_Height = rect.height();

    m_Text2DEntity->setHeight(m_Height);
    m_Text2DEntity->setWidth(m_Width);
}

int EntityText::Height() const { return m_Height; }
int EntityText::Width() const { return m_Width; }

