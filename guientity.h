#ifndef GUIENTITY_H
#define GUIENTITY_H

#include <QFont>
#include <QVector2D>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QExtrudedTextMesh>

const QColor FONT_COLOR = Qt::white;
const QFont FONT = QFont("monospace", 30, QFont::Normal);
const float PANEL_POSY = 0.01f;
const float TEXT_POSY = 0.0f;
const int BUTTON_ANIM_TIME = 120;
const float BUTTON_ANIM_INDENT = 1.5f;

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
    void Position(QVector2D value);
    void Position(QVector3D value);

protected:
    QWindow* m_Window;
    Qt3DCore::QTransform* m_Transform;
    QRectF m_Rect;
    QSizeF m_Size;
    SizePosFactor m_SizePosFactor;
    QVector2D m_Position;
    float m_DefaultWidth;
    float m_DefaultHeight;

protected Q_SLOTS:
    void slotResize();
};

class Entity3DText : public EntityGui
{
    Q_OBJECT
public:
    Entity3DText(Qt3DCore::QEntity *parent,
                 const QSizeF& size,
                 SizePosFactor sizeFactor = SizePosFactor::Absolute,
                 const QFont& font = FONT);
    bool isInteractive() const;
    void Interactive(bool value);  

private:
    Qt3DExtras::QDiffuseSpecularMaterial* m_Material;
    Qt3DExtras::QExtrudedTextMesh* m_Mesh;
    QFont m_Font;
    int m_LoadingStatus;
    bool m_Interactive;
    bool m_Animated;

    void animationClick();

public Q_SLOTS:
    void slotWrite(const QString& text, const QColor &color = FONT_COLOR);
    void slotClicked();

Q_SIGNALS:
    void signalClicked();

};

#endif // GUIENTITY_H
