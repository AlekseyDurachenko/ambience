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
#include "cmainwindow.h"

#include <QSettings>
#include <QInputDialog>
#include <QDebug>
#include <QUrl>
#include <QFileDialog>
#include <sndfile.h>
#include <ctaskdelete.h>
#include "csound.h"
#include <QThread>
#include <QPushButton>
#include <QProgressBar>
#include "ctaskexecutor.h"
#include "ctaskopen.h"
#include "ctasksave.h"
#include <QMessageBox>
#include "global.h"
#include "ctasktrim.h"
#include "ctasknormalize.h"

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(G_SHORT_PROGRAM_NAME());
    m_progressBar = new CProgressBar(this);
    m_progressBar->hide();

    m_taskThread = new QThread(this);
    m_taskExecutor = new CTaskExecutor;
    m_taskExecutor->moveToThread(m_taskThread);
    m_taskThread->start();

    connect(g_soundPlayer, SIGNAL(indexChanged(qint64)), ui->soundWidget->playCursor(), SLOT(setIndex(qint64)));
    //connect(gSoundPlayer, SIGNAL(stopped(qint64)), ui->soundWidget->playCursor(), SLOT(reset()));
    connect(g_soundPlayer, SIGNAL(started(qint64)), this, SLOT(slot_PlayStarted(qint64)));
    connect(g_soundPlayer, SIGNAL(stopped(qint64)), this, SLOT(slot_PlayStopped(qint64)));
    connect(ui->soundWidget, SIGNAL(playClicked(qint64)), this, SLOT(playFromPosition(qint64)));
    connect(ui->soundWidget->selection(), SIGNAL(changed()), this, SLOT(slot_SelectionChanged()));

    m_paused = false;
    m_soundIsLocked = false;
    updateActions();
}

CMainWindow::~CMainWindow()
{
    m_taskThread->quit();
    delete m_taskExecutor;
}

void CMainWindow::slot_Open(const QString &fileName)
{
    CTaskOpen* taskOpen = new CTaskOpen;
    taskOpen->setFileName(fileName);
    connect(taskOpen, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskOpen, SIGNAL(done(QSharedPointer<CSound>)), this, SLOT(hideProgressBar()));
    connect(taskOpen, SIGNAL(done(QSharedPointer<CSound>)), this, SLOT(openSoundDone(QSharedPointer<CSound>)));
    connect(taskOpen, SIGNAL(fail(QString, QString)), this, SLOT(hideProgressBar()));
    connect(taskOpen, SIGNAL(fail(QString, QString)), this, SLOT(openSoundFail(QString, QString)));
    m_taskExecutor->execute(taskOpen);

    showProgressBar();
}

void CMainWindow::on_action_Open_triggered()
{
    G_SETTINGS_INIT();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
        settings.value("LastDirectory").toString(), tr("All (* *.*)"));

    if (fileName.isEmpty())
        return;

    settings.setValue("LastDirectory", QFileInfo(fileName).absolutePath());

    slot_Open(fileName);
}

void CMainWindow::on_action_Save_triggered()
{    
    if (QMessageBox::question(this, tr("Question"), tr("Do you want to replace the : %1").arg(m_sound->fileName()),
                              QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
        return;

    CTaskSave* taskSave = new CTaskSave;
    taskSave->setSound(m_sound);
    connect(taskSave, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskSave, SIGNAL(done()), this, SLOT(hideProgressBar()));
    connect(taskSave, SIGNAL(done()), this, SLOT(saveSoundDone()));
    connect(taskSave, SIGNAL(fail(QString, QString)), this, SLOT(hideProgressBar()));
    connect(taskSave, SIGNAL(fail(QString, QString)), this, SLOT(saveSoundFail(QString, QString)));
    m_taskExecutor->execute(taskSave);

    showProgressBar();
}

void CMainWindow::on_action_SaveAs_triggered()
{
    G_SETTINGS_INIT();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
        settings.value("LastDirectory").toString(), tr("All (* *.*)"));

    if (fileName.isEmpty())
        return;

    settings.setValue("LastDirectory", QFileInfo(fileName).absolutePath());

    CTaskSave* taskSave = new CTaskSave;
    taskSave->setSound(m_sound);
    taskSave->setFileName(fileName);

    connect(taskSave, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskSave, SIGNAL(done()), this, SLOT(hideProgressBar()));
    connect(taskSave, SIGNAL(done()), this, SLOT(saveSoundDone()));
    connect(taskSave, SIGNAL(fail(QString, QString)), this, SLOT(hideProgressBar()));
    connect(taskSave, SIGNAL(fail(QString, QString)), this, SLOT(saveSoundFail(QString, QString)));
    m_taskExecutor->execute(taskSave);

    showProgressBar();
}

void CMainWindow::on_action_SaveSelectionAs_triggered()
{
    G_SETTINGS_INIT();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open File"),
        settings.value("LastDirectory").toString(), tr("All (* *.*)"));

    if (fileName.isEmpty())
        return;

    settings.setValue("LastDirectory", QFileInfo(fileName).absolutePath());

    CTaskSave* taskSave = new CTaskSave;
    taskSave->setSound(m_sound);
    taskSave->setFileName(fileName);
    taskSave->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());

    connect(taskSave, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskSave, SIGNAL(done()), this, SLOT(hideProgressBar()));
    connect(taskSave, SIGNAL(done()), this, SLOT(saveSoundDone()));
    connect(taskSave, SIGNAL(fail(QString, QString)), this, SLOT(hideProgressBar()));
    connect(taskSave, SIGNAL(fail(QString, QString)), this, SLOT(saveSoundFail(QString, QString)));
    m_taskExecutor->execute(taskSave);

    showProgressBar();
}


