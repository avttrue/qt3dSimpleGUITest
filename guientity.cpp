#include "guientity.h"
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DRender/QGeometry>

EntityTransform::EntityTransform(Qt3DCore::QEntity *parent):
    Qt3DCore::QEntity(parent),
    m_Size(QSizeF()),
    m_Rect(QRectF()),
    m_DefaultWidth(0.0f),
    m_DefaultHeight(0.0f)
{
    m_Transform = new Qt3DCore::QTransform;
    addComponent(m_Transform);
    QObject::connect(this, &QObject::destroyed,
                     [=]() { qDebug() << parent->objectName() << ":" << objectName() << "destroyed"; });
}

QRectF EntityTransform::Rect() const { return m_Rect; }
Qt3DCore::QTransform *EntityTransform::Transform() const { return m_Transform; }

Entity3DText::Entity3DText(Qt3DCore::QEntity *parent,
                           const QSizeF &size,
                           const QFont &font):
    EntityTransform(parent),
    m_Font(font),
    m_LoadingStatus(0)
{  
    setObjectName(QString("Entity3DText"));
    m_Transform->setRotationX(180.0f);
    m_Size = size;

    m_Material = new Qt3DExtras::QDiffuseSpecularMaterial;
    m_Material->setSpecular(QColor(Qt::white));
    addComponent(m_Material);

    m_Mesh = new Qt3DExtras::QExtrudedTextMesh;
    m_Mesh->setFont(m_Font);
    m_Mesh->setDepth(0.0f);
    addComponent(m_Mesh);
}

void Entity3DText::slotWrite(const QString &text,
                             const QColor &color)
{
    m_Material->setAmbient(color);
    m_Material->setDiffuse(color.lighter());

    auto funcExtentChanged = [=]()
    {
        m_LoadingStatus++;
        if(m_LoadingStatus > 1)
        {
            m_LoadingStatus = 0;

            m_DefaultWidth = abs((m_Mesh->geometry()->maxExtent() - m_Mesh->geometry()->minExtent()).x());
            m_DefaultHeight = abs((m_Mesh->geometry()->maxExtent() - m_Mesh->geometry()->minExtent()).y());

            resize();
            emit signalWrited();

            QObject::disconnect(m_Mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, nullptr, nullptr);
            QObject::disconnect(m_Mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, nullptr, nullptr);
        }
    };
    QObject::connect(m_Mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, funcExtentChanged);
    QObject::connect(m_Mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, funcExtentChanged);

    m_Mesh->setText(text);
}

void Entity3DText::resize()
{
    if(m_DefaultWidth <= 0 || m_DefaultHeight <= 0) {qCritical() << __func__ << ": incorrect default size"; return; }
    if(m_Size.width() <= 0 && m_Size.height() <= 0) {qCritical() << __func__ << ": incorrect size"; return; }

    float w_scale = m_Size.width() > 0
                        ? static_cast<float>(m_Size.width()) / m_DefaultWidth
                        : static_cast<float>(m_Size.height()) / m_DefaultHeight;

    float h_scale = m_Size.height() > 0
                        ? static_cast<float>(m_Size.height()) / m_DefaultHeight
                        : static_cast<float>(m_Size.width()) / m_DefaultWidth;

    m_Transform->setScale3D(QVector3D(w_scale, h_scale, 1.0f));

    m_Rect = QRectF(static_cast<qreal>(Transform()->translation().x()),
                    static_cast<qreal>(Transform()->translation().y()),
                    static_cast<qreal>(m_DefaultWidth * w_scale),
                    static_cast<qreal>(m_DefaultHeight * h_scale));
}

EntityButton::EntityButton(Qt3DCore::QEntity *parent,
                           const QSizeF &size,
                           const QColor &color,
                           const QFont &font):
    Entity3DText(parent, QSizeF(size.width() * 0.8, size.height() * 0.6), font)

{
    setObjectName(QString("EntityButton"));
    auto panel = new EntityTransform(this);
    panel->setObjectName("Panel");
    auto materialPanel = new Qt3DExtras::QDiffuseSpecularMaterial;
    materialPanel->setAmbient(color);
    materialPanel->setDiffuse(color.lighter());
    materialPanel->setSpecular(QColor(Qt::white));

    auto meshPanel = new Qt3DExtras::QCuboidMesh;
    meshPanel->setXExtent(1.0f);
    meshPanel->setYExtent(1.0f);
    meshPanel->setZExtent(0.0f);

    panel->addComponent(materialPanel);
    panel->addComponent(meshPanel);
    auto func = [=]()
    {
        m_Transform->setTranslation(
            QVector3D(Transform()->translation().x() + static_cast<float>(m_Rect.width()) * 0.1f,
                      Transform()->translation().y() - static_cast<float>(m_Rect.height()) * 0.2f,
                      0.0f));

        meshPanel->setXExtent(m_DefaultWidth * 1.2f);
        meshPanel->setYExtent(m_DefaultHeight * 1.4f);

        panel->Transform()->setTranslation(
            QVector3D(m_DefaultWidth * 0.5f,
                      m_DefaultHeight * 0.5f,
                      0.01f));

        m_Rect = QRectF(m_Rect.x(), m_Rect.y(), m_Rect.width() * 1.2, m_Rect.height() * 1.4);

        QObject::disconnect(this, &Entity3DText::signalWrited, nullptr, nullptr);
    };
    QObject::connect(this, &Entity3DText::signalWrited, func);
}

