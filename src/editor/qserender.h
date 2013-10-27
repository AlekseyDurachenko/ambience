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
#ifndef QSERENDER_H
#define QSERENDER_H

#include <QObject>
#include <QRect>
#include <QSharedPointer>
#include "csound.h"
#include "qsegeometry.h"

class QseRender : public QObject
{
    Q_OBJECT
public:
    explicit QseRender(QObject *parent = 0);
public slots:
    void readPeaks(qint64 id, QSharedPointer<CSound> sound, int channel,
        const QseGeometry &geometry, const QRect &rect);
    void waitForId(int id);
signals:
    void peaksReaded(qint64 id, QSharedPointer<CSound> sound, int channel, const QseGeometry &geometry,
        QList<double> minimums, QList<double> maximums);
private:
    volatile bool m_stop;
    QAtomicInt m_waitedId;
};


#endif // QSERENDER_H
