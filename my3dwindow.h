#ifndef MY3DWINDOW_H
#define MY3DWINDOW_H

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QLayer>

class Entity3DText;

class My3DWindow : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT
public:
    My3DWindow(QScreen *screen = nullptr);

    Qt3DRender::QLayer *LayerMain() const;
    Qt3DRender::QLayer *LayerGui() const;
    Qt3DCore::QEntity *Scene() const;
    void createScene();
    Entity3DText* creatTextEntity(const QString &text,
                                  const QSizeF &size,
                                  const QColor &color);

    bool isMouseButtonPressEnabled() const;
    void MouseButtonPressEnabled(bool value);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void createFramegraph();
    void setGuiCameraProjection();
    void setMainCameraProjection(int width, int height);
    void Test1();
    void Test2();

    Qt3DCore::QTransform* testTransform; // delete
    Qt3DCore::QEntity *m_Scene;
    Qt3DRender::QCamera *m_CameraMain;
    Qt3DRender::QCamera *m_CameraGui;
    Qt3DRender::QLayer *m_LayerMain;
    Qt3DRender::QLayer *m_LayerGui;
    bool m_MouseButtonPressEnabled;
};

#endif // MY3DWINDOW_H
