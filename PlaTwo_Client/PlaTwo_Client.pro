QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gameform.cpp \
    leaderboard.cpp \
    loginform.cpp \
    main.cpp \
    mainmenu.cpp \
    mainwindow.cpp \
    morrisgameform.cpp \
    morrisnodeitem.cpp \
    profileform.cpp \
    signupform.cpp \
    store.cpp \
    lineitem.cpp

HEADERS += \
    gameform.h \
    leaderboard.h \
    loginform.h \
    mainmenu.h \
    mainwindow.h \
    morrisgameform.h \
    morrisnodeitem.h \
    profileform.h \
    signupform.h \
    store.h \
    lineitem.h

FORMS += \
    gameform.ui \
    leaderboard.ui \
    loginform.ui \
    mainmenu.ui \
    mainwindow.ui \
    morrisgameform.ui \
    profileform.ui \
    signupform.ui \
    store.ui
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
