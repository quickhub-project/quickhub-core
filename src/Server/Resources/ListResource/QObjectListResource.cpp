#include "QObjectListResource.h"

#include <QDateTime>
#include <QLoggingCategory>
#include <QUuid>

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

Q_LOGGING_CATEGORY(lcQObjectListResource, "quickhub.resource.qobjectlist")

// ----------------------------------------------------------------------------
// Ctors
// ----------------------------------------------------------------------------

/*!
    Constructs an empty QObjectListResource.
    Dynamic content and user access are disabled by default.
*/
QObjectListResource::QObjectListResource(QObject *parent) : ListResource{nullptr, parent}
{
    setDynamicContent(false);
    setAllowUserAccess(false);
}

/*!
    Constructs a QObjectListResource pre-populated with the given \a objects.
*/
QObjectListResource::QObjectListResource(QList<QObject*> objects, QObject *parent)
    : QObjectListResource{ parent}
{
    QListIterator<QObject*> it(objects);
    while(it.hasNext())
    {
        appendObject(it.next());
    }
}

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------

/*!
    Appends \a object to the list resource. The object must not already be
    contained and, once initialized, must match (or inherit from) the class
    of the first inserted object. A UUID is assigned automatically if the
    object does not already carry one.
    Returns \c false if the object was rejected.
*/
bool QObjectListResource::appendObject(QObject *object)
{
    if(_items.contains(object))
    {
        qCWarning(lcQObjectListResource) << "Object already exists, ignoring append."
                                          << "class:" << _className;
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
            qCWarning(lcQObjectListResource) << "Type mismatch, object must inherit from"
                                              << _className
                                              << "but is:" << object->metaObject()->className();
            return false;
        }
    }

    if(!object->property("uuid").isValid())
    {
        // setProperty can't be called on an object living in another thread
        QMetaObject::invokeMethod(object, [=](){object->setProperty("uuid", QUuid::createUuid().toString(QUuid::WithoutBraces));});
    }
    _items.append(object);
    _rawPtrs.append(object);
    connectObject(object);

    qCDebug(lcQObjectListResource) << "Object appended."
                                    << "class:" << object->metaObject()->className()
                                    << "count:" << _items.count();

    Q_EMIT itemAppended(toVariant(object), iUserPtr());
    return true;
}

/*!
    Removes \a object from the list resource and disconnects its
    change-tracking signals. Returns \c false if the object was not found.
*/
bool QObjectListResource::removeObject(QObject *object)
{
    int idx = _items.indexOf(object);
    if(idx < 0)
    {
        qCWarning(lcQObjectListResource) << "Object not found, cannot remove."
                                          << "class:" << object->metaObject()->className();
        return false;
    }

    _items.removeAll(object);
    _rawPtrs.removeAll(object);
    disconnectObject(object);

    qCDebug(lcQObjectListResource) << "Object removed."
                                    << "index:" << idx
                                    << "uuid:" << object->property("uuid").toString()
                                    << "count:" << _items.count();

    Q_EMIT itemRemoved(idx, object->property("uuid").toString(), iUserPtr());
    return true;
}

/*!
    Returns the number of objects in the list.
*/
int QObjectListResource::getCount() const
{
    return _items.count();
}

/*!
    Returns an empty metadata map (not used for QObject-based lists).
*/
QVariantMap QObjectListResource::getMetadata() const
{
    return QVariantMap();
}

/*!
    Serializes all objects in the list to a QVariantList.
*/
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

/*!
    Returns the serialized representation of the item at \a idx.
    If \a uuid is set it must match the item's UUID.
*/
QVariant QObjectListResource::getItem(int idx, QString uuid) const
{
	QObject* object = getObject(idx, uuid);

    return toVariant(object);
}

/*!
    Writes a single \a property on the item at \a index.
    The caller must provide a valid authentication \a token.
    Returns a ModificationResult with the appropriate error code.
*/
IResource::ModificationResult QObjectListResource::setProperty(QString property, QVariant data, int index, QString uuid, QString token)
{
    ModificationResult result;
    if(!isPermittedToWrite(AuthenticationService::instance()->validateToken(token)))
    {
        qCWarning(lcQObjectListResource) << "Permission denied for setProperty."
                                          << "property:" << property
                                          << "index:" << index;
        result.error = IResource::PERMISSION_DENIED;
        return result;
    }

    if(index < 0 || index >= _items.count())
    {
        qCWarning(lcQObjectListResource) << "Invalid index for setProperty."
                                          << "index:" << index
                                          << "count:" << _items.count();
        result.error = IResource::INVALID_PARAMETERS;
        return result;
    }

    QObject* object = _items.at(index);
    auto prop = _propertiesByName.value(property);
    if(!prop.isWritable())
    {
        qCWarning(lcQObjectListResource) << "Property is not writable."
                                          << "property:" << property;
        result.error = IResource::NOT_SUPPORTED;
        return result;
    }

    if(prop.write(object, data))
    {
        qCDebug(lcQObjectListResource) << "Property written."
                                        << "property:" << property
                                        << "index:" << index;
        QVariantMap map;
        map["lastupdate"] = QDateTime::currentDateTime();
        result.data = map;
        return result;
    }

    qCWarning(lcQObjectListResource) << "Failed to write property."
                                      << "property:" << property
                                      << "index:" << index;
    result.error = IResource::UNKNOWN_ERROR;
    return result;
}

