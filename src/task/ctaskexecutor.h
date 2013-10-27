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
#ifndef CTASKEXECUTOR_H
#define CTASKEXECUTOR_H

#include <QObject>
#include "cabstracttask.h"

class CTaskExecutor : public QObject
{
    Q_OBJECT
public:
    explicit CTaskExecutor(QObject *parent = 0);   
    void execute(CAbstractTask *task);
signals:
    void executed(CAbstractTask *task);
private slots:
    void privateExecute(CAbstractTask *task);
};

#endif // CTASKEXECUTOR_H
