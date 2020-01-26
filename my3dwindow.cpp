#include "my3dwindow.h"
#include "guientity.h"

#include <QResizeEvent>

#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QSortPolicy>

// Test()
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

My3DWindow::My3DWindow(QScreen *screen):
    Qt3DExtras::Qt3DWindow(screen),
    m_Scene(nullptr),
    m_CameraMain(nullptr),
    m_CameraGui(nullptr),
    m_LayerMain(nullptr),
    m_LayerGui(nullptr),
    m_MouseButtonPressEnabled(true)
{
    setObjectName(metaObject()->className());
    setSurfaceType(QSurface::OpenGLSurface);
    installEventFilter(this);
}

bool My3DWindow::eventFilter(QObject* object, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::Resize:
    {
        auto e = static_cast<QResizeEvent*>(event);
        if(!e) { qCritical() << "Resize Event error"; return true; }

        setMainCameraProjection(e->size().width(), e->size().height());
        setGuiCameraProjection();

        return QObject::eventFilter(object, event);
    }
    case QEvent::KeyPress:
    {
        auto e = static_cast<QKeyEvent*>(event);
        if(!e) { qCritical() << "Key Event error"; return true; }

        if(e->key() == Qt::Key_Space) createScene();

        else if(e->key() == Qt::Key_W && testTransform)
        {
            auto pos = testTransform->translation() + QVector3D(0.0f, 0.0f, -0.1f);
            testTransform->setTranslation(pos);
            qDebug() << testTransform->translation();
        }
        else if(e->key() == Qt::Key_S && testTransform)
        {
            auto pos = testTransform->translation() + QVector3D(0.0f, 0.0f, 0.1f);
            testTransform->setTranslation(pos);
            qDebug() << testTransform->translation();
        }

        return !QObject::eventFilter(object, event);
    }
    case QEvent::MouseMove:
    {
        return QObject::eventFilter(object, event);
    }
    case QEvent::MouseButtonPress:
    {
        auto e = static_cast<QMouseEvent*>(event);
        if(!e) { qCritical() << "Mouse Event error"; return true; }

        if(captionText)
        captionText->slotWrite(QString("X: %1 Y: %2").
                               arg(QString::number(e->pos().x()), QString::number(e->pos().y())));
        qDebug() << e->pos();

        for(auto entity: m_GuiList)
        {
            auto guiEntity = qobject_cast<Entity3DText*>(entity);
            if(!guiEntity) continue;
            if(!guiEntity->isInteractive()) continue;

            auto rect = guiEntity->Rect();
            rect.moveTop(guiEntity->Rect().y() - guiEntity->Rect().height());
            if(rect.contains(e->pos())) guiEntity->slotClicked();
        }

        // если нажали в GUI
        //MouseButtonPressEnabled = false;

        return QObject::eventFilter(object, event);
    }
    case QEvent::MouseButtonRelease:
    {
        // если отжали в GUI
        //MouseButtonPressEnabled = true;
        return QObject::eventFilter(object, event);
    }
    default: {return QObject::eventFilter(object, event);}
    }
}

void My3DWindow::createScene()
{
    m_GuiList.clear();
    if(m_Scene) m_Scene->deleteLater();
    createFramegraph();
    setRootEntity(m_Scene);
    captionText = creatTextGUI("X: - Y: -", QSizeF(0, 15));

    // tests
    auto text = creatTextGUI("TEST2", QSizeF(150, 50), Qt::red, QVector2D(100.0f, 150.0f));
    text->Interactive(true);
    auto btn = creatButtonGUI("Button1", QSizeF(150, 0), Qt::blue, Qt::white, QVector2D(100.0f, 200.0f));
    btn->Interactive(true);
    Test1();
    Test2();
}

