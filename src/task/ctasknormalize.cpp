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
#include "ctasknormalize.h"

CTaskNormalize::CTaskNormalize(QObject *parent) :
    CAbstractTask(parent)
{
}

void CTaskNormalize::setSound(QSharedPointer<CSound> sound)
{
    m_sound = sound;
    m_normValue = 1.0;
    m_start = -1;
    m_end = -1;
}

void CTaskNormalize::setRange(qint64 start, qint64 end)
{
    m_start = start;
    m_end = end;
}

void CTaskNormalize::setNormValue(double value)
{
    m_normValue = value;
}

void CTaskNormalize::task()
{
    privateNormalize(m_sound, m_start, m_end, m_normValue);
}

void CTaskNormalize::privateNormalize(QSharedPointer<CSound> sound, qint64 start, qint64 end, double normValue)
{
    try
    {
        emit progress(0.0);
        sound->lockForWrite();
        sound->normalize(start, end, normValue);
        sound->unlockForWrite(true);
        emit progress(100.0);
        emit done();
    }
    catch (const QString &error)
    {
        emit fail(error);
    }
}
