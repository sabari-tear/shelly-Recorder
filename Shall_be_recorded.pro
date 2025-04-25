QT += quick multimedia core widgets svg

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        screenrecorder.cpp

RESOURCES += resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .gitignore \
    main.qml \
    selectarea.qml

HEADERS += \
    mainwindow.h \
    screenrecorder.h

# Ensure QML files are included in the build
OTHER_FILES += \
    main.qml \
    selectarea.qml

win32:LIBS += -lpthread -lole32 -loleaut32

win32: LIBS += -L$$PWD/lib/ -lavcodec  -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

unix:LIBS += -lavformat -lavcodec -lavutil -lavdevice -lm -lswscale -lX11 -lpthread -lswresample -lasound


win32:INCLUDEPATH += $$PWD/include
win32:DEPENDPATH += $$PWD/include
