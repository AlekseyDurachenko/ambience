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
#include "ctasktrim.h"

CTaskTrim::CTaskTrim(QObject *parent)
    : CAbstractTask(parent)
{
}

void CTaskTrim::setSound(QSharedPointer<CSound> sound)
{
    m_sound = sound;
    m_start = -1;
    m_end = -1;
}

void CTaskTrim::setRange(qint64 start, qint64 end)
{
    m_start = start;
    m_end = end;
}

void CTaskTrim::task()
{
    privateWrite(m_sound, m_start, m_end);
}

void CTaskTrim::privateWrite(QSharedPointer<CSound> sound, qint64 start, qint64 end)
{
    try
    {
        emit progress(0.0);
        sound->lockForWrite();
        sound->trim(start, end);
        sound->unlockForWrite(true);
        emit progress(100.0);
        emit done();
    }
    catch (const QString &error)
    {
        emit fail(error);
    }
}
