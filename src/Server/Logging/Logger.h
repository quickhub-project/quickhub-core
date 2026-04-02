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
#include <cstdio>

class Logger
{
public:
    static void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        const QByteArray time     = QDateTime::currentDateTime()
        .toString("dd.MM. hh:mm:ss.zzz")
            .toLocal8Bit();
        const QByteArray message  = msg.toLocal8Bit();
        const QByteArray category = (context.category && qstrlen(context.category) > 0
                                     && qstrcmp(context.category, "default") != 0)
                                        ? QByteArray("[") + context.category + "] "
                                        : QByteArray();

               // ANSI color codes
        const char* reset  = "\033[0m";
        const char* bold   = "\033[1m";

        const char* color;
        const char* label;

        switch (type)
        {
        case QtDebugMsg:    color = "\033[36m";    label = "DEBUG   "; break;  // Cyan
        case QtInfoMsg:     color = "\033[32m";    label = "INFO    "; break;  // Green
        case QtWarningMsg:  color = "\033[33m";    label = "WARNING "; break;  // Yellow
        case QtCriticalMsg: color = "\033[31m";    label = "CRITICAL"; break;  // Red
        case QtFatalMsg:    color = "\033[35m";    label = "FATAL   "; break;  // Magenta
        default:            color = reset;          label = "UNKNOWN "; break;
        }


        const QByteArray rawCat = (context.category && qstrlen(context.category) > 0
                                   && qstrcmp(context.category, "default") != 0)
                                      ? QByteArray(context.category)
                                      : QByteArray("default");

        fprintf(stderr, "%s%s%s %s| %-30s | %s%s\n",
                color, bold, label,
                time.constData(),
                rawCat.constData(),   // %-20s paddet rechts mit Leerzeichen
                message.constData(),
                reset);

        // fprintf(stderr, "%s%s%s %s| %s%s|%s %s%s\n",
        //         color, bold, label,
        //         time.constData(),
        //         color, category.constData(),
        //         reset,
        //         message.constData(),
        //         reset);
    }
};

#endif // LOGGER_H