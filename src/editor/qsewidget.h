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
#ifndef QSEWIDGET_H
#define QSEWIDGET_H

#include <QWidget>
#include <QThread>
#include <QSharedPointer>
#include "qsescrollbar.h"
#include "qserender.h"
#include "csound.h"
#include "qseplotwidget.h"
#include "qselinearplot.h"
#include "qsegeometry.h"
#include "qseaxiswidget.h"
#include "qsehorizontalcontroller.h"
#include "qseverticalcontroller.h"
#include "qseplotcontroller.h"
#include "qsetimemetricprovider.h"
#include "qsecursor.h"
#include "qseselection.h"

class QseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QseWidget(QWidget *parent = 0);
    ~QseWidget();

    inline const QseGeometry &geometry() const;

    inline QseSelection *selection();
    inline QseCursor *positionCursor();
    inline QseCursor *playCursor();
signals:
    void playClicked(qint64 index);
public slots:
    // because at first time we cann't know the sound parameters,
    // we should set their by hand
    void setSound(QSharedPointer<CSound> sound, int channels, qint64 frames, double sampleRate);
    // set the geometry of the widgets. this slot set geometry for all waveform (for each nannel)
    // and controller for next operation
    void setGeometry(const QseGeometry &geometry);
private slots:
    // this slot must be connected to the CSound::changed() signal
    void setSoundParameters(int channels, qint64 frames, double sampleRate);
    // this slot must be connected to the CSound::locked() signal
    void setSoundLock();
    // this slot must be connected to the CSound::unlocked() signal
    void setSoundUnlock(bool changes);
    // when thread ends the read of samples, we shuold send it to requester plotter
    void setReadedSamples(qint64 id, QSharedPointer<CSound> sound, int channel,
        const QseGeometry &geometry, QList<double> minimums, QList<double> maximums);
    // when one of the plotter needs the peak, it call this slot and it slot
    // call the thread method
    void peaksNeeded(qint64 id, int channel, const QseGeometry &geometry, const QRect &rect);
    //
    void slotPlayClicked(qint64 index);
    void slotScrollBarValueChanged(int value);
private:
    // rebuild the widget according to the current channel count
    // widget will be cleared if mSound.isNull() == true
    void rebuildTheWidget();
private:
    // this variable is used for waveform peak calculation
    // because it is the time cost operation and must be executed
    // in the separated thread
    QThread *m_readerThread;
    QList<QseRender *> m_readerList;
    // the sound should be displayd at the time
    // if mSound.isNull() == true, then sound shouldn't displaed
    // and all operation width the manipulators must be ignored
    QSharedPointer<CSound> m_sound;
    // the cached parameters of the mSound, becouse direct acces
    // without locking is time cost operation. but last state of the
    // sound need for other calculation of the gui manipulators.
    // this variabled will be updated when mSound emit the "changed()"
    // signal.
    int m_soundChannels;
    qint64 m_soundFrames;
    double m_soundSampleRate;
    bool m_soundIsLocked;
    // this variable indicate then mSound is processed at the time
    // and set/reset by "locked()", "unlocked()" signals of the mSound.
    // * bool mSoundIsLocked;
    // this variable is waveform and manipulators such as cursor, play cursor
    // selection, e.t.c.
    QList<QsePlotWidget *> m_waveformWidgetList;
    QList<QseLinearPlot *> m_linearPlotterList;
    QList<QseAxisWidget *> m_amplitudeList;
    QseSelectionPlot *m_selectionPlot;
    QseSelection *m_selection;
    QseCursorPlot *m_positionCursorPlot, *m_playCursorPlot;
    QseCursor *m_positionCursor, *m_playCursor;
    //QseController *mController;
    QseGeometry m_geometry;
    QseAxisWidget *m_timeWidget;
    QseScrollBar *m_scrollBar;
    QseVerticalController *m_amplitudeController;
    QseHorizontalController *m_timeController;
    QsePlotController *m_waveformController;
    QseTimeMetricProvider *m_timeProvider;
};

const QseGeometry &QseWidget::geometry() const
{
    return m_geometry;
}

QseSelection *QseWidget::selection()
{
    return m_selection;
}

QseCursor *QseWidget::positionCursor()
{
    return m_positionCursor;
}

QseCursor *QseWidget::playCursor()
{
    return m_playCursor;
}

#endif // QSEWIDGET_H
