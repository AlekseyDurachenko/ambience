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
#include "qselinearplot.h"
#include "qsefunc.h"
#include <QDebug>

QseLinearPlot::QseLinearPlot(QObject *parent) :
    QseAbstractPlot(parent)
{
    m_channel = -1;
    m_frames = 0;
    m_lastId = -1;
    m_id = 0;
    m_redraw = true;
    m_pen.setColor(Qt::darkBlue);
    m_opacity = 0.9;
}

void QseLinearPlot::setPen(const QPen &pen)
{
    if (m_pen != pen)
    {
        m_pen = pen;
        emit changed();
    }
}

void QseLinearPlot::setOpacity(qreal opacity)
{
    if (m_opacity != opacity)
    {
        m_opacity = opacity;
        emit changed();
    }
}

void QseLinearPlot::setChannel(int channel)
{
    if (channel != m_channel)
    {
        m_channel = channel;
        emit changed();
    }
}

void QseLinearPlot::setFrames(qint64 frames)
{
    if (frames != m_frames)
    {
        m_frames = frames;
        emit changed();
    }
}

void QseLinearPlot::reset()
{
    m_redraw = true;
    emit changed();
}

void QseLinearPlot::setPeaks(qint64 id, const QseGeometry &geometry,
    QList<double> minimums, QList<double> maximums)
{
    m_minimums = minimums;
    m_maximums = maximums;

    m_cacheSamplePerPixel = geometry.samplePerPixel();
    m_cacheX = geometry.x();
    m_lastId = id;

    setUpdateOnce(true);

    emit changed();
}

bool QseLinearPlot::hasChanges(const QRect &rect, const QseGeometry &geometry)
{
    return (m_lastGeometry != geometry || rect != m_lastRect || isUpdateOnce() || m_redraw);
}

void QseLinearPlot::draw(QPainter *painter, const QRect &rect, const QseGeometry &geometry)
{
    // the channel for the waveform is unknow, therefore we cannot draw the waveform
    if (m_channel == -1)
        return;

    if (m_lastGeometry != geometry || rect != m_lastRect || m_redraw)
    {
        ++m_id;
        emit waitForId(m_id);
        emit peaksNeeded(m_id, m_channel, geometry, rect);
    }

    if (m_id == m_lastId && !m_redraw)
    {
        painter->setPen(m_pen);
        painter->setOpacity(m_opacity);
        if (geometry.samplePerPixel() < 0)
            drawAsLine(painter, m_minimums, rect, geometry);
        else if (geometry.samplePerPixel() > 0)
            drawAsPeak(painter, m_minimums, m_maximums, rect, geometry);
        setUpdateOnce(false);
    }
    else if (m_lastId != -1 && m_cacheSamplePerPixel == geometry.samplePerPixel() && !m_redraw)
    {
        painter->setPen(m_pen);
        painter->setOpacity(m_opacity);
        QseGeometry gm = geometry;
        gm.setX(m_cacheX);
        gm.setSamplePerPixel(m_cacheSamplePerPixel);
        if (gm.samplePerPixel() < 0)
            drawAsLine(painter, m_minimums, rect, gm, geometry.x() - gm.x());
        else if (gm.samplePerPixel() > 0)
            drawAsPeak(painter, m_minimums, m_maximums, rect, gm, geometry.x() - gm.x());
    }

    // store last values for next iteration comparing
    m_redraw = false;
    m_lastGeometry = geometry;
    m_lastRect = rect;
}


void QseLinearPlot::drawAsLine(QPainter *painter, const QList<double> &points, const QRect &rect,
    const QseGeometry &geometry, int offset)
{
    int space = 0;
    int first = 0;
    if (offset < 0)
        space = qAbs(geometry.samplePerPixel() * offset);
    else
        first = offset;

    for (int i = first + 1; i < points.count(); ++i)
    {
        double x1 = space + (i - 1 - first) * qAbs(geometry.samplePerPixel());
        double x2 = space + (i - first) * qAbs(geometry.samplePerPixel());
        double y1 = rect.height()/2 - (geometry.y() + points[i-1]) * rect.height() / geometry.height();
        double y2 = rect.height()/2 - (geometry.y() + points[i]) * rect.height() / geometry.height();

        if (x1 >= rect.width())
            break;

        if ((y1 < 0 && y2 < 0) || (y1 >= rect.height() && y2 >= rect.height()))
            continue;

        double yy1 = y1;
        double yy2 = y2;
        double xx1 = x1;
        double xx2 = x2;

        if (y1 >= rect.height())
            QseFunc::findLineIntersection(x1, y1, x2, y2, x1, rect.height()-1, x2, rect.height()-1, xx1, yy1);
        if (y2 >= rect.height())
            QseFunc::findLineIntersection(x1, y1, x2, y2, x1, rect.height()-1, x2, rect.height()-1, xx2, yy2);
        if (y1 < 0)
            QseFunc::findLineIntersection(x1, y1, x2, y2, x1, 0, x2, 0, xx1, yy1);
        if (y2 < 0)
            QseFunc::findLineIntersection(x1, y1, x2, y2, x1, 0, x2, 0, xx2, yy2);

        painter->drawLine(QPointF(xx1, yy1), QPointF(xx2, yy2));
    }
}

void QseLinearPlot::drawAsPeak(QPainter *painter, const QList<double> &minimums,
    const QList<double> &maximums, const QRect &rect, const QseGeometry &geometry, int offset)
{    
    int space = 0;
    int first = 0;
    if (offset < 0)
        space = -offset;
    else
        first = offset;

    if (first >= maximums.count())
        return;

    double prevMin = rect.height()/2-(geometry.y()+minimums[first])*rect.height()/geometry.height();
    double prevMax = rect.height()/2-(geometry.y()+maximums[first])*rect.height()/geometry.height();
    double x1 = space;
    double x2 = space;
    if (x1 >= rect.width())
        return;
    if (prevMin < 0.0) prevMin = 0.0;
    if (prevMax < 0.0) prevMax = 0.0;
    if (prevMin >= rect.height()) prevMin = rect.height()-1;
    if (prevMax >= rect.height()) prevMax = rect.height()-1;
    if (prevMin == prevMax)
        painter->drawPoint(QPointF(x1, prevMin));
    else
        painter->drawLine(QPointF(x1, prevMin), QPointF(x2, prevMax));

    for (int i = first + 1; i < m_minimums.count(); ++i)
    {
        x1 = space + i - first;
        x2 = space + i - first;
        if (x1 >= rect.width())
            break;
        double curMin = rect.height()/2-(geometry.y()+minimums[i])*rect.height()/geometry.height();
        double curMax = rect.height()/2-(geometry.y()+maximums[i])*rect.height()/geometry.height();
        if (! ((curMin < 0.0 && curMax < 0.0) ||
              (curMin >= rect.height() && curMax >= rect.height())))
        {
            if (curMin < 0.0) curMin = 0.0;
            if (curMax < 0.0) curMax = 0.0;
            if (curMin >= rect.height()) curMin = rect.height()-1;
            if (curMax >= rect.height()) curMax = rect.height()-1;

            //Условия подтяжки к предыдущей линии, только если есть разрыв
            if (curMin < prevMax) curMin = prevMax-1;
            if (curMax > prevMin) curMax = prevMin+1;

            //
            prevMin = curMin;
            prevMax = curMax;

            if (curMin == curMax)
                painter->drawPoint(QPointF(x1, curMin));
            else
                painter->drawLine(QPointF(x1, curMin), QPointF(x2, curMax));
        }
    }
}
