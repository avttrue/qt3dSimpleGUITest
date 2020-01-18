QT       += core 3dlogic 3dextras 3dinput 3danimation

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    guientity.cpp \
    main.cpp \
    my3dwindow.cpp

HEADERS += \
    guientity.h \
    my3dwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md
