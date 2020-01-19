#include "my3dwindow.h"

#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QOpenGLContext>

int main(int argc, char *argv[])
{
    QGuiApplication application(argc, argv);

    QSurfaceFormat surfaceFormat;
    surfaceFormat.setDepthBufferSize(24);
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        surfaceFormat.setVersion(3, 2);
        surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
    }
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    My3DWindow window;
    window.setFormat(surfaceFormat);

    window.show();
    return application.exec();
}
