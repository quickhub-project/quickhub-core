/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include "qhcore_global.h"
#include <QDateTime>

class Logger
{

public:
    static void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        Q_UNUSED(context)
        QByteArray localMsg = msg.toLocal8Bit();
        QByteArray time = QDateTime::currentDateTime().toString("dd.MM. - hh:mm:ss:zzz").toLocal8Bit();

        switch (type)
        {
            case QtDebugMsg:

                fprintf(stderr, "[%s %s] -- %s\n" , "DEBUG   ", time.constData(), localMsg.constData());
                break;

            case QtInfoMsg:
                fprintf(stderr, "[%s %s] -- %s\n" , "INFO    ", time.constData(), localMsg.constData());
                break;

            case QtWarningMsg:
                fprintf(stderr, "[%s %s] -- %s\n" , "WARNING ", time.constData(), localMsg.constData());
                break;

            case QtCriticalMsg:
                fprintf(stderr, "[%s %s] -- %s\n" , "CRITICAL", time.constData(), localMsg.constData());
                break;

            case QtFatalMsg:
                fprintf(stderr, "[%s %s] -- %s\n" , "FATAL", time.constData(), localMsg.constData());
                break;
        }
    }
};

#endif // LOGGER_H
