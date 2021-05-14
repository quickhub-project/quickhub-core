#ifndef DEVICEQMLADAPTER_H
/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/

#define DEVICEQMLADAPTER_H

#include <QObject>
#include <QQmlPropertyMap>
#include <QQmlListProperty>
#include "../../Devices/DeviceManager.h"
#include "qhcore_global.h"

class COREPLUGINSHARED_EXPORT DeviceQmlAdapter : public QQmlPropertyMap
{
    Q_OBJECT

    Q_PROPERTY(QString mapping READ mapping WRITE setMapping NOTIFY deviceMappingChanged)
    Q_PROPERTY(QString online READ online NOTIFY onlineChanged)
    Q_PROPERTY(QQmlListProperty<QObject> children READ children)
    Q_CLASSINFO("DefaultProperty", "children")

public:
    explicit DeviceQmlAdapter(QObject *parent = 0);

    QString mapping() const;
    void setMapping(const QString &mapping);
    Q_INVOKABLE void call(QString function, QVariant parameters = QVariant());
    QQmlListProperty<QObject> children();
    bool online() const;

private:
    QString         _mapping;
    deviceHandlePtr _deviceHandle;
    bool            _initialized;
    bool            registerDevice(QString deviceMapping);
    bool            _online;
    QList<QObject*> _children;

protected:
    QVariant updateValue(const QString &key, const QVariant &input) override;

signals:
    void deviceMappingChanged();
    void onlineChanged();
    void dataReceived(QString uuid, QString subject, QVariantMap data);

private slots:
    void propertyChangedSlot(QString uuid, QString property, QVariant value, bool dirty);
    void deviceStateChangedSlot(QString uuid, IDevice::DeviceState state);
    void initHandle();

};

#endif // DEVICEQMLADAPTER_H