/*!
    Returns a plain QList of all tracked QObject pointers.
*/
QList<QObject *> QObjectListResource::getObjects() const
{
    QList<QObject *> objects;
    foreach (QPointer<QObject> item, _items) {
        objects << item;
    }
    return objects;
}

/*!
    Returns the QObject at \a idx. If \a uuid is non-empty, returns the
    object only when its UUID matches; otherwise returns \c nullptr.
*/
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

/*!
    Sets an explicit list of property names to expose. If empty, all
    Q_PROPERTY entries of the tracked class are used.
*/
void QObjectListResource::setResourceProperties(QStringList properties)
{
    _resourceProperties = properties;

    qCDebug(lcQObjectListResource) << "Resource properties set."
                                    << "count:" << properties.size()
                                    << "properties:" << properties;
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

/*!
    Initializes the property maps from the meta-object of \a firstObject.
    Called automatically when the first object is connected.
*/
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

    qCInfo(lcQObjectListResource) << "Initialized."
                                   << "class:" << _className
                                   << "trackedProperties:" << _propertiesByName.size();
}

// ----------------------------------------------------------------------------
// Signal connections
// ----------------------------------------------------------------------------

/*!
    Connects all notify signals of \a object to the change-tracking slot.
    Triggers init() on the first call.
*/
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

/*!
    Disconnects all notify signals of \a object from this resource.
*/
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

/*!
    Overrides the auto-detected class name with \a newClassName.
*/
void QObjectListResource::setClassName(const QString &newClassName)
{
    _className = newClassName;

    qCDebug(lcQObjectListResource) << "Class name set."
                                    << "className:" << newClassName;
}

// ----------------------------------------------------------------------------
// Serialization
// ----------------------------------------------------------------------------

/*!
    Converts \a object to a QVariantMap containing all tracked property
    values plus the object's UUID. Returns an empty map for null objects.
*/
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

// ----------------------------------------------------------------------------
// Change tracking
// ----------------------------------------------------------------------------

/*!
    Slot invoked when any tracked property's notify signal fires.
    Identifies the property via sender signal index and emits propertySet.
*/
void QObjectListResource::objectPropertyChanged()
{
    int idx = senderSignalIndex();
    QMetaProperty property = _propertiesByIndex.value(idx);
    auto* object = sender();
    if(!property.isValid() || nullptr == object)
    {
        qCWarning(lcQObjectListResource) << "Property change ignored: invalid property or null sender."
                                          << "signalIndex:" << idx;
        return;
    }

    QString name = property.name();
    QVariant value = property.read(object);
    if(value.metaType().flags() & QMetaType::IsEnumeration)
        value = value.toInt();

    int index = _items.indexOf(object);
    QString uuid = object->property("uuid").toString();

    qCDebug(lcQObjectListResource) << "Property changed."
                                    << "property:" << name
                                    << "index:" << index
                                    << "uuid:" << uuid;

    Q_EMIT propertySet(name, value, index, uuid, iUserPtr(), QDateTime::currentMSecsSinceEpoch());
}

/*!
    Called when a tracked QObject is destroyed. Removes it from the
    internal lists and emits itemRemoved.
*/
void QObjectListResource::objectDestroyed(QObject *object)
{
    int idx = _rawPtrs.indexOf(object);
    if(idx < 0)
    {
        qCWarning(lcQObjectListResource) << "Destroyed object not found in raw pointer list.";
        return;
    }

    qCInfo(lcQObjectListResource) << "Tracked object destroyed."
                                   << "index:" << idx
                                   << "count:" << (_items.count() - 1);

    _items.removeAt(idx);
    _rawPtrs.removeAt(idx);
    Q_EMIT itemRemoved(idx, object->property("uuid").toString(), iUserPtr());
}
