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
#ifndef CTASKNORMALIZE_H
#define CTASKNORMALIZE_H

#include <QSharedPointer>
#include "cabstracttask.h"
#include "csound.h"

class CTaskNormalize : public CAbstractTask
{
    Q_OBJECT
public:
    explicit CTaskNormalize(QObject *parent = 0);

    void setSound(QSharedPointer<CSound> sound);
    void setRange(qint64 start, qint64 end);
    void setNormValue(double value);
    void task();
signals:
    void progress(double percent);
    void fail(const QString &message);
    void done();
private slots:
    void privateNormalize(QSharedPointer<CSound> sound, qint64 start, qint64 end, double value);
private:
    QSharedPointer<CSound> m_sound;
    double m_normValue;
    qint64 m_start, m_end;

};

#endif // CTASKNORMALIZE_H
