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
#ifndef CTASKOPEN_H
#define CTASKOPEN_H

#include <QSharedPointer>
#include "cabstracttask.h"
#include "csound.h"

class CTaskOpen : public CAbstractTask
{
    Q_OBJECT
public:
    explicit CTaskOpen(QObject *parent = 0);

    void setFileName(const QString &fileName);
    void task();
signals:
    void progress(double percent);
    void fail(const QString &fileName, const QString &message);
    void done(QSharedPointer<CSound> sound);
private:
    void privateRead(const QString &fileName);
private:
    QString m_fileName;
};

#endif // CTASKOPEN_H
