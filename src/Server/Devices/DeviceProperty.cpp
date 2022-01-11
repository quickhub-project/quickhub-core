/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "DeviceProperty.h"
#include <QDateTime>
#include <QDebug>
#include <QReadLocker>
#include <QWriteLocker>


DeviceProperty::DeviceProperty(QString name, DeviceHandle *parent, QVariantMap metadata) : QObject(parent),
    _name(name)
{
    if(!metadata.isEmpty())
    {
        _mutex.lockForWrite();
        _realValue              = metadata["val"];
        _setValue               = metadata["setVal"];
        _timestamp              = metadata["timestamp"].toLongLong();
        _dirty                  = metadata["dirty"].toBool();
        _metadata               = metadata["metadata"].toMap();
        _mutex.unlock();
        _mutex.lockForRead();
        Q_EMIT setValueChanged(_name, _setValue, _dirty);
        Q_EMIT realValueChanged(_name, _realValue, _dirty, _timestamp);
        _mutex.unlock();
    }
}

QVariant DeviceProperty::getValue() const
{
     QReadLocker locker(&_mutex);
        return _dirty ? _setValue : _realValue;
}

QVariant DeviceProperty::getRealValue() const
{
    QReadLocker locker(&_mutex);
    return _realValue;
}

void DeviceProperty::setRealValue(const QVariant &realValue,  bool keepDirtyFlag)
{
    if(!keepDirtyFlag)
        setDirty(false, realValue == _setValue);

    _mutex.lockForWrite();
    _realValue = realValue;
    _timestamp = QDateTime::currentMSecsSinceEpoch();
    _mutex.unlock();
    _mutex.lockForRead();
    Q_EMIT realValueChanged(_name, realValue, _dirty, _timestamp);
    _mutex.unlock();
}

void DeviceProperty::setDirty(bool dirty, bool accepted)
{
    _mutex.lockForWrite();
    _dirty = dirty;
    _mutex.unlock();
    _mutex.lockForRead();
    Q_EMIT dirtyChanged(_name, dirty);
    _mutex.unlock();

    if(!dirty)
    {
        _mutex.lockForRead();
        Q_EMIT confirmed(_name,_timestamp, accepted);
        _mutex.unlock();
    }
}

DeviceProperty::~DeviceProperty()
{
}


QVariantMap DeviceProperty::toMap() const
{
    QVariantMap metadata;
    _mutex.lockForRead();
    metadata["name"]     = _name;
    metadata["val"]      = _realValue;
    metadata["setVal"]   = _setValue;
    metadata["timestamp"]  = _timestamp;
    metadata["dirty"]    = _dirty;
    metadata["metadata"] = _metadata;
    _mutex.unlock();
    return metadata;
}

QVariant DeviceProperty::getSetValue() const
{
    QReadLocker locker(&_mutex);
    return _setValue;
}

QString DeviceProperty::getName() const
{
    QReadLocker locker(&_mutex);
    return _name;
}

void DeviceProperty::setValue(const QVariant &setValue)
{
    setDirty(true, false);
    _mutex.lockForWrite();
    _setValue = setValue;
    _mutex.unlock();
    _mutex.lockForRead();
    Q_EMIT setValueChanged(_name, setValue, true);
    _mutex.unlock();
}

void DeviceProperty::setMetadata(QString key, QVariant value)
{
    _mutex.lockForWrite();
    _metadata[key] = value;
    _mutex.unlock();
    _mutex.lockForRead();
    Q_EMIT metadataChanged(_name, key, value);
    _mutex.unlock();
}

bool DeviceProperty::isDirty() const
{
    QReadLocker locker(&_mutex);
    return _dirty;
}

qlonglong DeviceProperty::confirmedTimestamp() const
{
    QReadLocker locker(&_mutex);
    return _timestamp;
}

