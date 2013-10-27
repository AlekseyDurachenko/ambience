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
#ifndef PORTAUDIOBACKEND_CPP
#define PORTAUDIOBACKEND_CPP

#include <portaudio.h>
#include <QString>
#include <QHash>

/*! This is abstract class will be used for provide the data to the port-audio.
 * usually it used for multible inheritance
 */
class PortAudioBackend
{
public:
    virtual int cbPlay(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags) = 0;
    virtual void cbFinish() = 0;
};

/////////////////////////////////////////////////////////////////////////
namespace PortAudio {
/////////////////////////////////////////////////////////////////////////

bool init(QString *reason = 0);
void deinit();
QHash<PaHostApiIndex, QString> hostApiNames();
QHash<PaDeviceIndex, QString> deviceNames(PaHostApiIndex hostApi);
PaHostApiIndex defaultHostApi();
PaDeviceIndex defaultOutputDevice(PaHostApiIndex hostApi);
PaDeviceIndex defaultInputDevice(PaHostApiIndex hostApi);
int maxOutputChannels(PaHostApiIndex hostApi, PaDeviceIndex device);
int maxInputChannels(PaHostApiIndex hostApi, PaDeviceIndex device);
PaTime defaultLowOutputLatency(PaHostApiIndex hostApi, PaDeviceIndex device);
PaTime defaultLowInputLatency(PaHostApiIndex hostApi, PaDeviceIndex device);
bool configure(const PaStreamParameters* inputParametsrs, const PaStreamParameters* outputParameters,
        double sampleRate, unsigned long samplePerBuffer, PaStreamFlags streamFlags,
        PortAudioBackend *cb, QString *reason = 0);
bool play(QString *reason = 0);
bool stop(QString *reason = 0);
bool isPlayed();

/////////////////////////////////////////////////////////////////////////
} // namespace PortAudio
/////////////////////////////////////////////////////////////////////////

#endif // PORTAUDIOBACKEND_CPP
