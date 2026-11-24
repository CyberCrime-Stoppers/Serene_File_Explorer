#pragma once
/*
 * Serene File Explorer - transfer engine (QThread worker pool)
 * Queued copy/move with per-item progress, overall progress, pause/resume/cancel.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QObject>
#include <QQueue>
#include <QAtomicInt>

#include "core/types.h"

struct TransferItem {
    QString srcPath;
    QString dstPath;
    bool    isMove  = false;
    qint64  totalBytes = 0;
};

class TransferManager : public QObject
{
    Q_OBJECT
public:
    enum State { Idle, Running, Paused, Failed, Done };

    explicit TransferManager(QObject *parent = nullptr);

    void enqueueCopy(const QStringList &paths, const QString &destDir);
    void enqueueMove(const QStringList &paths, const QString &destDir);

    void pause();
    void resume();
    void cancel();

    State state() const { return m_state; }

signals:
    void started(int totalJobs);
    void itemProgress(qint64 copiedBytes);
    void itemFinished(int index);
    void finished(bool cancelled);
    void error(const QString &message);

private:
    void pump();

    QQueue<TransferItem> m_queue;
    QAtomicInt m_pause;
    QAtomicInt m_cancel;
    State      m_state = Idle;
};
