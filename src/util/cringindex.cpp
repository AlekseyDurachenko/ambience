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
#include "cringindex.h"

CRingIndex::CRingIndex()
{
    m_readWriteLock = new QReadWriteLock(QReadWriteLock::Recursive);
    m_index = m_max = m_min = 0;
}

CRingIndex::~CRingIndex()
{
    delete m_readWriteLock;
}

qint64 CRingIndex::index() const
{
    QReadLocker locker(m_readWriteLock);
    return m_index;
}

qint64 CRingIndex::min() const
{
    QReadLocker locker(m_readWriteLock);
    return m_min;
}

qint64 CRingIndex::max() const
{
    QReadLocker locker(m_readWriteLock);
    return m_max;
}

void CRingIndex::inc(qint64 value)
{
    QWriteLocker locker(m_readWriteLock);
    m_index += value % (m_max - m_min + 1);
    if (m_index > m_max)
        m_index = m_min + (m_index - m_max);
}

qint64 CRingIndex::indexAndInc(qint64 value)
{
    QWriteLocker locker(m_readWriteLock);
    qint64 currentIndex = m_index;
    m_index += value % (m_max - m_min + 1);
    if (m_index > m_max)
        m_index = m_min + (m_index - m_max);
    return currentIndex;
}

void CRingIndex::fetchAndInc(qint64 *index, qint64 *min, qint64 *max, qint64 value)
{
    QWriteLocker locker(m_readWriteLock);

    if (index)
        *index = m_index;
    if (min)
        *min = m_min;
    if (max)
        *max = m_max;

    m_index += value % (m_max - m_min + 1);
    if (m_index > m_max)
        m_index = m_min + (m_index - m_max);
}

void CRingIndex::setIndex(qint64 value)
{
    QWriteLocker locker(m_readWriteLock);
    m_index = value;
    if (m_index > m_max)
        m_index = m_max;
    else if (m_index < m_min)
        m_index = m_min;
}

void CRingIndex::setRange(qint64 min, qint64 max)
{
    QWriteLocker locker(m_readWriteLock);
    m_max = max;
    m_min = min;
    if (m_max < m_min)
        m_max = m_min;
}
