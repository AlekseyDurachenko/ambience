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
#include "csound.h"
#include <sndfile.h>
#include <QDebug>


CSound::CSound(QObject *parent)
    : QObject(parent)
{
    m_data = 0;
    m_channels = 0;
    m_frames = 0;
    m_sampleRate = 0.0;
    m_fileName = QString();
    m_format = 0;
    m_lock = new QReadWriteLock(QReadWriteLock::Recursive);
}

CSound::CSound(float **data, int channels, qint64 sampleCount, double sampleRate, QObject *parent)
    : QObject(parent)
{
    m_data = data;
    m_channels = channels;
    m_frames = sampleCount;
    m_sampleRate = sampleRate;
    m_fileName = QString();
    m_format = 0;
    m_lock = new QReadWriteLock(QReadWriteLock::Recursive);
}

CSound::~CSound()
{
    for (int i = 0; i < m_channels; ++i)
        delete [] m_data[i];
    delete [] m_data;
    delete m_lock;
}

QString CSound::property(Id id) const
{
    return m_property.contains(id) ?  m_property[id] : QString();
}

void CSound::setProperty(CSound::Id id, const QString &value)
{
    m_property[id] = value;
}

const QString &CSound::fileName() const
{
    return m_fileName;
}

void CSound::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

double CSound::sampleRate() const
{
    return m_sampleRate;
}

void CSound::setSampleRate(double sampleRate)
{
    m_sampleRate = sampleRate;
}

qint64 CSound::frames() const
{
    return m_frames;
}

void CSound::setFrames(qint64 sampleCount)
{
    if (m_frames == sampleCount)
        return;

    for (int i = 0; i < m_channels; ++i)
    {
        float *newData = new float[m_frames];
        if (sampleCount > m_frames)
        {
            memcpy(newData, m_data[i], m_frames*sizeof(float));
            memset(&newData[m_frames], 0, (sampleCount-m_frames)*(sizeof(float)));
        }
        else
        {
            memcpy(newData, m_data[i], sampleCount*sizeof(float));
        }
        delete [] m_data[i];
        m_data[i] = newData;
    }

    m_frames = sampleCount;
}

int CSound::channels() const
{
    return m_channels;
}

void CSound::setChannels(int channelCount)
{
    if (channelCount == m_channels)
        return;

    float **newData = new float*[channelCount];
    for (int i = 0; i < channelCount && i < m_channels; ++i)
        newData[i] = m_data[i];

    if (channelCount < m_channels)
    {
        for (int i = channelCount; i < m_channels; ++i)
            delete [] m_data[i];
    }
    else
    {
        for (int i = m_channels; i < channelCount; ++i)
            newData[i] = new float[m_frames];
    }

    delete [] m_data;
    m_data = newData;

    m_channels = channelCount;
}

void CSound::remove(qint64 left, qint64 right)
{
    for (int channel = 0; channel < m_channels; ++channel)
    {
        float *to = &m_data[channel][left];
        float *from = &m_data[channel][right+1];
        qint64 count = (m_frames - right - 1) * sizeof(float);
        memmove(to, from, count);
        m_frames -= right - left + 1;
    }
}

void CSound::trim(qint64 left, qint64 right)
{
    for (int channel = 0; channel < m_channels; ++channel)
    {
        float *to = &m_data[channel][0];
        float *from = &m_data[channel][left];
        qint64 count = (right - left + 1) * sizeof(float);
        memmove(to, from, count);
        m_frames = right - left + 1;
    }
}

void CSound::normalize(qint64 left, qint64 right, double normValue)
{
    float maximum = 0.0;

    for (int channel = 0; channel < m_channels; ++channel)
    {
        for (qint64 i = left; i <= right; ++i)
            if (qAbs(m_data[channel][i]) > maximum)
                maximum = qAbs(m_data[channel][i]);
    }

    for (int channel = 0; channel < m_channels; ++channel)
    {
        float k = normValue / maximum;
        for (qint64 i = left; i <= right; ++i)
            m_data[channel][i] *= k;
    }
}

int CSound::format() const
{
    return m_format;
}

void CSound::setFormat(int format)
{
    m_format = format;
}

void CSound::read(float *ptr, int channel, qint64 offset, qint64 count)
{
    memcpy(ptr, &m_data[channel][offset], count*sizeof(float));
}

void CSound::write(float *ptr, int channel, qint64 offset, qint64 count)
{
    memcpy(&m_data[channel][offset], ptr, count*sizeof(float));
}

void CSound::readMultiplexed(float *ptr, qint64 offset, qint64 count)
{
    for (qint64 i = 0; i < count; ++i)
        for (int ch = 0; ch < m_channels; ++ch)
            *ptr++ = m_data[ch][offset+i];
}

void CSound::readToList(QList<double> *list, int channel, qint64 offset, qint64 count)
{
    count = offset + count;
    for (qint64 i = offset; i < count && i < m_frames; ++i)
        list->push_back(m_data[channel][i]);
}

void CSound::readPeakToList(QList<double> *minimums, QList<double> *maximums, int channel, qint64 offset,
    qint64 samplePerPeak, qint64 count, volatile bool *abort)
{
    count = offset + count;
    for (qint64 i = offset; i < count && i < m_frames; i += samplePerPeak)
    {
        if (abort && *abort)
            break;

        qint64 s = i;
        qint64 e = i+samplePerPeak;
        if (e > count)
            e = count;
        if (e > m_frames)
            e = m_frames;
        float min, max;
        min = max = m_data[channel][s];
        for (qint64 j = s+1; j < e; ++j)
        {
            float curdata = m_data[channel][j];
            if (curdata > max)
                max = curdata;
            else if (curdata < min)
                min = curdata;
        }
        maximums->push_back(max);
        minimums->push_back(min);
    }
}

void CSound::lockForWrite()
{
    emit locked();
    m_lock->lockForWrite();
}

void CSound::lockForRead()
{
    m_lock->lockForRead();
}

void CSound::unlockForRead()
{
    m_lock->unlock();
}

void CSound::unlockForWrite(bool changes)
{
    m_lock->unlock();
    if (changes)
        emit changed(channels(), frames(), sampleRate());
    emit unlocked(changes);
}
