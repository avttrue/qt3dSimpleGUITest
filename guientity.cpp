#include "guientity.h"

#include <QGuiApplication>
#include <QPropertyAnimation>
#include <Qt3DExtras/Qt3DWindow>

EntityGui::EntityGui(Qt3DCore::QEntity *parent):
    Qt3DCore::QEntity(parent),
    m_Window(nullptr),
    m_Rect(QRectF()),
    m_Size(QSizeF()),
    m_SizePosFactor(SizePosFactor::Absolute)
{
    for(auto w: QGuiApplication::allWindows())
    {
        auto window = qobject_cast<Qt3DExtras::Qt3DWindow*>(w);
        if(window) { m_Window = window; break; }
    }
    if(!m_Window) { qCritical() << objectName() << ": main window not detected"; }

    m_Transform = new Qt3DCore::QTransform;
    addComponent(m_Transform);

    QObject::connect(this, &QObject::destroyed,
                     [=]() { qDebug() << parent->objectName() << ":" << objectName() << "destroyed"; });
}

QRectF EntityGui::Rect() const { return m_Rect; }

Entity3DText::Entity3DText(Qt3DCore::QEntity *parent,
                           const QSizeF &size,
                           SizePosFactor sizePosFactor,
                           const QFont &font):
    EntityGui(parent),
    m_Font(font),
    m_DefaultWidth(0.0f),
    m_DefaultHeight(0.0f),
    m_LoadingStatus(0),
    m_Interactive(false),
    m_Animated(false)
{  
    setObjectName(QString("Entity3DText"));
    m_Size = size;
    m_SizePosFactor = sizePosFactor;
    m_Transform->setRotationX(180.0f);

    m_Material = new Qt3DExtras::QDiffuseSpecularMaterial;
    m_Material->setSpecular(QColor(Qt::white));
    addComponent(m_Material);

    m_Mesh = new Qt3DExtras::QExtrudedTextMesh;
    m_Mesh->setFont(m_Font);
    m_Mesh->setDepth(0.0f);
    addComponent(m_Mesh);

    if(m_SizePosFactor == SizePosFactor::Relative)
    {
        QObject::connect(m_Window, &QWindow::widthChanged, this, &Entity3DText::slotResize, Qt::QueuedConnection);
        QObject::connect(m_Window, &QWindow::heightChanged, this, &Entity3DText::slotResize, Qt::QueuedConnection);
    }
}

void Entity3DText::setPosition(QVector2D value)
{
    m_Position = value;

    if(m_SizePosFactor == SizePosFactor::Relative)
    {
        m_Transform->setTranslation(QVector3D(m_Window->size().width() * m_Position.x(),
                                              m_Window->size().height() * m_Position.y(),
                                              TEXT_POSZ));
    }
    else if(m_SizePosFactor == SizePosFactor::Absolute)
    {
        m_Transform->setTranslation(QVector3D(m_Position.x(), m_Position.y(), TEXT_POSZ));
    }

    m_Rect = QRectF(static_cast<qreal>(m_Transform->translation().x()),
                    static_cast<qreal>(m_Transform->translation().y()),
                    static_cast<qreal>(m_DefaultWidth * m_Transform->scale3D().x()),
                    static_cast<qreal>(m_DefaultHeight * m_Transform->scale3D().y()));
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

            slotResize();

            QObject::disconnect(m_Mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, nullptr, nullptr);
            QObject::disconnect(m_Mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, nullptr, nullptr);
        }
    };
    QObject::connect(m_Mesh->geometry(), &Qt3DRender::QGeometry::maxExtentChanged, funcExtentChanged);
    QObject::connect(m_Mesh->geometry(), &Qt3DRender::QGeometry::minExtentChanged, funcExtentChanged);

    m_Mesh->setText(text);
}

void Entity3DText::slotClicked()
{
    if(m_Animated) return;
    m_Animated = true;
    qInfo() << objectName() << "clicked";

    animationClick();
}

void Entity3DText::animationClick()
{
    auto scale = m_Transform->scale3D();
    auto newscale = scale - QVector3D(BUTTON_ANIM_INDENT, BUTTON_ANIM_INDENT, 0.0f);
    auto translation = m_Transform->translation();
    auto newtranslation = translation +
                          QVector3D(m_DefaultWidth * newscale.x() / scale.x() / 2,
                                    - m_DefaultHeight * newscale.y() / scale.y() / 2, 0.0f);

    auto animationScale = new QPropertyAnimation(m_Transform, "scale3D");
    animationScale->setDuration(BUTTON_ANIM_TIME);
    animationScale->setLoopCount(1);
    animationScale->setStartValue(scale);
    animationScale->setEndValue(newscale);
    auto funcScaleBack = [=]()
    {
        animationScale->setStartValue(m_Transform->scale3D());
        animationScale->setEndValue(scale);
        animationScale->start(QAbstractAnimation::DeleteWhenStopped);
    };
    QObject::connect(animationScale, &QPropertyAnimation::finished, funcScaleBack);
    QObject::connect(animationScale, &QAbstractAnimation::destroyed,
                     [=](){ qDebug() << objectName() << "scale animation destroyed"; });
    animationScale->start();

    auto animationPos = new QPropertyAnimation(m_Transform, "translation");
    animationPos->setDuration(BUTTON_ANIM_TIME);
    animationPos->setLoopCount(1);
    animationPos->setStartValue(translation);
    animationPos->setEndValue(newtranslation);

    auto funcPosBack = [=]()
    {
        animationPos->setStartValue(m_Transform->translation());
        animationPos->setEndValue(translation);
        animationPos->start(QAbstractAnimation::DeleteWhenStopped);
    };
    QObject::connect(animationPos, &QAbstractAnimation::finished, funcPosBack);
    QObject::connect(animationPos, &QAbstractAnimation::destroyed,
                     [=]() { m_Animated = false;
                         qDebug() << objectName() << "position animation destroyed";
                         emit signalClicked(); });
    animationPos->start();
}

