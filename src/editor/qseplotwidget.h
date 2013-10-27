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
#ifndef QSEPLOTWIDGET_H
#define QSEPLOTWIDGET_H

#include <QWidget>
#include "qsecursorplot.h"
#include "qseselectionplot.h"
#include "qselinearplot.h"
#include "qsegeometry.h"
#include "qseabstractwidget.h"
#include "qsecoverplot.h"

class QsePlotWidget : public QseAbstractWidget
{
    Q_OBJECT
public:
    explicit QsePlotWidget(QWidget *parent = 0);
    // style
    // additional manipulators
    inline QseCursorPlot *position();
    inline QseCursorPlot *play();
    inline QseSelectionPlot *selection();
    inline QseLinearPlot *plotter();
    inline QseCoverPlot *cover();
    void setPosition(QseCursorPlot *cursor);
    void setPlay(QseCursorPlot *cursor);
    void setSelection(QseSelectionPlot *selection);
    void setPlotter(QseLinearPlot *plotter);
    void setCover(QseCoverPlot *cover);
protected:
    void paintEvent(QPaintEvent *event);
private:
    QImage m_waveformImage;
    QseCursorPlot *m_position, *m_play;
    QseSelectionPlot *m_selection;
    QseLinearPlot *m_waveformPlot;
    QseCoverPlot *m_coverPlot;
};


QseCursorPlot *QsePlotWidget::position()
{
    return m_position;
}

QseCursorPlot *QsePlotWidget::play()
{
    return m_play;
}

QseSelectionPlot *QsePlotWidget::selection()
{
    return m_selection;
}

QseLinearPlot *QsePlotWidget::plotter()
{
    return m_waveformPlot;
}

QseCoverPlot *QsePlotWidget::cover()
{
    return m_coverPlot;
}

#endif // QSEPLOTWIDGET_H
