/*
 * Copyright 2020  David Edmundson <davidedmundson@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pamhandle.h"
#include <QDebug>
#include <QEventLoop>
#include <security/pam_appl.h>

class PamWorker : public QObject
{
    Q_OBJECT
public:
    PamWorker();
    ~PamWorker();
    void start(const QString &service, const QString &user);
    void authenticate();
    void changePassword();

    static int converse(int n, const struct pam_message **msg, struct pam_response **resp, void *data);
Q_SIGNALS:
    void prompt(const QString &prompt);
    void finished(bool success);

    // internal
    void promptReceived(const QByteArray &prompt);
    void cancelled();

private:
    pam_handle_t *m_handle = nullptr; //< the actual PAM handle
    struct pam_conv m_conv;

    int m_result;
};

int PamWorker::converse(int n, const struct pam_message **msg, struct pam_response **resp, void *data)
{
    PamWorker *c = static_cast<PamWorker *>(data);

    *resp = (struct pam_response *)calloc(n, sizeof(struct pam_response));
    if (!*resp) {
        return PAM_BUF_ERR;
    }

    for (int i = 0; i < n; i++) {
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON: {
            const QString prompt = QString::fromLocal8Bit(msg[i]->msg);
            emit c->prompt(prompt);
            qDebug() << "new request" << prompt;

            QByteArray response;
            bool cancelled = false;
            QEventLoop e;
            QObject::connect(c, &PamWorker::promptReceived, &e, [&](const QByteArray &_response) {
                response = _response;
                e.quit();
            });
            QObject::connect(c, &PamWorker::cancelled, &e, [&]() {
                cancelled = true;
                e.quit();
            });

            e.exec();
            if (cancelled) {
                return PAM_CONV_ERR;
            }

            resp[i]->resp = (char *)malloc(response.length() + 1);
            // on error, get rid of everything
            if (!resp[i]->resp) {
                for (int j = 0; j < n; j++) {
                    free(resp[i]->resp);
                    resp[i]->resp = nullptr;
                }
                free(*resp);
                *resp = nullptr;
                return PAM_BUF_ERR;
            }

            memcpy(resp[i]->resp, response.constData(), response.length());
            resp[i]->resp[response.length()] = '\0';

            break;
        }
        case PAM_ERROR_MSG:
            qDebug() << QString::fromLocal8Bit(msg[i]->msg);
            break;
        case PAM_TEXT_INFO:
            // if there's only the info message, let's predict the prompts too
            qDebug() << QString::fromLocal8Bit(msg[i]->msg);
            break;
        default:
            break;
        }
    }

    qDebug() << "DONE";

    return PAM_SUCCESS;
}

PamWorker::PamWorker()
    : QObject(nullptr)
{
    m_conv = {&PamWorker::converse, this};
}

PamWorker::~PamWorker()
{
    if (m_handle) {
        pam_end(m_handle, PAM_SUCCESS);
    }
}

void PamWorker::authenticate()
{
    qDebug() << "Start auth";
    int rc = pam_authenticate(m_handle, PAM_SILENT);
    qDebug() << "Auth done RC" << rc;

    bool success = rc == PAM_SUCCESS;
    emit finished(success);
}

void PamWorker::start(const QString &service, const QString &user)
{
    if (user.isEmpty())
        m_result = pam_start(qPrintable(service), nullptr, &m_conv, &m_handle);
    else
        m_result = pam_start(qPrintable(service), qPrintable(user), &m_conv, &m_handle);

    if (m_result != PAM_SUCCESS) {
        qWarning() << "[PAM] start" << pam_strerror(m_handle, m_result);
        return;
    } else {
        qDebug() << "[PAM] Starting...";
    }
}

PamJob::PamJob()
    : QObject(nullptr)
{
    d = new PamWorker;

    d->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished, d, &QObject::deleteLater);
    connect(d, &PamWorker::prompt, this, &PamJob::prompt);
    connect(d, &PamWorker::finished, this, &PamJob::finished);

    m_thread.start();
}

PamJob::~PamJob()
{
    cancel();
    m_thread.quit();
    m_thread.wait();
}

void PamJob::init(const QString &service, const QString &user)
{
    QMetaObject::invokeMethod(d, [this, service, user]() {
        d->start(service, user);
    });
}

void PamJob::authenticate()
{
    QMetaObject::invokeMethod(d, &PamWorker::authenticate);
}

void PamJob::changePassword()
{
    QMetaObject::invokeMethod(d, &PamWorker::changePassword);
}

void PamJob::respond(const QByteArray &response)
{
    QMetaObject::invokeMethod(
        d,
        [this, response]() {
            emit d->promptReceived(response);
        },
        Qt::QueuedConnection);
}

void PamJob::cancel()
{
    QMetaObject::invokeMethod(d, &PamWorker::cancelled);
}

AuthenticateUserJob::AuthenticateUserJob(const QString &service, const QString &user)
    : PamJob()
{
    init(service, user);
}

void AuthenticateUserJob::start()
{
    authenticate();
}

#include "pamhandle.moc"
