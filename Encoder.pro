QT += core xml
QT -= gui

TARGET = Encoder
CONFIG += console
CONFIG -= app_bundle
chip = HI3531D
include(../../LinkLib/Link.pri)
include(../libmaia-master/maia.pri)

TEMPLATE = app

SOURCES += main.cpp \
    RPC.cpp \
    Group.cpp \
    Channel.cpp \
    ChannelNet.cpp \
    ChannelVI.cpp \
    Config.cpp \
    GroupRPC.cpp \
    ChannelMix.cpp \
    ChannelUSB.cpp \
    ChannelFile.cpp

HEADERS += \
    RPC.h \
    Group.h \
    Channel.h \
    ChannelNet.h \
    ChannelVI.h \
    Config.h \
    GroupRPC.h \
    ChannelMix.h \
    ChannelUSB.h \
    ChannelFile.h

