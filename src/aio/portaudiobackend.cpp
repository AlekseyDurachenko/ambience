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
#include "portaudiobackend.h"
#include <QDebug>

/////////////////////////////////////////////////////////////////////////
namespace PortAudio {
/////////////////////////////////////////////////////////////////////////
static PaStream *stream = 0;

static int paPlayCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    return reinterpret_cast<PortAudioBackend*>(userData)->cbPlay(inputBuffer, outputBuffer,
        framesPerBuffer, timeInfo, statusFlags);
}

static void paStreamFinished(void* userData)
{
    reinterpret_cast<PortAudioBackend*>(userData)->cbFinish();
}

bool init(QString *reason)
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        if (reason)
            *reason = Pa_GetErrorText(err);

        return false;
    }

    return true;
}

void deinit()
{
    Pa_CloseStream(stream);
    Pa_Terminate();
}

QHash<int, QString> hostApiNames()
{
    QHash<int, QString> names;
    for (int i = 0; i < Pa_GetHostApiCount(); ++i)
       names[i] = Pa_GetHostApiInfo(i)->name;

    return names;
}

QHash<PaDeviceIndex, QString> deviceNames(PaHostApiIndex hostApi)
{
    QHash<int, QString> names;
    for (int i = 0; i < Pa_GetHostApiInfo(hostApi)->deviceCount; ++i)
    {
        PaDeviceIndex index = Pa_HostApiDeviceIndexToDeviceIndex(hostApi, i);
        names[index] = Pa_GetDeviceInfo(index)->name;
    }

    return names;
}

PaHostApiIndex defaultHostApi()
{
    return Pa_GetDefaultHostApi();
}

PaDeviceIndex defaultOutputDevice(PaHostApiIndex hostApi)
{
    return Pa_GetHostApiInfo(hostApi)->defaultOutputDevice;
}

PaDeviceIndex defaultInputDevice(PaHostApiIndex hostApi)
{
    return Pa_GetHostApiInfo(hostApi)->defaultInputDevice;
}

int maxOutputChannels(PaHostApiIndex hostApi, PaDeviceIndex device)
{
    return Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device))->maxOutputChannels;
}

int maxInputChannels(PaHostApiIndex hostApi, PaDeviceIndex device)
{
    return Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device))->maxInputChannels;
}

PaTime defaultLowOutputLatency(PaHostApiIndex hostApi, PaDeviceIndex device)
{
    return Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device))->defaultLowOutputLatency;
}

PaTime defaultLowInputLatency(PaHostApiIndex hostApi, PaDeviceIndex device)
{
    return Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device))->defaultLowInputLatency;
}

bool configure(const PaStreamParameters* inputParametsrs, const PaStreamParameters* outputParameters,
        double sampleRate, unsigned long samplePerBuffer, PaStreamFlags streamFlags,
        PortAudioBackend *cb, QString *reason)
{
    try
    {        
        if (stream)
        {
            PaError err = Pa_CloseStream(stream);
            stream = 0;
            if (err != paNoError)
                throw QString(Pa_GetErrorText(err));
        }

        PaError err = Pa_OpenStream(&stream, inputParametsrs, outputParameters, sampleRate, samplePerBuffer,
                streamFlags, &paPlayCallback, cb);
        if (err != paNoError)
            throw QString(Pa_GetErrorText(err));

        err = Pa_SetStreamFinishedCallback(stream, &paStreamFinished);
        if (err != paNoError)
            throw QString(Pa_GetErrorText(err));

        return true;
    }
    catch (const QString &error)
    {
        if (reason)
            *reason = error;
    }

    return false;
}

bool play(QString *reason)
{
    try
    {
        // when playing throws by the paComplite(paAbort) we should correct
        // call the Pa_StopStream, otherwise we cannot start plaing the stream
        // again
        if (Pa_IsStreamActive(stream) == 0 && Pa_IsStreamStopped(stream) == 0)
        {
            PaError err = Pa_StopStream(stream);
            if (err != paNoError)
                throw QString(Pa_GetErrorText(err));
        }

        PaError err = Pa_StartStream(stream);
        if (err != paNoError)
            throw QString(Pa_GetErrorText(err));

        return true;
    }
    catch (const QString &error)
    {
        if (reason)
            *reason = error;
    }

    return false;
}

bool stop(QString *reason)
{
    try
    {
        PaError err = Pa_StopStream(stream);
        if (err != paNoError)
            throw QString(Pa_GetErrorText(err));

        return true;
    }
    catch (const QString &error)
    {
        if (reason)
            *reason = error;
    }

    return false;
}

bool isPlayed()
{
    return (Pa_IsStreamActive(stream) == 1);
}

/////////////////////////////////////////////////////////////////////////
} // namespace PortAudio
/////////////////////////////////////////////////////////////////////////