void CMainWindow::openSoundDone(QSharedPointer<CSound> sound)
{
    // at first time, after open the sound, nobody may write in to the
    // sound, therefore we can read directly from the sound
    // without fear of the corruption of the data

    // initialization the sound player
    if (g_soundPlayer->isPlayed())
        g_soundPlayer->stop();
    g_soundPlayer->setSound(sound);
    g_soundPlayer->setRange(0, sound->frames() - 1);

    // initialization the sound editor widget
    QseGeometry geometry;
    if (sound->frames() > ui->soundWidget->width())
        geometry.setSamplePerPixel(sound->frames() / ui->soundWidget->width());
    ui->soundWidget->setSound(sound, sound->channels(), sound->frames(), sound->sampleRate());
    ui->soundWidget->selection()->reset();
    ui->soundWidget->positionCursor()->reset();
    ui->soundWidget->setGeometry(geometry);

    m_sound = sound;
    m_soundChannels = sound->channels();
    m_soundFrames = sound->frames();
    m_soundSampleRate = sound->sampleRate();
    m_soundIsLocked = false;
    connect(m_sound.data(), SIGNAL(locked()), this, SLOT(slot_SoundLocked()));
    connect(m_sound.data(), SIGNAL(unlocked()), this, SLOT(slot_SoundUnlocked()));
    connect(m_sound.data(), SIGNAL(changed(int,qint64,double)), this, SLOT(slot_SoundChanged(int,qint64,double)));
    setWindowTitle(m_sound->fileName() + " - " + G_SHORT_PROGRAM_NAME());
    updateActions();
}

void CMainWindow::openSoundFail(const QString &fileName, const QString &message)
{
    QMessageBox::warning(this, tr("Open"), tr("Can't open %1: %2").arg(fileName, message));
    updateActions();
}

void CMainWindow::saveSoundDone()
{
    QMessageBox::information(this, tr("Done"), tr("Done"));
    updateActions();
}

void CMainWindow::saveSoundFail(const QString &fileName, const QString &message)
{
    QMessageBox::warning(this, tr("Open"), tr("Can't open %1: %2").arg(fileName, message));
    updateActions();
}


void CMainWindow::showProgressBar()
{
    ui->statusbar->addWidget(m_progressBar, 1);
    m_progressBar->show();
}

void CMainWindow::hideProgressBar()
{
    ui->statusbar->removeWidget(m_progressBar);
}

void CMainWindow::slot_SoundChanged(int channels, qint64 frames, double sampleRate)
{
    m_soundChannels = channels;
    m_soundFrames = frames;
    m_soundSampleRate = sampleRate;
}

void CMainWindow::slot_SoundLocked()
{
    m_soundIsLocked = true;
    updateActions();
}

void CMainWindow::slot_SoundUnlocked()
{
    m_soundIsLocked = false;
    updateActions();
}

void CMainWindow::slot_PlayStarted(qint64 index)
{
    Q_UNUSED(index);
    ui->soundWidget->playCursor()->setIndex(index);
    updateActions();
}

void CMainWindow::slot_PlayStopped(qint64 index)
{
    Q_UNUSED(index);
    if (m_paused)
        ui->soundWidget->playCursor()->setIndex(index);
    else
        ui->soundWidget->playCursor()->reset();
    updateActions();
}

void CMainWindow::soundChanged(int channels, qint64 frames, double sampleRate)
{
    m_soundChannels = channels;
    m_soundFrames = frames;
    m_soundSampleRate = sampleRate;
}

void CMainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_sound.isNull())
        return;

    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Space)
    {
        on_action_PlayAll_triggered();
    }
    else if (event->key() == Qt::Key_Space)
    {
        if (g_soundPlayer->isPlayed())
            on_action_Stop_triggered();
        else
            on_action_Play_triggered();
    }

    if (event->key() == Qt::Key_Delete && !ui->soundWidget->selection()->isNull())
    {
        on_action_Delete_triggered();
    }
}

void CMainWindow::updateActions()
{
    if (m_sound.isNull())
    {
        ui->action_Close->setEnabled(false);
        ui->action_Save->setEnabled(false);
        ui->action_SaveAs->setEnabled(false);
        ui->action_SaveSelectionAs->setEnabled(false);
        ui->action_Delete->setEnabled(false);
        ui->action_Trim->setEnabled(false);
        ui->action_Normalize->setEnabled(false);
        ui->action_Loop->setEnabled(false);
        ui->action_Play->setEnabled(false);
        ui->action_PlayAll->setEnabled(false);
        ui->action_Pause->setEnabled(false);
        ui->action_Stop->setEnabled(false);
        ui->action_GoToEnd->setEnabled(false);
        ui->action_GoToStart->setEnabled(false);
        ui->action_FitInWindow->setEnabled(false);
        ui->action_ZoomIn->setEnabled(false);
        ui->action_ZoomOut->setEnabled(false);
        ui->action_ZoomToSelection->setEnabled(false);
    }
    else
    {
        ui->action_Close->setEnabled(true);
        ui->action_Save->setEnabled(true);
        ui->action_SaveAs->setEnabled(true);
        ui->action_SaveSelectionAs->setEnabled(!ui->soundWidget->selection()->isNull());
        ui->action_Delete->setEnabled(!ui->soundWidget->selection()->isNull());
        ui->action_Trim->setEnabled(!ui->soundWidget->selection()->isNull());
        ui->action_Normalize->setEnabled(true); //!ui->soundWidget->selection()->isNull());
        ui->action_Loop->setEnabled(true);
        ui->action_Play->setEnabled(true);
        ui->action_PlayAll->setEnabled(true);
        ui->action_Pause->setEnabled(g_soundPlayer->isPlayed() || m_paused);
        ui->action_Stop->setEnabled(g_soundPlayer->isPlayed() || m_paused);
        ui->action_GoToEnd->setEnabled(true);
        ui->action_GoToStart->setEnabled(true);
        ui->action_FitInWindow->setEnabled(true);
        ui->action_ZoomIn->setEnabled(true);
        ui->action_ZoomOut->setEnabled(true);
        ui->action_ZoomToSelection->setEnabled(!ui->soundWidget->selection()->isNull());
    }
}

void CMainWindow::on_action_PlayAll_triggered()
{
    m_paused = false;

    g_soundPlayer->setRange(0, m_soundFrames-1);
    g_soundPlayer->setIndex(0);
    if (!g_soundPlayer->isPlayed())
        g_soundPlayer->play();
}


void CMainWindow::on_action_Play_triggered()
{
    m_paused = false;

    if (ui->soundWidget->selection()->isNull())
        g_soundPlayer->setRange(0, m_sound->frames()-1);
    else
        g_soundPlayer->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());

    g_soundPlayer->setIndex(ui->soundWidget->positionCursor()->index());
    if (!g_soundPlayer->isPlayed())
        g_soundPlayer->play();
}

void CMainWindow::on_action_Pause_triggered()
{
    if (g_soundPlayer->isPlayed())
    {
        m_paused = true;
        g_soundPlayer->stop();
    }
    else
    {
        m_paused = false;
        g_soundPlayer->play();
    }
}

void CMainWindow::on_action_Stop_triggered()
{
    m_paused = false;
    ui->soundWidget->playCursor()->reset();
    g_soundPlayer->stop();
    updateActions();
}

void CMainWindow::on_action_GoToStart_triggered()
{
    ui->soundWidget->positionCursor()->setIndex(0);
}

void CMainWindow::on_action_GoToEnd_triggered()
{
    ui->soundWidget->positionCursor()->setIndex(m_soundFrames-1);
}

void CMainWindow::playFromPosition(qint64 index)
{
    m_paused = false;

    if (ui->soundWidget->selection()->isNull())
        g_soundPlayer->setRange(index, m_sound->frames()-1);
    else
        g_soundPlayer->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());

    g_soundPlayer->setIndex(index);
    if (!g_soundPlayer->isPlayed())
        g_soundPlayer->play();
}

void CMainWindow::slot_SelectionChanged()
{
    if (!ui->soundWidget->selection()->isNull())
        g_soundPlayer->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());
    else
        g_soundPlayer->setRange(0, m_soundFrames);
    updateActions();
}

