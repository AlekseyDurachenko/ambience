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
#include "qserender.h"


QseRender::QseRender(QObject *parent) :
    QObject(parent)
{
    m_stop = false;
}

void QseRender::readPeaks(qint64 id, QSharedPointer<CSound> sound, int channel,
    const QseGeometry &geometry, const QRect &rect)
{
    if (m_stop && id != m_waitedId)
        return;

    if (m_stop && id == m_waitedId)
        m_stop = false;

    QList<double> minimums, maximums;
    sound->lockForRead();
    if (geometry.samplePerPixel() > 0)
    {
        qint64 offset = geometry.x() * geometry.samplePerPixel();
        qint64 count = rect.width() * geometry.samplePerPixel();
        sound->readPeakToList(&minimums, &maximums, channel, offset, geometry.samplePerPixel(), count, &m_stop);
    }
    else
    {
        qint64 count = rect.width() / qAbs(geometry.samplePerPixel()) + 1
                + ((rect.width()%qAbs(geometry.samplePerPixel())) ? 1 : 0);
        sound->readToList(&minimums, channel, geometry.x(), count);
    }
    sound->unlockForRead();

    if (m_stop && id != m_waitedId)
        return;

    emit peaksReaded(id, sound, channel, geometry, minimums, maximums);
}

void QseRender::waitForId(int id)
{
    m_stop = true;
    m_waitedId = id;
}
