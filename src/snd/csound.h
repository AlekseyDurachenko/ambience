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
#ifndef CSOUND_H
#define CSOUND_H

#include <QReadWriteLock>
#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QHash>


class CSound : public QObject
{
    Q_OBJECT
public:
    enum Id { Title, Copyright, Software, Artist, Comment, Date, Albom, License, TrackNumber, Genre };

    explicit CSound(QObject *parent);
    CSound(float **data, int channels, qint64 frames, double sampleRate, QObject *parent = 0);
    ~CSound();

    // this methods should not be called from non-gui thread
    int format() const;
    QString property(Id id) const;
    const QString &fileName() const;
    void setFormat(int format);
    void setProperty(Id id, const QString &value);
    void setFileName(const QString &fileName);

    // this methods should be called from the any thread
    // and them MUST be called with blocking mechanism
    double sampleRate() const;
    qint64 frames() const;
    int channels() const;
    void setSampleRate(double sampleRate);
    void setFrames(qint64 frames);
    void setChannels(int channels);
    // raw data manipulation methods
    void remove(qint64 left, qint64 right);
    void trim(qint64 left, qint64 right);
    void normalize(qint64 left, qint64 right, double normValue);
    void read(float *ptr, int channel, qint64 offset, qint64 count);
    void write(float *ptr, int channel, qint64 offset, qint64 count);
    void readMultiplexed(float *ptr, qint64 offset, qint64 count);
    void readToList(QList<double> *list, int channel, qint64 offset, qint64 count);
    void readPeakToList(QList<double> *minimums, QList<double> *maximums, int channel, qint64 offset,
            qint64 samplePerPeak, qint64 count, volatile bool *abort = 0);

    // the blocing mechanism
    // for the write operation you should use lockForWrite() and unlockForWrite(), for read - another pair
    void lockForWrite();
    void lockForRead();
    void unlockForRead();
    void unlockForWrite(bool changes = true);
signals:
    void locked();
    void unlocked(bool changes = false);
    void changed(int channels, qint64 frames, double sampleRate);
private:
    // sound metadata
    QHash<Id, QString> m_property;
    QString m_fileName;
    int m_format;
    // sound data
    double m_sampleRate;
    qint64 m_frames;
    float **m_data;
    int m_channels;
    // variable for the blocking
    QReadWriteLock *m_lock;
};


#endif // CSOUND_H
