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
#include "asoundbackend.h"
#include <QDebug>

ASoundBackend::ASoundBackend(QObject *parent)
    : QThread(parent)
{
    mIndexTimer = 0;
    m_indexTimerInterval = 50;
    m_sound = QSharedPointer<CSound>(0);
    m_loop = false;
    m_outputParameters.device = Pa_GetDefaultOutputDevice();
}

ASoundBackend::~ASoundBackend()
{
}

PaDeviceIndex ASoundBackend::deviceIndex() const
{
    return m_outputParameters.device;
}

void ASoundBackend::setDeviceIndex(PaDeviceIndex index)
{
    m_outputParameters.device = index;
}

bool ASoundBackend::reconfigure(QString *reason)
{
    if (!m_sound.isNull())
    {
        m_outputParameters.channelCount = m_sound->channels();
        m_outputParameters.sampleFormat = paFloat32;
        m_outputParameters.suggestedLatency = Pa_GetDeviceInfo(m_outputParameters.device)->defaultLowOutputLatency;
        m_outputParameters.hostApiSpecificStreamInfo = 0;

        bool wasPlayed = isPlayed();
        if (wasPlayed)
            stop();
        if (!PortAudio::configure(0, &m_outputParameters, m_sound->sampleRate(), 128, paClipOff, this, reason))
            return false;
        if (wasPlayed)
            play();
    }

    return true;
}

bool ASoundBackend::isPlayed() const
{
    return PortAudio::isPlayed();
}

qint64 ASoundBackend::index() const
{
    return m_playIndex.index();
}

bool ASoundBackend::setSound(QSharedPointer<CSound> sound, QString *reason)
{
    m_sound = sound;
    return reconfigure(reason);
}

void ASoundBackend::play()
{
    if (!isPlayed())
    {
        qint64 index = m_playIndex.index();
        QString reason;
        qDebug() << PortAudio::play(&reason);
        qDebug() << reason;
        mIndexTimer = startTimer(m_indexTimerInterval);
        emit started(index);
        emit indexChanged(index);
    }
}

void ASoundBackend::stop()
{
    if (isPlayed())
    {
        killTimer(mIndexTimer);
        PortAudio::stop();
        emit indexChanged(m_playIndex.index());
        emit stopped(m_playIndex.index());
    }
}

void ASoundBackend::setLoop(bool loop)
{
    m_loop = loop;
}

void ASoundBackend::setRange(qint64 start, qint64 end)
{
    m_playIndex.setRange(start, end);
}

void ASoundBackend::setIndex(qint64 index)
{
    m_playIndex.setIndex(index);
}

void ASoundBackend::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mIndexTimer)
    {
        emit indexChanged(m_playIndex.index());
        if (!isPlayed())
        {
            emit stopped(m_playIndex.index());
            killTimer(mIndexTimer);
        }
    }
}

void ASoundBackend::run()
{
}


int ASoundBackend::cbPlay(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags)
{
    // this variables currently not used, but may be used in the future
    // mark as unused it for escape compiler warnings
    Q_UNUSED(inputBuffer);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);

    float *floatOutput = (float *)outputBuffer;

    qint64 index, min, max;
    m_playIndex.fetchAndInc(&index, &min, &max, framesPerBuffer);
    if (static_cast<qint64>(framesPerBuffer) <= (max-index+1))
    {
        m_sound->readMultiplexed((float *)outputBuffer, index, framesPerBuffer);
    }
    else
    {
        if (m_loop == false)
        {
            m_sound->readMultiplexed(floatOutput, index, max-index+1);
            memset(&floatOutput[max-index+1], 0,
                    (framesPerBuffer-(max-index+1)) * sizeof(float) * m_sound->channels());
            return paComplete;
        }
        else
        {
            m_sound->readMultiplexed(floatOutput, index, max-index+1);
            qint64 offset = max-index+1;
            qint64 sampleForBuffer = framesPerBuffer - (max-index+1);
            while (sampleForBuffer > 0)
            {
                qint64 count = sampleForBuffer;
                if (sampleForBuffer > max-min+1)
                    count = max-min+1;
                m_sound->readMultiplexed(&floatOutput[offset], offset, count);
                sampleForBuffer -= count;
                offset += count;
            }
        }
    }

    return paContinue;
}

void ASoundBackend::cbFinish()
{
}
