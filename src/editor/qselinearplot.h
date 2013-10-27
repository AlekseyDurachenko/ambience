// Copyright 2013, Durachenko Aleksey V. <durachenko.aleksey@gmail.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#ifndef QSELINEARPLOT_H
#define QSELINEARPLOT_H

#include <QObject>
#include <QPainter>
#include <QPen>
#include "qseabstractplot.h"

class QseLinearPlot : public QseAbstractPlot
{
    Q_OBJECT
public:
    explicit QseLinearPlot(QObject *parent = 0);

    // style
    inline int channel() const;
    inline qint64 frames() const;
    inline QPen pen() const;
    inline qreal opacity() const;

    void setChannel(int channel);
    void setFrames(qint64 frames);
    void setPen(const QPen &pen);
    void setOpacity(qreal opacity);

    void reset();
    bool hasChanges(const QRect &rect, const QseGeometry &geometry);
    void draw(QPainter *painter, const QRect &rect, const QseGeometry &geometry);
public slots:
    void setPeaks(qint64 id, const QseGeometry &geometry, QList<double> minimums, QList<double> maximums);
signals:
    void peaksNeeded(qint64 id, int channel, const QseGeometry &geometry, const QRect &rect);
    void waitForId(int id);
private:
    void drawAsLine(QPainter *painter, const QList<double> &points, const QRect &rect,
        const QseGeometry &geometry, int offset = 0);
    void drawAsPeak(QPainter *painter, const QList<double> &minimums, const QList<double> &maximums,
        const QRect &rect, const QseGeometry &geometry, int offset = 0);
private:
    int m_channel;
    qint64 m_frames;
    // waveform style
    QPen m_pen;
    qreal m_opacity;
    // this array stored the waveform peaks
    QList<double> m_minimums, m_maximums;
    qint64 m_cacheX, m_cacheSamplePerPixel;
    // the values which was used for previouse rendering
    QseGeometry m_lastGeometry;
    QRect m_lastRect;
    qint64 m_lastId, m_id;
    bool m_redraw;
};


int QseLinearPlot::channel() const
{
    return m_channel;
}

qint64 QseLinearPlot::frames() const
{
    return m_frames;
}

QPen QseLinearPlot::pen() const
{
    return m_pen;
}

qreal QseLinearPlot::opacity() const
{
    return m_opacity;
}


#endif // QSELINEARPLOT_H
