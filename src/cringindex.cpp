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
    mReadWriteLock = new QReadWriteLock(QReadWriteLock::Recursive);
    mIndex = mMax = mMin = 0;
}

CRingIndex::~CRingIndex()
{
    delete mReadWriteLock;
}

qint64 CRingIndex::index() const
{
    QReadLocker locker(mReadWriteLock);
    return mIndex;
}

qint64 CRingIndex::min() const
{
    QReadLocker locker(mReadWriteLock);
    return mMin;
}

qint64 CRingIndex::max() const
{
    QReadLocker locker(mReadWriteLock);
    return mMax;
}

void CRingIndex::inc(qint64 value)
{
    QWriteLocker locker(mReadWriteLock);
    mIndex += value % (mMax - mMin + 1);
    if (mIndex > mMax)
        mIndex = mMin + (mIndex - mMax);
}

qint64 CRingIndex::indexAndInc(qint64 value)
{
    QWriteLocker locker(mReadWriteLock);
    qint64 currentIndex = mIndex;
    mIndex += value % (mMax - mMin + 1);
    if (mIndex > mMax)
        mIndex = mMin + (mIndex - mMax);
    return currentIndex;
}

void CRingIndex::fetchAndInc(qint64 *index, qint64 *min, qint64 *max, qint64 value)
{
    QWriteLocker locker(mReadWriteLock);

    if (index)
        *index = mIndex;
    if (min)
        *min = mMin;
    if (max)
        *max = mMax;

    mIndex += value % (mMax - mMin + 1);
    if (mIndex > mMax)
        mIndex = mMin + (mIndex - mMax);
}

void CRingIndex::setIndex(qint64 value)
{
    QWriteLocker locker(mReadWriteLock);
    mIndex = value;
    if (mIndex > mMax)
        mIndex = mMax;
    else if (mIndex < mMin)
        mIndex = mMin;
}

void CRingIndex::setRange(qint64 min, qint64 max)
{
    QWriteLocker locker(mReadWriteLock);
    mMax = max;
    mMin = min;
    if (mMax < mMin)
        mMax = mMin;
}
