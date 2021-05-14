/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef DEVICEUPDATELOGIC_H
#define DEVICEUPDATELOGIC_H

#include <QObject>
#include <QNetworkAccessManager>

class DeviceUpdateLogic : public QObject
{
    Q_OBJECT

    /*!
        \class DeviceUpdateLogic
        \brief This class wraps the functionylity to check for and trigger new updates for a distinct device.
        \ingroup devices

        This class encapsulates the logic to check the availability of new updates.
        For this purpose an environment variable can be used to specify a web server base URL that provides
        the appropriate information via a simple JSON file. The environment variable is:

        FIRMWARE_UPDATE_LOOKUP

        The URL for the query is resolved as follows:
        FIRMWARE_UPDATE_LOOKUP / <device type> / version.json

        The version.json must be a JSON text file with a "version" field and an "url" field. The value of
        version needs to be a string consisting of the major version number and a minor version number separated by a dot.

        JSON Example:
        {
            "version": "12.3",  // <Major>.<Minor>
            "url": "path-to-firmware.binary"
        }

        To be able to compare version numbers an integer is generated. According to the formula:
        major * 1000 + minor.If the resulting value is greater than the firmware version, you know that a
        new firmware version is available.
        This class is primarily used by DeviceService and separates the functionality from the service interface.

        \sa DeviceHandleHandler DeviceManager
    */
public:
    enum UpdateCheckResult
    {
        PERMISSION_DENIED=-3,
        CHECK_FAILED =-2,
        DEVICE_NOT_AVAILABLE=-1,
        UP_TO_DATE=0,
        UPDATE_AVAILABLE=1,
        CMD_UPDATE_SENT=2,
    };

    explicit    DeviceUpdateLogic(QObject *parent = nullptr);

    /*!
        \fn bool checkForUpdates(QString mapping, QString cbID);
        Returns true when there is is a newer version available for the given device address.
        See the class description to find out how the check works.
    */
    bool        checkForUpdates(QString mapping, QString cbID);

    /*!
        Tells the Device to update the firmware.
        This function is asynchronous.
        - token must be the token of a logged in user with admin permissions
        - mapping is the QuickHub internal address of the device
        - url must be a path to a valid firmware binary. The Device will try to download the firmware from there.
        - The callbackID (cbID) allows to assign the response of the call to the call.
        \sa sendResult(QString cbID, QVariant result)
    */
    bool        startUpdate(QString token, QString mapping, QString url, QString cbID);

private:
    QNetworkReply *startLookup(QString mapping);
    QNetworkAccessManager*  _nam = nullptr;
    QString                 _firmwareLookup;

private slots:
    void lookupFinished();

signals:
    void sendResult(QString cbID, QVariant result);

};

#endif // DEVICEUPDATELOGIC_H