void CMainWindow::on_action_Loop_triggered(bool checked)
{
    g_soundPlayer->setLoop(checked);
}

void CMainWindow::on_action_About_triggered()
{
    QMessageBox::about(this, G_FULL_PROGRAM_NAME(), G_PROGRAM_ABOUT());
}

void CMainWindow::on_action_AboutQt_triggered()
{
    qApp->aboutQt();
}

void CMainWindow::on_action_Quit_triggered()
{
    close();
}

void CMainWindow::on_action_Delete_triggered()
{
    g_soundPlayer->stop();

    CTaskDelete* taskDelete = new CTaskDelete;
    taskDelete->setSound(m_sound);
    taskDelete->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());
    connect(taskDelete, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskDelete, SIGNAL(done()), this, SLOT(hideProgressBar()));
    connect(taskDelete, SIGNAL(done()), this, SLOT(deleteSoundDone()));
    connect(taskDelete, SIGNAL(fail(QString)), this, SLOT(hideProgressBar()));
    connect(taskDelete, SIGNAL(fail(QString)), this, SLOT(deleteSoundFail(QString)));
    m_taskExecutor->execute(taskDelete);

    showProgressBar();
}

void CMainWindow::deleteSoundDone()
{
    ui->soundWidget->selection()->reset();
    updateActions();
}

void CMainWindow::deleteSoundFail(const QString &message)
{
    QMessageBox::warning(this, tr("Open"), tr("Can't delete the selected region: %2").arg(message));
    updateActions();
}

void CMainWindow::normalizeSoundDone()
{
    updateActions();
}

void CMainWindow::normalizeSoundFail(const QString &message)
{
    QMessageBox::warning(this, tr("Normalize"), tr("Can't normalize the selected region: %2").arg(message));
    updateActions();
}


void CMainWindow::on_action_Trim_triggered()
{
    g_soundPlayer->stop();

    CTaskTrim* taskTrim = new CTaskTrim;
    taskTrim->setSound(m_sound);
    taskTrim->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());
    connect(taskTrim, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskTrim, SIGNAL(done()), this, SLOT(hideProgressBar()));
    connect(taskTrim, SIGNAL(done()), this, SLOT(deleteSoundDone()));
    connect(taskTrim, SIGNAL(fail(QString)), this, SLOT(hideProgressBar()));
    connect(taskTrim, SIGNAL(fail(QString)), this, SLOT(deleteSoundFail(QString)));
    m_taskExecutor->execute(taskTrim);

    showProgressBar();
}


void CMainWindow::on_action_Normalize_triggered()
{
    bool ok;
    double normValue = QInputDialog::getDouble(this, tr("Normalize"), tr("Level:"), 0.995, 0.0, 1.0, 3, &ok);
    if (!ok)
        return;

    g_soundPlayer->stop();

    CTaskNormalize* taskNormalize = new CTaskNormalize;
    taskNormalize->setSound(m_sound);
    if (!ui->soundWidget->selection()->isNull())
        taskNormalize->setRange(ui->soundWidget->selection()->left(), ui->soundWidget->selection()->right());
    else
        taskNormalize->setRange(0, m_soundFrames-1);
    taskNormalize->setNormValue(normValue);
    connect(taskNormalize, SIGNAL(progress(double)), m_progressBar, SLOT(setValue(double)));
    connect(taskNormalize, SIGNAL(done()), this, SLOT(hideProgressBar()));
    connect(taskNormalize, SIGNAL(done()), this, SLOT(normalizeSoundDone()));
    connect(taskNormalize, SIGNAL(fail(QString)), this, SLOT(hideProgressBar()));
    connect(taskNormalize, SIGNAL(fail(QString)), this, SLOT(normalizeSoundFail(QString)));
    m_taskExecutor->execute(taskNormalize);

    showProgressBar();
}


void CMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->urls().count() == 1)
    {
        event->acceptProposedAction();
    }
}

void CMainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        foreach (QUrl url, event->mimeData()->urls())
        {
            if (url.isValid() && QFileInfo(url.toLocalFile()).isFile())
            {
                QString fileName = url.toLocalFile();
                slot_Open(fileName);
                break;
            }
        }
    }
}

void CMainWindow::on_action_ZoomIn_triggered()
{
}

void CMainWindow::on_action_ZoomOut_triggered()
{
}

void CMainWindow::on_action_FitInWindow_triggered()
{
}

void CMainWindow::on_action_ZoomToSelection_triggered()
{
}

void CMainWindow::on_action_Close_triggered()
{
}
