#include "guientity.h"

#include <QFontMetrics>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QGeometry>

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

Entity3DText::Entity3DText(Qt3DCore::QEntity *parent,
                           const QString &text,
                           int pointSize,
                           const QSizeF &size,
                           const QColor &color,
                           const QString &family,
                           int weight) :
    EntityTransform(parent),
    m_RealWidth(0.0f),
    m_RealHeight(0.0f),
    m_Size(0.0, 0.0),
    m_LoadingStatus(0)
{
    setObjectName("Entity3DText");
    m_Font = QFont(family, pointSize, weight);

    auto *material = new Qt3DExtras::QPhongMaterial;
    material->setAmbient(color);
    auto *mesh = new Qt3DExtras::QExtrudedTextMesh;
    mesh->setFont(m_Font);
    mesh->setDepth(0.0f);

    auto funcExtentChanged = [=]()
    {
        m_LoadingStatus++;
        auto xExtent = abs((mesh->geometry()->maxExtent() - mesh->geometry()->minExtent()).x());
        auto yExtent = abs((mesh->geometry()->maxExtent() - mesh->geometry()->minExtent()).y());

        if(m_LoadingStatus > 1)
        {
            m_LoadingStatus = 0;
            m_RealWidth = xExtent;
            m_RealHeight = yExtent;

            resize(size);

            emit signalLoaded();
            QObject::disconnect(mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, nullptr, nullptr);
            QObject::disconnect(mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, nullptr, nullptr);
        }
    };
    QObject::connect(mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, funcExtentChanged);
    QObject::connect(mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, funcExtentChanged);
    mesh->setText(text);

    addComponent(material);
    addComponent(mesh);

    m_Transform->setRotationX(180.0f);
}

void Entity3DText::resize(const QSizeF &size)
{
    float w_scale = static_cast<float>(size.width()) / m_RealWidth;
    float h_scale = static_cast<float>(size.height()) / m_RealHeight;
    m_Transform->setScale3D(QVector3D(w_scale, h_scale, 1.0f));
    m_Size = QSizeF(size);
}

QSizeF Entity3DText::getSize() const { return m_Size; }
float Entity3DText::RealWidth() const { return m_RealWidth; }
float Entity3DText::RealHeight() const { return m_RealHeight; }