void Entity3DText::slotResize()
{
    if(m_DefaultWidth <= 0 || m_DefaultHeight <= 0) {qCritical() << __func__ << ": incorrect default size"; return; }
    if(m_Size.width() <= 0 && m_Size.height() <= 0) {qCritical() << __func__ << ": incorrect size"; return; }

    auto size = m_Size;
    if(m_SizePosFactor == SizePosFactor::Relative)
    {
        size = QSizeF(m_Window->size().width() * size.width(),
                      m_Window->size().height() * size.height());
    }

    float scale_w = size.width() > 0
                        ? static_cast<float>(size.width()) / m_DefaultWidth
                        : static_cast<float>(size.height()) / m_DefaultHeight;

    float scale_h = size.height() > 0
                        ? static_cast<float>(size.height()) / m_DefaultHeight
                        : static_cast<float>(size.width()) / m_DefaultWidth;

    m_Transform->setScale3D(QVector3D(scale_w, scale_h, 1.0f));

    setPosition(m_Position);
}

bool Entity3DText::isInteractive() const { return m_Interactive; }
void Entity3DText::Interactive(bool value) { m_Interactive = value; }

EntityPanel::EntityPanel(Qt3DCore::QEntity *parent,
                         const QSizeF &size,
                         const QColor &color,
                         SizePosFactor sizePosFactor):
    EntityGui(parent)
{
    setObjectName(QString("EntityPanel"));
    m_Size = size;
    m_SizePosFactor = sizePosFactor;

    auto material = new Qt3DExtras::QDiffuseSpecularMaterial;
    material->setAmbient(color);
    material->setDiffuse(color.lighter());
    material->setSpecular(QColor(Qt::white));

    m_Mesh = new Qt3DExtras::QCuboidMesh;
    m_Mesh->setZExtent(0.0f);

    addComponent(material);
    addComponent(m_Mesh);

    slotResize();

    if(m_SizePosFactor == SizePosFactor::Relative)
    {
        QObject::connect(m_Window, &QWindow::widthChanged, this, &EntityPanel::slotResize, Qt::QueuedConnection);
        QObject::connect(m_Window, &QWindow::heightChanged, this, &EntityPanel::slotResize, Qt::QueuedConnection);
    }
}

void EntityPanel::setPosition(QVector2D value)
{
    m_Position = value;
    if(m_SizePosFactor == SizePosFactor::Relative)
    {
        m_Transform->setTranslation(QVector3D(m_Window->size().width() * m_Position.x() +
                                                  m_Mesh->xExtent() * 0.5f,
                                              m_Window->size().height() * m_Position.y() +
                                                  m_Mesh->yExtent() * 0.5f, PANEL_POSZ));
    }
    else if(m_SizePosFactor == SizePosFactor::Absolute)
    {
        m_Transform->setTranslation(QVector3D(m_Position.x() + m_Mesh->xExtent() * 0.5f,
                                              m_Position.y() + m_Mesh->yExtent() * 0.5f, PANEL_POSZ));
    }

    m_Rect = QRectF(static_cast<qreal>(m_Transform->translation().x()),
                    static_cast<qreal>(m_Transform->translation().y()),
                    static_cast<qreal>(m_Mesh->xExtent()),
                    static_cast<qreal>(m_Mesh->yExtent()));
}

void EntityPanel::slotResize()
{
    if(m_Size.width() <= 0 && m_Size.height() <= 0) {qCritical() << __func__ << ": incorrect size"; return; }

    auto size = m_Size;
    if(m_SizePosFactor == SizePosFactor::Relative)
    {
        size = QSizeF(m_Window->size().width() * size.width(),
                      m_Window->size().height() * size.height());
    }

    float size_w = size.width() > 0
                       ? static_cast<float>(size.width())
                       : m_Window->size().width();

    float size_h = size.height() > 0
                       ? static_cast<float>(size.height())
                       : m_Window->size().height();

    m_Mesh->setXExtent(size_w);
    m_Mesh->setYExtent(size_h);

    setPosition(m_Position);
}
