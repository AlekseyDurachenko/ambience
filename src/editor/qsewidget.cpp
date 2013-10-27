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
#include "qsewidget.h"
#include <QtGui>
#include <QVBoxLayout>
#include <QDebug>
#include "qsefunc.h"
#include "qsetimemetricprovider.h"
#include "qseabsolutemetricprovider.h"

QseWidget::QseWidget(QWidget *parent) :
    QWidget(parent)
{
    // selection
    m_selection = new QseSelection(this);
    m_selectionPlot = new QseSelectionPlot(this);
    m_selectionPlot->setSelection(m_selection);

    // position cursor
    m_positionCursor = new QseCursor(this);
    m_positionCursorPlot = new QseCursorPlot(this);
    m_positionCursorPlot->setCursor(m_positionCursor);

    // play cursor
    m_playCursor = new QseCursor(this);
    m_playCursorPlot = new QseCursorPlot(this);
    m_playCursorPlot->setPen(QPen(Qt::darkGreen));
    m_playCursorPlot->setCursor(m_playCursor);

    // scroll bar
    m_scrollBar = new QseScrollBar(this);
    m_scrollBar->hide();
    connect(m_scrollBar, SIGNAL(valueChanged(int)), this, SLOT(slotScrollBarValueChanged(int)));

    // time widget && controller
    m_timeController = new QseHorizontalController(this);
    m_timeWidget = new QseAxisWidget(QseAxisWidget::Top, this);
    m_timeProvider = new QseTimeMetricProvider(m_timeWidget);
    m_timeWidget->setController(m_timeController);
    m_timeWidget->setMetricProvider(m_timeProvider);
    m_timeWidget->hide();
    connect(m_timeController, SIGNAL(geometryChanged(QseGeometry)), this, SLOT(setGeometry(QseGeometry)));

    // amplitude controller
    m_amplitudeController = new QseVerticalController(this);
    connect(m_amplitudeController, SIGNAL(geometryChanged(QseGeometry)), this, SLOT(setGeometry(QseGeometry)));

    // waveform controller
    m_waveformController = new QsePlotController(this);
    m_waveformController->setPosition(m_positionCursor);
    m_waveformController->setSelection(m_selection);
    connect(m_waveformController, SIGNAL(geometryChanged(QseGeometry)), this, SLOT(setGeometry(QseGeometry)));
    connect(m_waveformController, SIGNAL(playClicked(qint64)), this, SLOT(slotPlayClicked(qint64)));

    // in the thread will be calculated the all peak and somthing else
    m_readerThread = new QThread(this);
    m_readerThread->start();
}

QseWidget::~QseWidget()
{
    foreach (QseRender *reader, m_readerList)
    {
        reader->waitForId(-1);
        reader->deleteLater();
    }
    m_readerThread->quit();
    m_readerThread->wait();
}

void QseWidget::setSound(QSharedPointer<CSound> sound, int channels, qint64 frames, double sampleRate)
{
    if (sound == m_sound)
        return;

    if (m_sound)
        disconnect(m_sound.data(), 0, this, 0);

    m_sound = sound;
    if (m_sound)
    {
        connect(m_sound.data(), SIGNAL(changed(int,qint64,double)), this, SLOT(setSoundParameters(int,qint64,double)));
        connect(m_sound.data(), SIGNAL(locked()), this, SLOT(setSoundLock()));
        connect(m_sound.data(), SIGNAL(unlocked(bool)), this, SLOT(setSoundUnlock(bool)));
    }

    setSoundParameters(channels, frames, sampleRate);

    // reset manipulators to default, because it is the new sound
    m_selectionPlot->reset();
    m_positionCursorPlot->reset();
    m_playCursorPlot->reset();

    setGeometry(QseGeometry());
}

void QseWidget::setGeometry(const QseGeometry &geometry)
{
    QseGeometry newGeometry = geometry;
    if (geometry.x() < 0)
        newGeometry.setX(0);
    if (geometry.y() < -1.0)
        newGeometry.setY(-1.0);
    if (geometry.y() > 1.0)
        newGeometry.setY(1.0);

    foreach (QsePlotWidget *plot, m_waveformWidgetList)
        plot->setGeometry(newGeometry);
    foreach (QseAxisWidget *amp, m_amplitudeList)
        amp->setGeometry(newGeometry);
    m_timeWidget->setGeometry(newGeometry);
    m_scrollBar->setSoundPosition(geometry.x(), QseFunc::width(geometry.samplePerPixel(), m_soundFrames));

    m_geometry = newGeometry;
}

void QseWidget::setSoundParameters(int channels, qint64 frames, double sampleRate)
{
    m_soundChannels = channels;
    m_soundFrames = frames;
    m_soundSampleRate = sampleRate;
    m_soundIsLocked = false;

    // becouse some changes can be critical, we should try to rebuild the widget
    rebuildTheWidget();

    // apply the limits
    m_selection->setMaximum(frames-1);
    m_positionCursor->setMaximum(frames-1);
    m_playCursor->setMaximum(frames-1);
    m_timeProvider->setSampleRate(sampleRate);
    foreach (QseLinearPlot *plot, m_linearPlotterList)
    {
        plot->setFrames(frames);
        plot->reset();
    }

    // reacalculate geometry for new sound, becouse it may be out of range
    setGeometry(m_geometry);
}

