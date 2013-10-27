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
#include "cprogressbar.h"
#include <QPaintEvent>
#include <QPainter>

CProgressBar::CProgressBar(QWidget *parent) :
    QWidget(parent)
{
    m_max = 100.0;
    m_min = 0.0;
    m_value = 0.0;
    m_progressColor = Qt::darkBlue;
    m_backgroundColor = Qt::gray;
    m_percentColor = Qt::white;
}

void CProgressBar::paintEvent(QPaintEvent *)
{
    if (height() > 0 && width() > 0)
    {
        int w = qRound(width() * (m_value - m_min) / (m_max - m_min));
        QString percentText = QString::number(100 * (m_value - m_min) / (m_max - m_min), 'f', 1) + QString("%");

        QPainter painter(this);
        painter.setPen(m_progressColor);
        painter.setBrush(m_progressColor);
        painter.drawRect(0, 0, w, height());
        painter.setPen(m_backgroundColor);
        painter.setBrush(m_backgroundColor);
        painter.drawRect(w, 0, width() - w, height());
        painter.setFont(m_percentFont);
        painter.setPen(m_percentColor);
        painter.drawText(rect(), Qt::AlignCenter, percentText);
    }
}

void CProgressBar::resizeEvent(QResizeEvent *)
{
    if (m_percentFont.pixelSize() != height())
        m_percentFont.setPixelSize(height());
}

void CProgressBar::setRange(double min, double max)
{
    m_min = min;
    m_max = max;

    update();
}

void CProgressBar::setValue(double value)
{
    if (value > m_max)
        m_value = m_max;
    else if (m_value < m_min)
        m_value = m_min;
    else
        m_value = value;

    update();
}

void CProgressBar::reset()
{
    m_value = m_min;

    update();
}
