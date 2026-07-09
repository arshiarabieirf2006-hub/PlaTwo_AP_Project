QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    leaderboard.cpp \
    loginform.cpp \
    main.cpp \
    mainmenu.cpp \
    mainwindow.cpp \
    profileform.cpp \
    signupform.cpp \
    store.cpp

HEADERS += \
    leaderboard.h \
    loginform.h \
    mainmenu.h \
    mainwindow.h \
    profileform.h \
    signupform.h \
    store.h

FORMS += \
    leaderboard.ui \
    loginform.ui \
    mainmenu.ui \
    mainwindow.ui \
    profileform.ui \
    signupform.ui \
    store.ui
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
