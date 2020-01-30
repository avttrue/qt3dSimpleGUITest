#ifndef GUIENTITY_H
#define GUIENTITY_H

#include <QFont>
#include <QColor>
#include <QVector2D>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QCuboidMesh>

const QColor FONT_COLOR = Qt::white;
const QColor PANEL_COLOR = Qt::darkGray;
const QFont FONT = QFont("monospace", 30, QFont::Normal);
const float PANEL_POSZ = -0.001f;
const float TEXT_POSZ = 0.0f;
const int BUTTON_ANIM_TIME = 100;
const float BUTTON_ANIM_INDENT = 2.0f;

/*!
 * \brief SizePosFactor enum - способ задания позиции и размеров:
 * Absolute - в абсолютных величинах;
 * Relative - в сотых долях от высоты и ширины окна.
 */
enum SizePosFactor: int
{
    Absolute = 0,
    Relative
};

class EntityGui : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    EntityGui(Qt3DCore::QEntity *parent);
    QRectF Rect() const;

protected:
    QWindow* m_Window;
    Qt3DCore::QTransform* m_Transform;
    QRectF m_Rect;
    QSizeF m_Size;
    SizePosFactor m_SizePosFactor;
    QVector2D m_Position;
};

class Entity3DText : public EntityGui
{
    Q_OBJECT
public:
    Entity3DText(Qt3DCore::QEntity *parent,
                 const QSizeF& size,
                 SizePosFactor sizePosFactor = SizePosFactor::Absolute,
                 const QFont& font = FONT);
    void setPosition(QVector2D value);
    bool isInteractive() const;
    void Interactive(bool value);  

private:
    Qt3DExtras::QDiffuseSpecularMaterial* m_Material;
    Qt3DExtras::QExtrudedTextMesh* m_Mesh;
    QFont m_Font;
    float m_DefaultWidth;
    float m_DefaultHeight;
    int m_LoadingStatus;
    bool m_Interactive;
    bool m_Animated;

    void animationClick();

public Q_SLOTS:
    void slotWrite(const QString& text, const QColor &color = FONT_COLOR);
    void slotClicked();

protected Q_SLOTS:
    void slotResize();

Q_SIGNALS:
    void signalClicked();

};

class EntityPanel : public EntityGui
{
    Q_OBJECT
public:
    EntityPanel(Qt3DCore::QEntity *parent,
                const QSizeF& size,
                const QColor &color = PANEL_COLOR,
                SizePosFactor sizePosFactor = SizePosFactor::Absolute);
    void setPosition(QVector2D value);

private:
    Qt3DExtras::QCuboidMesh* m_Mesh;

protected Q_SLOTS:
    void slotResize();

};

#endif // GUIENTITY_H
