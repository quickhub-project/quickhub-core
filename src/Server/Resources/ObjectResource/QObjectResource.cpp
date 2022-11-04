#include "QObjectResource.h"
#include <QMetaProperty>
#include <QDateTime>

QObjectResource::QObjectResource(QObject* object, QObject* parent) : ObjectResource(nullptr, parent),
    _object(object)
{
    setDynamicContent(false);
    initObject(_object);
}

bool QObjectResource::initObject(QObject *object)
{
    if(_initialized)
        return false;

    _changedSlot = metaObject()->method(metaObject()->indexOfSlot("objectPropertyChanged()"));
    auto metaObject = object->metaObject();
    _className = metaObject->className();
    for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++)
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

IResource::ModificationResult QObjectResource::setProperty(QString name, const QVariant &value, QString token)
{

    ModificationResult result;
    if(!_initialized || nullptr == _object )
    {
        result.error = IResource::UNKNOWN_ITEM;
        return result;
    }

    if(!isPermittedToWrite(token))
    {
        result.error = IResource::PERMISSION_DENIED;
        return result;
    }

    auto prop = _propertiesByName.value(name);
    if(!prop.isWritable())
    {
        result.error = IResource::NOT_SUPPORTED;
        return result;
    }

    if(prop.write(_object, value))
    {
        result.data = value;
        return result;
    }

    result.error = IResource::UNKNOWN_ERROR;
    return result;
}

QVariantMap QObjectResource::getObjectData() const
{
    if(nullptr == _object)
            return QVariantMap();

    QVariantMap objectData;
    QMap<QString, QVariant> data = toVariant(_object);
    QMapIterator<QString, QVariant> it(data);
    {
        while(it.hasNext())
        {
            it.next();
            QVariantMap value;
            value.insert("data",it.value());
            objectData.insert(it.key(), value);
        }
    }

    return objectData;
}


QVariantMap QObjectResource::toVariant(QObject *object) const
{
    if(!_initialized)
        return QVariantMap();

    QVariantMap variant;
    QMapIterator<QString, QMetaProperty> it(_propertiesByName);
    while(it.hasNext())
    {
        auto property = it.next();
        if(!property->isValid())
            break;

        variant[property->name()] = property->read(object);
    }

    return variant;
}

void QObjectResource::objectPropertyChanged()
{
    int idx = senderSignalIndex();
    QMetaProperty property = _propertiesByIndex.value(idx);
    auto object = sender();
    if(!property.isValid() || nullptr == object)
        return;

    QString name = property.name();
    QVariant value = property.read(object);

    Q_EMIT propertyChanged(name, value, iUserPtr());
}


