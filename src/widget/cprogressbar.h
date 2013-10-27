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
#ifndef CPROGRESSBAR_H
#define CPROGRESSBAR_H

#include <QWidget>

class CProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit CProgressBar(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
public slots:
    void setRange(double min, double max);
    void setValue(double value);
    void reset();
private:
    double m_min, m_max, m_value;
    QFont m_percentFont;
    QColor m_progressColor;
    QColor m_backgroundColor;
    QColor m_percentColor;
};

#endif // CPROGRESSBAR_H
