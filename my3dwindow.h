#ifndef MY3DWINDOW_H
#define MY3DWINDOW_H

#include <QVector2D>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QLayer>

class Entity3DText;
class EntityGui;
enum SizePosFactor: int;

class My3DWindow : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT
public:
    My3DWindow(QScreen *screen = nullptr);

    Qt3DRender::QLayer *LayerMain() const;
    Qt3DRender::QLayer *LayerGui() const;
    Qt3DCore::QEntity *Scene() const;
    void createScene();
    Entity3DText* creatTextGUI(const QString& text,
                               const QSizeF& size,
                               SizePosFactor spFactor,
                               const QColor& color = QColor(Qt::white),
                               const QVector2D &position = QVector2D(0.0f, 0.0f));
    EntityGui* createPane(const QSizeF& size,
                          SizePosFactor spFactor,
                          const QColor& color = QColor(Qt::white),
                          const QVector2D &position = QVector2D(0.0f, 0.0f));

    bool isMouseButtonPressEnabled() const;
    void MouseButtonPressEnabled(bool value);
    void addToGuiList(Qt3DCore::QEntity* entity);
    void removeFromGuiList(Qt3DCore::QEntity* entity);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void createFramegraph();
    void setGuiCameraProjection();
    void setMainCameraProjection(int width, int height);
    void Test1();

    Entity3DText* captionText = nullptr; // test
    Qt3DCore::QTransform* testTransform; // test
    Qt3DCore::QEntity *m_Scene;
    Qt3DRender::QCamera *m_CameraMain;
    Qt3DRender::QCamera *m_CameraGui;
    Qt3DRender::QLayer *m_LayerMain;
    Qt3DRender::QLayer *m_LayerGui;
    QVector<Qt3DCore::QEntity*> m_GuiList;
    bool m_MouseButtonPressEnabled;    
};

#endif // MY3DWINDOW_H
