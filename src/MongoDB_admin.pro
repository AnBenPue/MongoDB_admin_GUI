#-------------------------------------------------
#
# Project created by QtCreator 2019-10-25T09:37:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MongoDB_admin
TEMPLATE = app

DESTDIR = ../bin

# Installation files:
icons.path = $$DESTDIR/icons
icons.files = icons/*
INSTALLS += icons

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
    -I /usr/local/include/mongocxx/v_noabi/mongocxx \
    -I /usr/local/include/bsoncxx/v_noabi/bsoncxx\
    -I /usr/local/include/libbson-1.0/bson \
    -I /usr/local/include/libmongoc-1.0 \
    -I /usr/local/lib

LIBS += -L /usr/local/lib -lmongocxx -lbsoncxx

DEFINES += SOURCE_PATH=\\\"$$PWD\\\"

SOURCES += \
        $$PWD/main.cpp \
        $$PWD/mongodb_manager.cpp \
        $$PWD/mongodb_table_model.cpp \
        $$PWD/mongodb_table_view.cpp \
        $$PWD/mongodb_table_roles_delegate.cpp \
        $$PWD/mongodb_logger.cpp \
        $$PWD/mongodb_document.cpp \
        $$PWD/mongodb_gui_admin.cpp \
        $$PWD/mongodb_gui_credentials_dialog.cpp \
        $$PWD/mongodb_gui_document.cpp

HEADERS += \
        $$PWD/mongodb_manager.h \
        $$PWD/mongodb_table_model.h \
        $$PWD/mongodb_table_view.h \
        $$PWD/mongodb_table_roles_delegate.h \
        $$PWD/mongodb_logger.h \
        $$PWD/mongodb_document.h \
        $$PWD/mongodb_structures.h \
        $$PWD/mongodb_gui_admin.h \
        $$PWD/mongodb_gui_credentials_dialog.h \
        $$PWD/mongodb_gui_document.h

FORMS += \
        $$PWD/mainwindow.ui \
        $$PWD/mongodb_credentials_dialog.ui \
        $$PWD/mongodb_gui_document.ui
