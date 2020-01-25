#ifndef GUIENTITY_H
#define GUIENTITY_H

#include <QFont>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include <Qt3DExtras/QExtrudedTextMesh>

const QColor FONT_COLOR = Qt::white;
const QFont FONT = QFont("monospace", 30, QFont::Bold);
const float PANEL_POS = 0.01f;
const float TEXT_POS = 0.0f;
const qreal BUTTON_HINDENT = 0.2;
const qreal BUTTON_WINDENT = 0.1;

class EntityTransform : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    EntityTransform(Qt3DCore::QEntity *parent);
    Qt3DCore::QTransform *Transform() const;
    QRectF Rect() const;
    bool isInteractive() const;
    void Interactive(bool value);

protected:
    Qt3DCore::QTransform* m_Transform;
    QSizeF m_Size;
    QRectF m_Rect;
    float m_DefaultWidth;
    float m_DefaultHeight;
    bool m_Interactive;
};

class Entity3DText : public EntityTransform
{
    Q_OBJECT

public:
    Entity3DText(Qt3DCore::QEntity *parent, const QSizeF& size, const QFont& font = FONT);

public Q_SLOTS:
    void slotWrite(const QString& text, const QColor &color = FONT_COLOR);
    void slotClicked();

protected:
    void resize();

private:
    Qt3DExtras::QDiffuseSpecularMaterial* m_Material;
    Qt3DExtras::QExtrudedTextMesh* m_Mesh;
    QFont m_Font;
    int m_LoadingStatus;

Q_SIGNALS:
        void signalWrited();

};

class EntityButton : public Entity3DText
{
    Q_OBJECT

public:
    EntityButton(Qt3DCore::QEntity *parent,
                 const QSizeF& size,
                 const QColor &color,
                 const QFont& font = FONT);
};

#endif // GUIENTITY_H
