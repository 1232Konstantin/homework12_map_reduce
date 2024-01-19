TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

# бустовский скомпилированная либа контекст
INCLUDEPATH = D:\boost_1_83_0\boost_1_83_0
#LIBS += "D:\boost_1_70_0\boost_1_70_0\boost_mingw730_32\lib\libboost_program_options-mgw49-mt-x32-1_70.a"


LIBS += "D:\boost_1_83_0\boost_1_83_0\stage\lib\libboost_program_options-mgw7-mt-x32-1_83.a"
LIBS += "D:\boost_1_83_0\boost_1_83_0\stage\lib\libboost_filesystem-mgw7-mt-x32-1_83.a"
LIBS += "D:\boost_1_83_0\boost_1_83_0\stage\lib\libboost_nowide-mgw7-mt-x32-1_83.a"

SOURCES += \
        _map.cpp \
        _reduce.cpp \
        _shuffle.cpp \
        input.cpp \
        main.cpp

HEADERS += \
    _map.h \
    _reduce.h \
    _shuffle.h \
    input.h
