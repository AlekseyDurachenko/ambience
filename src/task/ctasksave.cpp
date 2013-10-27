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
#include "ctasksave.h"

#include <QFile>
#include <QDebug>
#include "csndfile.h"

CTaskSave::CTaskSave(QObject *parent) :
    CAbstractTask(parent)
{
}

void CTaskSave::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

void CTaskSave::setSound(QSharedPointer<CSound> sound)
{
    m_sound = sound;
    m_format = sound->format();
    m_fileName = sound->fileName();
    m_start = 0;
    m_end = sound->frames() - 1;
}

void CTaskSave::setFormat(int format)
{
    m_format = format;
}

void CTaskSave::setRange(qint64 start, qint64 end)
{
    m_start = start;
    m_end = end;
}

void CTaskSave::task()
{
    privateWrite(m_fileName, m_sound, m_format, m_start, m_end);
}

void CTaskSave::privateWrite(const QString &fileName, QSharedPointer<CSound> sound, int format, qint64 start, qint64 end)
{
    try
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
            throw QObject::tr("can't open file for write");

        qint64 sampleCount = end - start + 1;

        CSndFile sndFile(&file);
        sndFile.setChannels(sound->channels());
        sndFile.setFormat(format);
        sndFile.setFrames(sampleCount);
        sndFile.setSampleRate(static_cast<int>(sound->sampleRate()));

        if (!sndFile.open(SFM_WRITE))
        {
            sndFile.close();
            file.close();
            QFile::remove(fileName);

            throw QObject::tr("the file format is not supported");
        }

        sndFile.setString(SF_STR_TITLE, sound->property(CSound::Title));
        sndFile.setString(SF_STR_COPYRIGHT, sound->property(CSound::Copyright));
        sndFile.setString(SF_STR_SOFTWARE, sound->property(CSound::Software));
        sndFile.setString(SF_STR_ARTIST, sound->property(CSound::Artist));
        sndFile.setString(SF_STR_COMMENT, sound->property(CSound::Comment));
        sndFile.setString(SF_STR_DATE, sound->property(CSound::Date));
        sndFile.setString(SF_STR_ALBUM, sound->property(CSound::Albom));
        sndFile.setString(SF_STR_LICENSE, sound->property(CSound::License));
        sndFile.setString(SF_STR_TRACKNUMBER, sound->property(CSound::TrackNumber));
        sndFile.setString(SF_STR_GENRE, sound->property(CSound::Genre));

        // allocate the memory
        float *tmp = 0;
        try
        {
            tmp = new float[1024*sndFile.channels()];
        }
        catch (...)
        {
            throw QObject::tr("out of memory");
        }

        // writing the data
        emit progress(0.0);
        qint64 emitFreq = sampleCount / 1024 / 20;
        if (emitFreq < 1) emitFreq = 1;
        bool write_error = false;
        for (qint64 i = start, k = 0; i <= end && !write_error; i += 1024, ++k)
        {
            qint64 wsize = 1024;
            if ((start + sampleCount - i) < 1024)
                wsize = start + sampleCount - i;

            sound->readMultiplexed(tmp, i, wsize);

            if (sndFile.writef_float(tmp, wsize) != wsize)
                write_error = true;

            if (k % emitFreq == 0)
                emit progress((100.0*i)/sampleCount);
        }
        emit progress(100.0);

        // free temporary buffer
        delete [] tmp;

        if (!write_error)
        {
            emit done();
        }
        else
        {
            sndFile.close();
            file.close();
            QFile::remove(fileName);

            emit fail(fileName, QObject::tr("there is not enough free disk space"));
        }
    }
    catch (const QString &error)
    {
        emit fail(fileName, error);
    }
}

