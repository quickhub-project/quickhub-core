#include "QObjectResourceFilesystemStorage.h"
#include "QObjectSerializer.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

Q_LOGGING_CATEGORY(lcQObjectStorage, "quickhub.storage.qobject")

// ----------------------------------------------------------------------------
// Ctors
// ----------------------------------------------------------------------------

/*!
    Constructs a QObjectResourceFilesystemStorage for the given \a qualifiedResourceName.
    The \a object is registered asynchronously via a single-shot timer so that
    the caller's constructor chain can finish first.
*/
QObjectResourceFilesystemStorage::QObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *object)
    : ObjectResourceFilesystemStorage{qualifiedResourceName, nullptr}
{
    QTimer::singleShot(0, this, [this, object]() { registerQObject(object); });
}

/*!
    Overloaded constructor that additionally accepts a \a parent QObject.
*/
QObjectResourceFilesystemStorage::QObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *object, QObject *parent)
    : ObjectResourceFilesystemStorage{qualifiedResourceName, parent}
{
    QTimer::singleShot(0, this, [this, object]() { registerQObject(object); });
}

// ----------------------------------------------------------------------------
// Serialization
// ----------------------------------------------------------------------------

/*!
    Serializes all Q_PROPERTY values of \a object into the internal property
    map and persists them to disk.
    Returns \c true on success.
*/
bool QObjectResourceFilesystemStorage::serializeQObject(QObject *object)
{
    _propertyData = QtPropertySerializer::serialize(object);

    qCDebug(lcQObjectStorage) << "Serialized QObject."
                               << "class:" << object->metaObject()->className()
                               << "propertyCount:" << _propertyData.size();
    return save();
}

/*!
    Deserializes the persisted property data back onto \a object.
    Returns \c false if no property data is available.
*/
bool QObjectResourceFilesystemStorage::deserializeQObject(QObject *object)
{
    if (_propertyData.isEmpty()) {
        qCDebug(lcQObjectStorage) << "No property data to deserialize."
                                   << "class:" << object->metaObject()->className();
        return false;
    }

    QtPropertySerializer::deserialize(object, _propertyData);

    qCDebug(lcQObjectStorage) << "Deserialized QObject."
                               << "class:" << object->metaObject()->className()
                               << "propertyCount:" << _propertyData.size();
    return true;
}

// ----------------------------------------------------------------------------
// Registration
// ----------------------------------------------------------------------------

/*!
    Registers \a object for automatic property change tracking.

    Iterates over all Q_PROPERTY entries of the object, connects each
    notify signal to objectPropertyChanged(), and restores previously
    persisted values via deserializeQObject().

    Returns \c false if the object was already registered.
*/
bool QObjectResourceFilesystemStorage::registerQObject(QObject *object)
{
    if (_initialized) {
        qCDebug(lcQObjectStorage) << "Object already registered, ignoring."
                                     << "class:" << _className;
        return false;
    }

    _object = object;
    _changedSlot = metaObject()->method(metaObject()->indexOfSlot("objectPropertyChanged()"));

    auto metaObject = object->metaObject();
    _className = metaObject->className();

    // Collect all properties (skip index 0 which is QObject::objectName).
    for (int i = 1; i < metaObject->propertyCount(); i++)
    {
        auto property = metaObject->property(i);
        _propertiesByIndex.insert(property.notifySignalIndex(), property);
        _propertiesByName.insert(property.name(), property);
    }

    // Connect every property's notify signal to our change-tracking slot.
    QMapIterator<QString, QMetaProperty> it(_propertiesByName);
    while (it.hasNext())
    {
        auto property = it.next();
        if (property.value().hasNotifySignal())
        {
            QObject::connect(object, property->notifySignal(), this, _changedSlot);
        }
    }

    // Cleanup when the tracked object is destroyed.
    connect(object, &QObject::destroyed, _object, [this]() {
        qCInfo(lcQObjectStorage) << "Tracked object destroyed, resetting."
                                  << "class:" << _className;
        _object = nullptr;
        _initialized = false;
    });

    _initialized = true;

    qCInfo(lcQObjectStorage) << "Object registered."
                              << "class:" << _className
                              << "properties:" << _propertiesByName.size();

    // Restore persisted property values onto the object.
    deserializeQObject(_object);
    Q_EMIT initComplete();
    return true;
}

// ----------------------------------------------------------------------------
// Change tracking
// ----------------------------------------------------------------------------

/*!
    Slot invoked whenever a tracked property's notify signal fires.
    Identifies the changed property via the sender signal index,
    reads the new value, and persists it to disk.
*/
void QObjectResourceFilesystemStorage::objectPropertyChanged()
{
    int idx = senderSignalIndex();
    QMetaProperty property = _propertiesByIndex.value(idx);
    auto *object = sender();

    if (!property.isValid() || nullptr == object)
    {
        qCWarning(lcQObjectStorage) << "Property change ignored: invalid property or null sender."
                                     << "signalIndex:" << idx;
        return;
    }

    QString name = property.name();
    QVariant value = property.read(object);

    qCDebug(lcQObjectStorage) << "Property changed."
                               << "class:" << _className
                               << "property:" << name
                               << "value:" << value;

    insertProperty(name, value);
    save();
}
