/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*********************************************************************/
#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QObject>

class QSocketNotifier;
class QTimer;
class KCheckPass;

enum class AuthenticationMode {
    Delayed,
    Direct
};

class Authenticator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool graceLocked READ isGraceLocked NOTIFY graceLockedChanged)
public:
    explicit Authenticator(AuthenticationMode mode = AuthenticationMode::Direct, QObject *parent = nullptr);
    ~Authenticator() override;

    bool isGraceLocked() const;

public Q_SLOTS:
    void tryUnlock(const QString &password);

Q_SIGNALS:
    void failed();
    void succeeded();
    void graceLockedChanged();
    void message(const QString & msg); // don't remove the "msg" param, used in QML!!!
    void error(const QString & err); // don't remove the "err" param, used in QML!!!

private:
    void setupCheckPass();
    QTimer *m_graceLockTimer;
    KCheckPass *m_checkPass;
};

class KCheckPass : public QObject
{
    Q_OBJECT
public:
    explicit KCheckPass(AuthenticationMode mode, QObject *parent = nullptr);
    ~KCheckPass() override;

    void start();

    bool isReady() const {
        return m_ready;
    }

    void setPassword(const QString &password) {
        m_password = password;
    }

    void startAuth();

Q_SIGNALS:
    void failed();
    void succeeded();
    void message(const QString &);
    void error(const QString &);

private Q_SLOTS:
    void handleVerify();

private:
    void cantCheck();
    void reapVerify();
    // kcheckpass interface
    int Reader(void *buf, int count);
    bool GRead(void *buf, int count);
    bool GWrite(const void *buf, int count);
    bool GSendInt(int val);
    bool GSendStr(const char *buf);
    bool GSendArr(int len, const char *buf);
    bool GRecvInt(int *val);
    bool GRecvArr(char **buf);

    QString m_password;
    QSocketNotifier *m_notifier;
    int m_pid;
    int m_fd;
    bool m_ready = false;
    AuthenticationMode m_mode;
};

#endif