Entity3DText* My3DWindow::creatTextGUI(const QString& text,
                                          const QSizeF& size,
                                          const QColor& color,
                                          const QVector2D& position)
{
    if(!m_Scene) {qCritical() << "Scene is empty"; return nullptr; }

    auto entity = new Entity3DText(m_Scene, size);
    addToGuiList(entity);

    auto pos = position;
    if(pos == QVector2D(0.0f, 0.0f)) pos = QVector2D(0.0f, static_cast<float>(size.height()));
    entity->Transform()->setTranslation(QVector2D(pos));
    entity->slotWrite(text, color);
    return entity;
}

EntityButton *My3DWindow::creatButtonGUI(const QString &text,
                                         const QSizeF &size,
                                         const QColor &color,
                                         const QColor &textColor,
                                         const QVector2D &position)
{
    auto entity = new EntityButton(m_Scene, size, color);
    addToGuiList(entity);
    entity->Transform()->setTranslation(QVector2D(position));
    entity->slotWrite(text, textColor);
    return entity;
}

void My3DWindow::createFramegraph()
{
    m_Scene = new Qt3DCore::QEntity;
    m_Scene->setObjectName("Scene");
    QObject::connect(m_Scene, &QObject::destroyed, [=](QObject* o){ qDebug() << o->objectName() << ": destroyed"; });

    auto renderSurfaceSelector = new Qt3DRender::QRenderSurfaceSelector;

    auto sortPolicy = new Qt3DRender::QSortPolicy(renderSurfaceSelector);
    sortPolicy->setSortTypes(QVector<Qt3DRender::QSortPolicy::SortType>() << Qt3DRender::QSortPolicy::BackToFront);

    renderSettings()->pickingSettings()->setPickMethod(Qt3DRender::QPickingSettings::TrianglePicking);
    renderSettings()->setRenderPolicy(Qt3DRender::QRenderSettings::OnDemand);

    auto viewport = new Qt3DRender::QViewport(renderSurfaceSelector);
    viewport->setNormalizedRect(QRectF(0.0, 0.0, 1.0, 1.0));

    auto clearBuffers = new Qt3DRender::QClearBuffers(viewport);
    clearBuffers->setBuffers(Qt3DRender::QClearBuffers::AllBuffers);
    clearBuffers->setClearColor(Qt::lightGray);

    auto renderStateSet = new Qt3DRender::QRenderStateSet(clearBuffers);
    auto renderCullFace = new Qt3DRender::QCullFace(renderStateSet);
    renderCullFace->setMode(Qt3DRender::QCullFace::NoCulling);
    renderStateSet->addRenderState(renderCullFace);

    new Qt3DRender::QNoDraw(clearBuffers);

    auto cameraSelectorMain = new Qt3DRender::QCameraSelector(viewport);
    m_CameraMain = new Qt3DRender::QCamera(cameraSelectorMain);
    m_CameraMain->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
    m_CameraMain->setPosition(QVector3D(0.0f, 0.0f, 100.0f));
    m_CameraMain->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    m_CameraMain->lens()->setNearPlane(0.1f);
    m_CameraMain->lens()->setFarPlane(1000.0f);
    setMainCameraProjection(width(), height());
    cameraSelectorMain->setCamera(m_CameraMain);

    m_LayerMain = new Qt3DRender::QLayer(cameraSelectorMain);
    m_LayerMain->setRecursive(true);
    auto filterMain = new Qt3DRender::QLayerFilter(cameraSelectorMain);
    filterMain->addLayer(m_LayerMain);

    auto cameraSelectorGui = new Qt3DRender::QCameraSelector(viewport);
    m_CameraGui = new Qt3DRender::QCamera(cameraSelectorGui);
    m_CameraGui->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);
    m_CameraGui->setPosition(QVector3D(0.0f, 0.0f, 0.0f));
    m_CameraGui->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    m_CameraGui->lens()->setLeft(0);
    m_CameraGui->lens()->setTop(0);
    m_CameraGui->lens()->setNearPlane(-1.0f);
    m_CameraGui->lens()->setFarPlane(1.0f);
    setGuiCameraProjection();
    cameraSelectorGui->setCamera(m_CameraGui);

    m_LayerGui = new Qt3DRender::QLayer(cameraSelectorGui);
    m_LayerGui->setRecursive(true);
    auto filterGui = new Qt3DRender::QLayerFilter(cameraSelectorGui);
    filterGui->addLayer(m_LayerGui);

    renderSurfaceSelector->setSurface(this);
    setActiveFrameGraph(renderSurfaceSelector);

    auto cameraControllerMain = new Qt3DExtras::QFirstPersonCameraController(m_CameraMain);
    cameraControllerMain->setCamera(m_CameraMain);
}

