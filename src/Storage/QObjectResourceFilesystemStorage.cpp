#include "QObjectResourceFilesystemStorage.h"
#include "QObjectSerializer.h"
#include <QTimer>
#include <QDebug>

QObjectResourceFilesystemStorage::QObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *object) : ObjectResourceFilesystemStorage{qualifiedResourceName, nullptr}
{
    QTimer::singleShot(1, this, [this, object](){registerQObject(object);});
}

QObjectResourceFilesystemStorage::QObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *object, QObject *parent)
    : ObjectResourceFilesystemStorage{qualifiedResourceName, parent}
{
    QTimer::singleShot(1 , this, [this, object](){registerQObject(object);});
}

bool QObjectResourceFilesystemStorage::serializeQObject(QObject *object)
{
    _propertyData = QtPropertySerializer::serialize(object);
    return save();
}

bool QObjectResourceFilesystemStorage::deserializeQObject(QObject *object)
{
    if(_propertyData.isEmpty()){
        return false;
    }
    QtPropertySerializer::deserialize(object, _propertyData);
    return true;
}

bool QObjectResourceFilesystemStorage::registerQObject(QObject *object)
{
    if(_initialized)
        return false;

    _object = object;
    deserializeQObject(_object);
    Q_EMIT initComplete();
    _changedSlot = metaObject()->method(metaObject()->indexOfSlot("objectPropertyChanged()"));
    auto metaObject = object->metaObject();
    _className = metaObject->className();
    for(int i = 1; i < metaObject->propertyCount(); i++)
    {
        auto property = metaObject->property(i);
        _propertiesByIndex.insert(property.notifySignalIndex(),property);
        _propertiesByName.insert(property.name(), property);
    }

    QMapIterator<QString, QMetaProperty> it(_propertiesByName);
    while(it.hasNext())
    {
        auto property = it.next();
        if(property.value().hasNotifySignal())
        {
            QObject::connect(object, property->notifySignal(), this, _changedSlot);
        }
    }

    connect(object, &QObject::destroyed, _object, [this](){ _object = nullptr; _initialized = false;});
    _initialized = true;
    return true;
}

void QObjectResourceFilesystemStorage::objectPropertyChanged()
{
    int idx = senderSignalIndex();
    QMetaProperty property = _propertiesByIndex.value(idx);
    auto* object = sender();
    if(!property.isValid() || nullptr == object)
        return;

    QString name = property.name();
    QVariant value = property.read(object);

    insertProperty(name, value);
    save();
}