void QseWidget::setSoundLock()
{
    foreach (QseRender *reader, m_readerList)
        reader->waitForId(-1);
    m_soundIsLocked = true;
}

void QseWidget::setSoundUnlock(bool changes)
{
    m_soundIsLocked = false;
    if (changes)
        rebuildTheWidget();
}

void QseWidget::setReadedSamples(qint64 id, QSharedPointer<CSound> sound, int channel,
    const QseGeometry &geometry, QList<double> minimums, QList<double> maximums)
{
    if (channel < m_linearPlotterList.count() && m_sound == sound)
        m_linearPlotterList[channel]->setPeaks(id, geometry, minimums, maximums);
}

void QseWidget::peaksNeeded(qint64 id, int channel, const QseGeometry &geometry, const QRect &rect)
{
    if (m_soundIsLocked)
        return;

    QMetaObject::invokeMethod(m_readerList[channel], "readPeaks", Qt::QueuedConnection,
            Q_ARG(qint64, id), Q_ARG(QSharedPointer<CSound>, m_sound), Q_ARG(int, channel),
                              Q_ARG(QseGeometry, geometry), Q_ARG(QRect, rect));
}

void QseWidget::slotPlayClicked(qint64 index)
{
    if (index >= 0 && index < m_soundFrames)
        emit playClicked(index);
}

void QseWidget::slotScrollBarValueChanged(int value)
{
    setGeometry(QseGeometry(value, m_geometry.y(), m_geometry.samplePerPixel(), m_geometry.height()));
}

void QseWidget::rebuildTheWidget()
{
    // at any time when channel count is changed, we should
    // correct GUI for the changes. We suppouse the channel count = 0,
    // if mSound.isNull() == true.
    // if channel count == 0, we show the empty widget
    if (m_sound.isNull() || m_soundChannels != m_waveformWidgetList.count())
    {
        qDeleteAll(m_waveformWidgetList);
        qDeleteAll(m_linearPlotterList);
        qDeleteAll(m_amplitudeList);
        foreach (QseRender *reader, m_readerList)
        {
            reader->waitForId(-1);
            reader->deleteLater();
        }
        m_waveformWidgetList.clear();
        m_linearPlotterList.clear();
        m_amplitudeList.clear();
        m_readerList.clear();

        // hide invisible elements
        m_timeWidget->hide();
        m_scrollBar->hide();

        // hide the layout
        delete layout();
    }

    if (!m_sound.isNull() && m_soundChannels != m_waveformWidgetList.count())
    {
        QGridLayout *grid = new QGridLayout;
        grid->setSpacing(0);
        grid->setContentsMargins(0, 0, 0, 0);

        grid->addWidget(m_timeWidget, 0, 1);
        for (int channel = 0, current = 1; channel < m_soundChannels; ++channel, ++current)
        {
            QsePlotWidget *plotWidget = new QsePlotWidget(this);
            QseAxisWidget *ampWidget = new QseAxisWidget(QseAxisWidget::Left, this);
            QseAbsoluteMetricProvider *amplitudeProvider = new QseAbsoluteMetricProvider(ampWidget);
            QseLinearPlot *plotter = new QseLinearPlot(this);
            QseCoverPlot *cover = new QseCoverPlot(this);
            QseRender *reader = new QseRender;
            plotWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            plotWidget->setCover(cover);
            plotWidget->setPlotter(plotter);
            plotWidget->setPosition(m_positionCursorPlot);
            plotWidget->setPlay(m_playCursorPlot);
            plotWidget->setSelection(m_selectionPlot);
            plotWidget->setGeometry(m_geometry);
            plotWidget->setController(m_waveformController);
            ampWidget->setController(m_amplitudeController);
            ampWidget->setMetricProvider(amplitudeProvider);
            plotter->setChannel(channel);
            plotter->setFrames(m_soundFrames);
            cover->setHorizontalMetricProvider(m_timeWidget->metricProvider());
            cover->setVerticalMetricProvider(ampWidget->metricProvider());
            reader->moveToThread(m_readerThread);

            if (channel > 0 && channel < m_soundChannels)
            {
                QWidget* widget = new QWidget(this);
                widget->setMinimumHeight(5);
                grid->addWidget(widget, current, 1);
                ++current;
            }

            grid->addWidget(plotWidget, current, 1);
            grid->addWidget(ampWidget, current, 0);

            connect(plotter, SIGNAL(peaksNeeded(qint64,int,QseGeometry,QRect)), this, SLOT(peaksNeeded(qint64,int,QseGeometry,QRect)));
            connect(plotter, SIGNAL(waitForId(int)), reader, SLOT(waitForId(int)), Qt::DirectConnection);
            connect(reader, SIGNAL(peaksReaded(qint64,QSharedPointer<CSound>,int,QseGeometry,QList<double>,QList<double>)), this, SLOT(setReadedSamples(qint64,QSharedPointer<CSound>,int,QseGeometry,QList<double>,QList<double>)), Qt::QueuedConnection);

            m_readerList.push_back(reader);
            m_linearPlotterList.push_back(plotter);
            m_waveformWidgetList.push_back(plotWidget);
            m_amplitudeList.push_back(ampWidget);

        }
        grid->addWidget(m_scrollBar, m_soundChannels+2, 1);

        // show invisible elements
        m_timeWidget->show();
        m_scrollBar->show();

        // install new layout
        setLayout(grid);
    }
}
