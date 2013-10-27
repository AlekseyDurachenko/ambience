#ifndef CPLAYERWORKER_H
#define CPLAYERWORKER_H

#include <QObject>
#include <QTimerEvent>
#include "cplayerringbuffer.h"
#include "cringindex.h"
#include "csounddatasource.h"

class CPlayerWorker : public QObject
{
    Q_OBJECT
public:
    CPlayerWorker();

    void setSound(CSoundDataSource *sound);
    void setPlayerBuffer(CPlayerRingBuffer *ringBuffer);
    void setPlayIndex(CRingIndex *index);
    void setSoundIndex(CRingIndex *index);
    void setTimerInterval(int msec);
signals:
    void started();
    void stopped();
public slots:
    void play();
    void stop();
protected:
    void timerEvent(QTimerEvent *event);
private:
    void updateBuffer();
private:
    int mTimerId, mTimerInterval;
    CSoundDataSource *mSound;
    CPlayerRingBuffer *mPlayerBuffer;
    CRingIndex *mPlayIndex, *mSoundIndex;
};

#endif // CPLAYERCONFIGURATION_H
