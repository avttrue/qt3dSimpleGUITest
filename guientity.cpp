#include "guientity.h"

#include <QFontMetrics>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QGoochMaterial>
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
                           const QSizeF &size,
                           const QColor &color,
                           const QFont &font):
    EntityTransform(parent),
    m_Size(size),
    m_Font(font),
    m_Rect(QRectF()),
    m_LoadingStatus(0),
    m_RealWidth(0.0f),
    m_RealHeight(0.0f)
{
    setObjectName(QString("Entity3DText:'%1'").arg(text));
    QFontMetrics fm(m_Font);
    auto rect = fm.boundingRect(text);
    m_FontMetricWH = rect.height() == 0 ? 1 : static_cast<float>(rect.width()) / rect.height();

    QObject::connect(this, &Entity3DText::signalLoaded, this, &Entity3DText::slotResize);
    init(text, color);
}

void Entity3DText::init(const QString &text,
                        const QColor &color)
{
    auto *material = new Qt3DExtras::QGoochMaterial;
    material->setDiffuse(color);
    material->setSpecular(color.lighter());
    auto *mesh = new Qt3DExtras::QExtrudedTextMesh;

    mesh->setFont(m_Font);
    mesh->setDepth(0.0f);

    auto funcExtentChanged = [=]()
    {
        m_LoadingStatus++;
        if(m_LoadingStatus > 1)
        {
            m_LoadingStatus = 0;

            m_RealWidth = abs((mesh->geometry()->maxExtent() - mesh->geometry()->minExtent()).x());
            m_RealHeight = abs((mesh->geometry()->maxExtent() - mesh->geometry()->minExtent()).y());

            emit signalLoaded();

            QObject::disconnect(mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, nullptr, nullptr);
            QObject::disconnect(mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, nullptr, nullptr);
        }
    };
    QObject::connect(mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, funcExtentChanged);
    QObject::connect(mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, funcExtentChanged);

    addComponent(material);
    mesh->setText(text);
    addComponent(mesh);
}

void Entity3DText::slotResize()
{
    if(m_RealWidth <= 0 || m_RealHeight <= 0) {qCritical() << __func__ << ": incorrect real size"; return; }
    if(m_Size.width() <= 0 && m_Size.height() <= 0) {qCritical() << __func__ << ": incorrect size"; return; }

    float w_scale = m_Size.width() > 0
                        ? static_cast<float>(m_Size.width()) / m_RealWidth
                        : static_cast<float>(m_Size.height()) * m_FontMetricWH;

    float h_scale = m_Size.height() > 0
                        ? static_cast<float>(m_Size.height()) / m_RealHeight
                        : static_cast<float>(m_Size.width()) / m_FontMetricWH;


    m_Rect = QRectF(static_cast<qreal>(m_Transform->translation().x()),
                    static_cast<qreal>(m_Transform->translation().y()),
                    static_cast<qreal>(m_RealWidth * w_scale),
                    static_cast<qreal>(m_RealHeight * h_scale));

    m_Transform->setScale3D(QVector3D(w_scale, h_scale, 1.0f));
    m_Transform->setRotationX(180.0f);
}

QRectF Entity3DText::getRect() const { return m_Rect; }
float Entity3DText::RealWidth() const { return m_RealWidth; }
float Entity3DText::RealHeight() const { return m_RealHeight; }