void My3DWindow::setGuiCameraProjection()
{
    if (!m_Scene) return;
    if (!m_CameraGui) {qCritical() << "GUI camera is empty"; return; }

    m_CameraGui->lens()->setRight(static_cast<float>(width() * devicePixelRatio()));
    m_CameraGui->lens()->setBottom(static_cast<float>(height() * devicePixelRatio()));
}

void My3DWindow::setMainCameraProjection(int width, int height)
{
    if (!m_Scene) return;
    if (!m_CameraMain) {qCritical() << "Main camera is empty"; return; }

    m_CameraMain->lens()->setAspectRatio(static_cast<float>(width) / height);
}

void My3DWindow::Test1()
{
    auto sphere = new Qt3DCore::QEntity(m_Scene);
    testTransform = new Qt3DCore::QTransform;
    testTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
    auto sphereMaterial = new Qt3DExtras::QPhongAlphaMaterial;
    sphereMaterial->setAmbient(Qt::blue);
    auto sphereMesh = new Qt3DExtras::QSphereMesh;
    sphereMesh->setRadius(5);
    sphereMesh->setSlices(16);
    sphereMesh->setRings(16);

    auto spherePicker = new Qt3DRender::QObjectPicker;
    QObject::connect(spherePicker, &Qt3DRender::QObjectPicker::clicked, [=](){if(m_MouseButtonPressEnabled) qDebug() << "sphere clicked"; });

    sphere->addComponent(sphereMaterial);
    sphere->addComponent(sphereMesh);
    sphere->addComponent(testTransform);
    sphere->addComponent(spherePicker);
    sphere->addComponent(m_LayerMain);
}

void My3DWindow::Test2()
{
    // cube
    auto cube = new Qt3DCore::QEntity(m_Scene);
    auto cubeTransform = new Qt3DCore::QTransform;
    cubeTransform->setTranslation(QVector3D(175.0f, 275.0f, -0.01f));
    auto cubeMaterial = new Qt3DExtras::QGoochMaterial;
    cubeMaterial->setDiffuse(Qt::green);
    cubeMaterial->setSpecular(QColor(Qt::green).lighter());
    auto cubeMesh = new Qt3DExtras::QCuboidMesh;
    cubeMesh->setXExtent(150);
    cubeMesh->setYExtent(150);
    cubeMesh->setZExtent(0);

    cube->addComponent(cubeMaterial);
    cube->addComponent(cubeMesh);
    cube->addComponent(cubeTransform);
    cube->addComponent(m_LayerGui);
}

void My3DWindow::addToGuiList(Qt3DCore::QEntity *entity)
{
    entity->addComponent(m_LayerGui);
    m_GuiList.append(entity);
    qInfo() << objectName() << "Gui list count:" << m_GuiList.count();
}

void My3DWindow::removeFromGuiList(Qt3DCore::QEntity *entity)
{
    entity->removeComponent(m_LayerGui);
    m_GuiList.removeAll(entity);
    qInfo() << objectName() << "Gui list count:" << m_GuiList.count();
}

void My3DWindow::MouseButtonPressEnabled(bool value) { m_MouseButtonPressEnabled = value; }
bool My3DWindow::isMouseButtonPressEnabled() const { return m_MouseButtonPressEnabled; }
Qt3DCore::QEntity *My3DWindow::Scene() const { return m_Scene; }
Qt3DRender::QLayer *My3DWindow::LayerMain() const { return m_LayerMain; }
Qt3DRender::QLayer *My3DWindow::LayerGui() const { return m_LayerGui; }
