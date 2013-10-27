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
#ifndef ASOUNDBACKEND_H
#define ASOUNDBACKEND_H

#include <QSharedPointer>
#include <QAtomicInt>
#include <portaudio.h>
#include <stdio.h>
#include "cringindex.h"
#include "csound.h"
#include "portaudiobackend.h"
#include <QThread>
#include <QTimerEvent>

/*! \brief The ASoundBackend class provides function for the play sound/recod sound throw portaudio
 */
class ASoundBackend : public QThread, public PortAudioBackend
{
Q_OBJECT
public:
    ASoundBackend(QObject *parent = 0);
    ~ASoundBackend();

    PaDeviceIndex deviceIndex() const;
    void setDeviceIndex(PaDeviceIndex index);
    bool reconfigure(QString *reason = 0);
    bool setSound(QSharedPointer<CSound> sound, QString *reason = 0);

    qint64 index() const;
    bool isPlayed() const;

    void play();
    void stop();
    void setLoop(bool loop);
    void setRange(qint64 start, qint64 end);
    void setIndex(qint64 index);
signals:
    void indexChanged(qint64 playIndex);
    void started(qint64 playIndex);
    void stopped(qint64 playIndex);
protected:
    void timerEvent(QTimerEvent *event);
private:
    void run();
private:
    // this function called from the port-audio callback. We should notice, the all
    // data will be used inside the function function must be thread-safe
    int cbPlay(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    void cbFinish();
private:
    PaStreamParameters m_outputParameters;
    int mIndexTimer, m_indexTimerInterval;
    QSharedPointer<CSound> m_sound;
    CRingIndex m_playIndex;
    QAtomicInt m_loop;
};

#endif // ASOUNDBACKEND_H
