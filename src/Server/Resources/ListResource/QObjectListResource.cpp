#include "QObjectListResource.h"
#include <QDateTime>
#include <QUuid>

QObjectListResource::QObjectListResource(QObject *parent) : ListResource{nullptr, parent}
{
    setDynamicContent(false);
    setAllowUserAccess(false);
}

QObjectListResource::QObjectListResource(QList<QObject*> objects, QObject *parent)
    : QObjectListResource{ parent}
{
    QListIterator<QObject*> it(objects);
    while(it.hasNext())
    {
        appendObject(it.next());
    }
}

bool QObjectListResource::appendObject(QObject *object)
{
    if(_items.contains(object))
    {
        qWarning()<<Q_FUNC_INFO<<": Inserted object already exists." << _className;
        return false;
    }

    if(_initialized && object->metaObject()->className() != _className)
    {
        auto superClass = object->metaObject()->superClass();
        bool isTypeOf = false;
        while(superClass != nullptr){
            if(superClass->className() == _className){
                isTypeOf = true;
                break;
            }

            superClass = superClass->superClass();
        }

        if(!isTypeOf)
        {
            qWarning()<<Q_FUNC_INFO<<": Inserted object must be an instance of " << _className;
            return false;
        }
    }
	
    if(!object->property("uuid").isValid())
    {
        // setProperty can't be called on an object living in another thread
        QMetaObject::invokeMethod(object, [=](){object->setProperty("uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));});
    }
    _items.append(object);
    connectObject(object);
    Q_EMIT itemAppended(toVariant(object), iUserPtr());
    return true;
}

bool QObjectListResource::removeObject(QObject *object)
{
    int idx = _items.indexOf(object);
    if(idx < 0)
    {
        return false;
    }

    _items.removeAll(object);
    disconnectObject(object);
    Q_EMIT itemRemoved(idx, object->property("uuid").toString(), iUserPtr());
    return true;
}


int QObjectListResource::getCount() const
{
    return _items.count();
}

QVariantMap QObjectListResource::getMetadata() const
{
    return QVariantMap();
}

QVariantList QObjectListResource::getListData() const
{
    QVariantList list;
    QListIterator<QPointer<QObject>> it(_items);
    while(it.hasNext())
    {
        list << toVariant(it.next());
    }

    return list;
}

QVariant QObjectListResource::getItem(int idx, QString uuid) const
{

	QObject* object = getObject(idx, uuid);
	
    return toVariant(object);
}

IResource::ModificationResult QObjectListResource::setProperty(QString property, QVariant data, int index, QString uuid, QString token)
{
    ModificationResult result;
    if(!isPermittedToWrite(AuthenticationService::instance()->validateToken(token)))
    {
        result.error = IResource::PERMISSION_DENIED;
        return result;
    }

    if(index < 0 || index >= _items.count())
    {
        result.error = IResource::INVALID_PARAMETERS;
        return result;
    }

    QObject* object = _items.at(index);
    auto prop = _propertiesByName.value(property);
    if(!prop.isWritable())
    {
        result.error = IResource::NOT_SUPPORTED;
        return result;
    }

    if(prop.write(object, data))
    {
        QVariantMap map;
        map["lastupdate"] = QDateTime::currentDateTime();
        result.data = map;
        return result;
    }

    result.error = IResource::UNKNOWN_ERROR;
    return result;
}

QList<QObject *> QObjectListResource::getObjects() const
{
    QList<QObject *> objects;
    foreach (QPointer<QObject> item, _items) {
        objects << item;
    }
    return objects;
}

QObject *QObjectListResource::getObject(int idx, QString uuid) const
{
	if(idx < 0 || idx >= _items.count())
        return nullptr;

   QObject* item = _items.at(idx);
	if(!uuid.isEmpty())
	{
		 if(item->property("uuid") == uuid)
			 return item;
		 else 
			 return nullptr;
	}
	
	return item;
}

void QObjectListResource::setResourceProperties(QStringList properties)
{
    _resourceProperties = properties;
}

void QObjectListResource::init(QObject *firstObject)
{
    if(_initialized)
    {
        return;
    }

    _changedSlot = metaObject()->method(metaObject()->indexOfSlot("objectPropertyChanged()"));
    const auto* metaObject = firstObject->metaObject();
    if(_className.isEmpty())
    {
        _className = metaObject->className();
    }

    for(int i = 1; i < metaObject->propertyCount(); i++)
    {
        auto property = metaObject->property(i);
        QString name = property.name();
        if(_resourceProperties.isEmpty() || _resourceProperties.contains(name))
        {
            _propertiesByIndex.insert(property.notifySignalIndex(),property);
            _propertiesByName.insert(name, property);
        }
        _initialized = true;
    }
}

void QObjectListResource::connectObject(QObject *object)
{
    if(!_initialized)
    {
        init(object);
    }

    connect(object, &QObject::destroyed, this, &QObjectListResource::objectDestroyed, Qt::UniqueConnection);
    QMapIterator<QString, QMetaProperty> it(_propertiesByName);
    while(it.hasNext())
    {
        auto property = it.next();
        if(property.value().hasNotifySignal())
        {
            QObject::connect(object, property->notifySignal(), this, _changedSlot, Qt::UniqueConnection);
        }
    }
}

void QObjectListResource::disconnectObject(QObject *object)
{
    if(!_initialized)
    {
        return;
    }

    disconnect(object, &QObject::destroyed, this, &QObjectListResource::objectDestroyed);
    QMapIterator<QString, QMetaProperty> it(_propertiesByName);
    while(it.hasNext())
    {
        auto property = it.next();
        if(property.value().hasNotifySignal())
        {
            QObject::disconnect(object, property->notifySignal(), this, _changedSlot);
        }
    }
}

void QObjectListResource::setClassName(const QString &newClassName)
{
    _className = newClassName;
}


QVariantMap QObjectListResource::toVariant(QObject *object) const
{
    if(object == nullptr)
        return QVariantMap();

    QVariantMap variant;
    QMapIterator<QString, QMetaProperty> it(_propertiesByName);
    while(it.hasNext())
    {
        auto property = it.next();
        if(!property->isValid())
            break;

        QVariant value = property->read(object);
        if(value.metaType().flags() & QMetaType::IsEnumeration)
            value = value.toInt();

        variant[property->name()] = value;
    }

    QVariantMap item;
    item["data"] = variant;
    item["uuid"] = object->property("uuid");
    return item;
}

void QObjectListResource::objectPropertyChanged()
{
    int idx = senderSignalIndex();
    QMetaProperty property = _propertiesByIndex.value(idx);
    auto* object = sender();
    if(!property.isValid() || nullptr == object)
        return;

    QString name = property.name();
    QVariant value = property.read(object);
    if(value.metaType().flags() & QMetaType::IsEnumeration)
        value = value.toInt();

    int index = _items.indexOf(object);
    QString uuid = object->property("uuid").toString();
    Q_EMIT propertySet(name, value, index, uuid, iUserPtr(), QDateTime::currentMSecsSinceEpoch());
}

void QObjectListResource::objectDestroyed(QObject *object)
{
    int idx = _items.indexOf(object);
    if(idx < 0)
    {
        return;
    }

    _items.removeAll(object);
    Q_EMIT itemRemoved(idx, object->property("uuid").toString(), iUserPtr());
}
