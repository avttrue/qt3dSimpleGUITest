#include "guientity.h"
#include <QFontMetrics>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DRender/QGeometry>

EntityTransform::EntityTransform(Qt3DCore::QEntity *parent):
    Qt3DCore::QEntity(parent),
    m_Size(QSizeF()),
    m_Rect(QRectF()),
    m_RealWidth(0.0f),
    m_RealHeight(0.0f)
{
    m_Transform = new Qt3DCore::QTransform;
    addComponent(m_Transform);
    QObject::connect(this, &QObject::destroyed,
                     [=]() { qDebug() << parent->objectName() << ":" << objectName() << "destroyed"; });
}

QRectF EntityTransform::Rect() const { return m_Rect; }
Qt3DCore::QTransform *EntityTransform::Transform() const { return m_Transform; }
float EntityTransform::RealWidth() const { return m_RealWidth; }
float EntityTransform::RealHeight() const { return m_RealHeight; }

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
}

void Entity3DText::write(const QString &text,
                         const QColor &color)
{
    QFontMetricsF fm(m_Font);
    m_FontMetricWH = static_cast<float>(fm.horizontalAdvance(text) / fm.height());

    auto *material = new Qt3DExtras::QDiffuseSpecularMaterial;
    material->setAmbient(color);
    material->setDiffuse(color.lighter());
    material->setSpecular(QColor(Qt::white));
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

            resize();
            emit signalWrited();

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

void Entity3DText::resize()
{
    if(m_RealWidth <= 0 || m_RealHeight <= 0) {qCritical() << __func__ << ": incorrect real size"; return; }
    if(m_Size.width() <= 0 && m_Size.height() <= 0) {qCritical() << __func__ << ": incorrect size"; return; }

    float w_scale = m_Size.width() > 0
                        ? static_cast<float>(m_Size.width()) / m_RealWidth
                        : static_cast<float>(m_Size.height()) * m_FontMetricWH * 0.5f; // TODO: разобраться с QFontMetrics

    float h_scale = m_Size.height() > 0
                        ? static_cast<float>(m_Size.height()) / m_RealHeight
                        : static_cast<float>(m_Size.width()) / m_FontMetricWH * 0.5f; //

    m_Transform->setScale3D(QVector3D(w_scale, h_scale, 1.0f));

    m_Size = QSizeF(static_cast<qreal>(m_RealWidth * w_scale),
                    static_cast<qreal>(m_RealHeight * h_scale));

    m_Rect = QRectF(static_cast<qreal>(Transform()->translation().x()),
                    static_cast<qreal>(Transform()->translation().y()),
                    m_Size.width(),
                    m_Size.height());
}

EntityButton::EntityButton(Qt3DCore::QEntity *parent,
                           const QSizeF &size,
                           const QColor &color,
                           const QFont &font):
    Entity3DText(parent, QSizeF(size.width() * 0.75, size.height() * 0.5), font)

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
        meshPanel->setXExtent(RealWidth() * 1.5f);
        meshPanel->setYExtent(RealHeight() * 2.0f);

        panel->Transform()->setTranslation(
            QVector3D(meshPanel->xExtent() / 3.0f,
                      meshPanel->yExtent() / 4.0f,
                      0.01f));

        m_Transform->setTranslation(
            QVector3D(Transform()->translation().x() + static_cast<float>(m_Size.width()) * 0.25f,
                      Transform()->translation().y() - static_cast<float>(m_Size.height()) * 0.5f,
                      0.0f));

        m_Rect = QRectF(static_cast<qreal>(m_Rect.x()),
                        static_cast<qreal>(m_Rect.y()),
                        m_Rect.width() * 1.5,
                        m_Rect.height() * 2.0);

        QObject::disconnect(this, &Entity3DText::signalWrited, nullptr, nullptr);
    };
    QObject::connect(this, &Entity3DText::signalWrited, func);
}

