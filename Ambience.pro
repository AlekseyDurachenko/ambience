# Copyright 2013, Durachenko Aleksey V. <durachenko.aleksey@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# common build section
TARGET          = ambience
TEMPLATE        = app
VERSION         = 0.1
DESTDIR         = bin

CONFIG   += console debug_and_release
CONFIG   -= app_bundle
QT       += core gui

# application defines
DEFINES        +=

# build option
OBJECTS_DIR     = build/release_obj
MOC_DIR         = build/release_moc
UI_DIR          = build/release_ui
RCC_DIR         = build/release_rcc

include($$PWD/libs/qse/qse.pri)

SOURCES +=                                      \
    src/main.cpp                                \
    src/cmainwindow.cpp                         \
    src/editor/qseplotwidget.cpp                \
    src/editor/qseplotcontroller.cpp            \
    src/editor/qsewidget.cpp                    \
    src/editor/qserender.cpp                    \
    src/aio/portaudiobackend.cpp                \
    src/aio/aglobal.cpp                         \
    src/snd/csndfile.cpp                        \
    src/snd/csound.cpp                          \
    src/widget/cprogressbar.cpp                 \
    src/task/ctasksave.cpp                      \
    src/task/ctaskopen.cpp                      \
    src/task/ctaskexecutor.cpp                  \
    src/task/cabstracttask.cpp                  \
    src/util/cringindex.cpp                     \
    src/aio/asoundbackend.cpp                   \
    src/task/ctaskdelete.cpp                    \
    src/task/ctasktrim.cpp                      \
    src/task/ctasknormalize.cpp                 \
    src/editor/qselinearplot.cpp                \
    src/editor/qsescrollbar.cpp

HEADERS +=                                      \
    src/global.h                                \
    src/version.h                               \
    src/cmainwindow.h                           \
    src/editor/qseplotwidget.h                  \
    src/editor/qseplotcontroller.h              \
    src/editor/qsewidget.h                      \
    src/editor/qserender.h                      \
    src/aio/portaudiobackend.h                  \
    src/aio/aglobal.h                           \
    src/snd/csndfile.h                          \
    src/snd/csound.h                            \
    src/widget/cprogressbar.h                   \
    src/task/cabstracttask.h                    \
    src/task/ctaskexecutor.h                    \
    src/task/ctaskopen.h                        \
    src/task/ctasksave.h                        \
    src/util/cringindex.h                       \
    src/aio/asoundbackend.h                     \
    src/task/ctaskdelete.h                      \
    src/task/ctasktrim.h                        \
    src/task/ctasknormalize.h                   \
    src/editor/qselinearplot.h                  \
    src/editor/qsescrollbar.h

FORMS += \
    ui/mainwindow.ui

RESOURCES +=                                    \
    $$PWD/rc/rc.qrc

INCLUDEPATH +=                                  \
    $$PWD/src                                   \
    $$PWD/src/editor                            \
    $$PWD/src/aio                               \
    $$PWD/src/snd                               \
    $$PWD/src/task                              \
    $$PWD/src/util                              \
    $$PWD/src/widget

win32 {
    RC_FILE += $$PWD/rc/rc.rc
}

unix {
    CONFIG  += x11
}

build_pass:CONFIG(debug, debug|release) {
    #TARGET      = $$join(TARGET,,,d)
    OBJECTS_DIR = $$join(OBJECTS_DIR,,,d)
    MOC_DIR     = $$join(MOC_DIR,,,d)
    UI_DIR      = $$join(UI_DIR,,,d)
    RCC_DIR     = $$join(RCC_DIR,,,d)

    win32 {
        LIBS    +=
    }

    unix {
        LIBS    += -lsndfile -lportaudio
    }
}

build_pass:CONFIG(release, debug|release) {
    win32 {
        LIBS    +=
    }

    unix {
        LIBS    += -lsndfile -lportaudio
    }
}
