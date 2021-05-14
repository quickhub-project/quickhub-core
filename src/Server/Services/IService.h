/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef ISERVICE_H
#define ISERVICE_H

#include <QObject>
#include <QVariant>

class IService : public QObject
{

/*!
    \class IService
    \brief This class is the base class you need to derive to implement your own services.

    Services are functions, i.e. RPCs with arguments and return values, which are called by
    clients. Services generally map actions and can be used in many ways depending on the plugin
    implementation.

    After implementing your service, register it in the init function of your plugin with

    <code><pre>
    IService* service = new MyAwesomeService(this);
    ServiceManager::instance()->registerService(service);
    </pre></code>
*/
    Q_OBJECT


public:
    IService(QObject* parent = nullptr) : QObject(parent){}
    virtual ~IService(){}

    /*!
      Return the name of your service. This is the string with which the service can be addressed from the client.
    */
    virtual QString         getServiceName() = 0;

    /*!
        Return a string list that contains all supported calls.
    */
    virtual QStringList     getServiceCalls() = 0;

    /*!
        Here is where all the magic happens. Since all service call calls are processed completely asynchronously,
        cbID is a generated UUID that is used to reassign the return value of a call to the original call.

        <code><pre>
        bool MyAwesomeService::call(QString call, QString token, QString cbID, QVariant argument)
        {
            iUserPtr user = AuthenticationService::instance()->validateToken(token);
            if(!user)
            return false;


            if (call == "makeItDouble")
            {
                int input = argument["input"].toInt();
                QVariantMap answer;
                answer["result"] = input * 2;
                Q_EMIT response(cbID, answer);
                return true;
            }

            return false;
        }
        </pre></code>
    */
    virtual bool            call(QString call, QString token, QString cbID, QVariant argument = QVariant()) = 0;


signals:
    void response(QString uid, QVariant answer);

};


#endif // ISERVICE_H
