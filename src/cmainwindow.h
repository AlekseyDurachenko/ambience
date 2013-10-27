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
#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QThread>
#include "ui_mainwindow.h"
#include "ctaskexecutor.h"
#include "cprogressbar.h"
#include "csound.h"
#include "aglobal.h"

namespace Ui {
class MainWindow;
}

class CMainWindow : public QMainWindow
{
    Q_OBJECT    
public:
    explicit CMainWindow(QWidget *parent = 0);
    ~CMainWindow();
public slots:
    void slot_Open(const QString &fileName);

private slots:
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();
    void on_action_SaveSelectionAs_triggered();
    void on_action_Delete_triggered();
    void on_action_Trim_triggered();
    void on_action_Normalize_triggered();
    void on_action_Play_triggered();
    void on_action_PlayAll_triggered();
    void on_action_Pause_triggered();
    void on_action_Stop_triggered();
    void on_action_GoToStart_triggered();
    void on_action_GoToEnd_triggered();
    void on_action_Loop_triggered(bool checked);
    void on_action_About_triggered();
    void on_action_AboutQt_triggered();
    void on_action_Quit_triggered();
    void on_action_ZoomIn_triggered();
    void on_action_ZoomOut_triggered();
    void on_action_FitInWindow_triggered();
    void on_action_ZoomToSelection_triggered();
    void on_action_Close_triggered();
private slots:
    void playFromPosition(qint64 index);
    void openSoundDone(QSharedPointer<CSound> sound);
    void openSoundFail(const QString &fileName, const QString &message);
    void saveSoundDone();
    void saveSoundFail(const QString &fileName, const QString &message);
    void deleteSoundDone();
    void deleteSoundFail(const QString &message);
    void normalizeSoundDone();
    void normalizeSoundFail(const QString &message);
    void showProgressBar();
    void hideProgressBar();
    void slot_SelectionChanged();
    void slot_SoundChanged(int channels, qint64 frames, double sampleRate);
    void slot_SoundLocked();
    void slot_SoundUnlocked();
    void slot_PlayStarted(qint64 index);
    void slot_PlayStopped(qint64 index);
    void soundChanged(int channels, qint64 frames, double sampleRate);         
protected:
    void keyPressEvent(QKeyEvent *event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent *event);
private:
    void updateActions();
private:
    Ui::MainWindow *ui;
    CProgressBar *m_progressBar;
    QThread *m_taskThread;
    CTaskExecutor *m_taskExecutor;
    QSharedPointer<CSound> m_sound;
    int m_soundChannels;
    qint64 m_soundFrames;
    double m_soundSampleRate;
    bool m_soundIsLocked;
    bool m_paused;
};

#endif // CMAINWINDOW_H
