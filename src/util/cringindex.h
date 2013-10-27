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
#ifndef CRINGINDEX_H
#define CRINGINDEX_H

#include <QReadWriteLock>

/*!
 * \class CRingIndex
 * \brief The CRingIndex class provides the ring position
 *
 * The ring position is the position wich changes cyclically from min() to max().
 * Initial values for min(), max(), position() is 0.
 * All function of the class are thread-safe.
 * \fn CRingIndex::position()
 * The current position of the ring
 *
 * \fn CRingIndex::min()
 * Minimum index of the ring
 *
 * \fn CRingIndex::max()
 * Maximum index of the ring
 *
 * \fn CRingIndex::add()
 * Increment the current position in the ring
 *
 * \fn CRingIndex::setPosition()
 * Set current position in the ring
 *
 * \fn CRingIndex::setRange()
 * Set the ring range
 */

class CRingIndex
{
public:
    explicit CRingIndex();
    ~CRingIndex();
    qint64 index() const;
    qint64 min() const;
    qint64 max() const;
    void inc(qint64 value);
    qint64 indexAndInc(qint64 value);
    void fetchAndInc(qint64 *index, qint64 *min, qint64 *max, qint64 value);
    void setIndex(qint64 value);
    void setRange(qint64 min, qint64 max);
private:
    QReadWriteLock *m_readWriteLock;
    qint64 m_index, m_min, m_max;
private:
    Q_DISABLE_COPY(CRingIndex)
};

#endif // CRINGPOSITION_H
