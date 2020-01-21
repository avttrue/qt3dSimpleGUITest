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
                           const QFont &font,
                           const QColor &color):
    EntityTransform(parent),
    m_RealWidth(0.0f),
    m_RealHeight(0.0f),
    m_Rect(QRectF()),
    m_LoadingStatus(0)
{
    init(text, size, font, color);
}

Entity3DText::Entity3DText(Qt3DCore::QEntity *parent,
                           const QString &text,
                           const QSizeF &size,
                           const QColor &color,
                           int pointSize,
                           const QString &family,
                           int weight,
                           bool italic) :
    EntityTransform(parent),
    m_RealWidth(0.0f),
    m_RealHeight(0.0f),
    m_Rect(QRectF()),
    m_LoadingStatus(0)
{
    init(text, size, QFont(family, pointSize, weight, italic), color);
}

void Entity3DText::init(const QString &text,
                        const QSizeF &size,
                        const QFont &font,
                        const QColor &color)
{
    setObjectName("Entity3DText");
    auto *material = new Qt3DExtras::QGoochMaterial;
    material->setDiffuse(color);
    material->setSpecular(color.lighter());
    auto *mesh = new Qt3DExtras::QExtrudedTextMesh;
    m_Font = QFont(font);
    mesh->setFont(m_Font);
    mesh->setDepth(0.0f);

    QFontMetrics fm(m_Font);
    auto rect = fm.boundingRect(text);
    m_FontMetricWH = rect.height() == 0 ? 1 : static_cast<float>(rect.width()) / rect.height();

    auto funcExtentChanged = [=]()
    {
        m_LoadingStatus++;
        if(m_LoadingStatus > 1)
        {
            m_LoadingStatus = 0;

            m_RealWidth = abs((mesh->geometry()->maxExtent() - mesh->geometry()->minExtent()).x());
            m_RealHeight = abs((mesh->geometry()->maxExtent() - mesh->geometry()->minExtent()).y());

            resize(size);
            m_Transform->setRotationX(180.0f);

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

void Entity3DText::resize(const QSizeF &size)
{
    if(m_RealWidth <= 0 || m_RealHeight <= 0) {qCritical() << __func__ << ": incorrect real size"; return; }
    if(size.width() <= 0 && size.height() <= 0) {qCritical() << __func__ << ": incorrect size"; return; }

    float w_scale = size.width() > 0
                        ? static_cast<float>(size.width()) / m_RealWidth
                        : static_cast<float>(size.height()) * m_FontMetricWH;

    float h_scale = size.height() > 0
                        ? static_cast<float>(size.height()) / m_RealHeight
                        : static_cast<float>(size.width()) / m_FontMetricWH;

    m_Transform->setScale3D(QVector3D(w_scale, h_scale, 1.0f));
    m_Rect = QRectF(static_cast<qreal>(m_Transform->translation().x()),
                    static_cast<qreal>(m_Transform->translation().y()),
                    static_cast<qreal>(m_RealWidth * w_scale),
                    static_cast<qreal>(m_RealHeight * h_scale));
}

QRectF Entity3DText::getRect() const { return m_Rect; }
float Entity3DText::RealWidth() const { return m_RealWidth; }
float Entity3DText::RealHeight() const { return m_RealHeight; }
