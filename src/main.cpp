// Copyright 2013, Durachenko Aleksey V. <durachenko.aleksey@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "cmainwindow.h"

void metatypeInit()
{
    qRegisterMetaType<QSharedPointer<CSound> >("QSharedPointer<CSound>");
    qRegisterMetaType<QList<double> >("QList<double>");
    qRegisterMetaType<CTaskExecutor*>("CTaskExecutor*");
    qRegisterMetaType<QseGeometry>("QseGeometry");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // register datatypes for signal/slot
    metatypeInit();

    // init the sound system
    PortAudio::init();
    g_soundPlayer = new ASoundBackend;

    CMainWindow window;
    window.show();

    QObject::connect(&window, SIGNAL(destroyed()), g_soundPlayer, SLOT(deleteLater()));

    // open the argument as the sound
    if (qApp->arguments().count() >= 2)
        QMetaObject::invokeMethod(&window, "slot_Open", Qt::QueuedConnection, Q_ARG(QString, qApp->arguments().at(1)));

    return app.exec();
}
