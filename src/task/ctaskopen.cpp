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
#include "ctaskopen.h"

#include <QFile>
#include <QDebug>
#include "csndfile.h"

CTaskOpen::CTaskOpen(QObject *parent)
    : CAbstractTask(parent)
{    
}

void CTaskOpen::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

void CTaskOpen::task()
{
    privateRead(m_fileName);
}

void CTaskOpen::privateRead(const QString &fileName)
{
    try
    {
        QSharedPointer<CSound> sound = QSharedPointer<CSound>(0);
        if (!QFile::exists(fileName))
            throw QObject::tr("file not found");

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
            throw QObject::tr("can't open file for read");

        CSndFile sndFile(&file);
        if (!sndFile.open(SFM_READ))
            throw QObject::tr("the file format is not supported");

        // allocate the memory
        float **data = 0, *tmp = 0;
        try
        {
            data = new float*[sndFile.channels()];
            for (int i = 0; i < sndFile.channels(); ++i)
                data[i] = 0;
            for (int i = 0; i < sndFile.channels(); ++i)
                data[i] = new float[sndFile.frames()];
            // temporary buffer
            tmp = new float[1024*sndFile.channels()];
        }
        catch (...)
        {
            if (data)
                for (int i = 0; i < sndFile.channels(); ++i)
                    delete [] data[i];
            delete [] data;
            delete [] tmp;

            throw QObject::tr("out of memory");
        }

        // loading the data
        emit progress(0.0);
        qint64 emitFreq = sndFile.frames() / 1024 / 20;
        if (emitFreq < 1) emitFreq = 1;
        for (qint64 i = 0, k = 0; i < sndFile.frames(); i += 1024, ++k)
        {
            qint64 readed = sndFile.readf_float(tmp, 1024);
            for (qint64 cur = 0; cur < readed; ++cur)
                for (int ch = 0; ch < sndFile.channels(); ++ch)
                    data[ch][i+cur] = tmp[cur*sndFile.channels()+ch];
            if (k % emitFreq == 0)
                emit progress((100.0*i)/sndFile.frames());
        }
        emit progress(100.0);

        // free temporary buffer
        delete [] tmp;

        // create sound object
        sound = QSharedPointer<CSound>(new CSound(data, sndFile.channels(), sndFile.frames(), sndFile.sampleRate()));
        sound->setFormat(sndFile.format());
        sound->setFileName(fileName);
        sound->setProperty(CSound::Title, sndFile.getString(SF_STR_TITLE));
        sound->setProperty(CSound::Copyright, sndFile.getString(SF_STR_COPYRIGHT));
        sound->setProperty(CSound::Software, sndFile.getString(SF_STR_SOFTWARE));
        sound->setProperty(CSound::Artist, sndFile.getString(SF_STR_ARTIST));
        sound->setProperty(CSound::Comment, sndFile.getString(SF_STR_COMMENT));
        sound->setProperty(CSound::Date, sndFile.getString(SF_STR_DATE));
        sound->setProperty(CSound::Albom, sndFile.getString(SF_STR_ALBUM));
        sound->setProperty(CSound::License, sndFile.getString(SF_STR_LICENSE));
        sound->setProperty(CSound::TrackNumber, sndFile.getString(SF_STR_TRACKNUMBER));
        sound->setProperty(CSound::Genre, sndFile.getString(SF_STR_GENRE));

        emit done(sound);
    }
    catch (const QString &error)
    {
        emit fail(fileName, error);
    }
}
