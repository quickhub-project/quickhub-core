#include "SharedQObjectListResource.h"

SharedQObjectListResource::SharedQObjectListResource(QObject* parent)
    : QObjectListResource{parent}
{
}


bool SharedQObjectListResource::appendSharedObject(const QSharedPointer<QObject>& object)
{
    if(appendObject(object.data())){
        _sharedObjects << object;
        return true;
    }
    return false;
}
bool SharedQObjectListResource::removeSharedObject(const QSharedPointer<QObject>& object)
{
    if((_sharedObjects.removeAll(object) > 0) && removeObject(object.data()))
    {
        return true;
    }
    return false;
}

QSharedPointer<QObject> SharedQObjectListResource::getSharedObject(int idx, QString uuid) const
{
    QObject* object = getObject(idx, uuid);
    if(object && _sharedObjects[idx].data() == object){
        return _sharedObjects[idx];
    }
    return nullptr;
}

const QList<QSharedPointer<QObject>> &SharedQObjectListResource::getSharedObjects() const
{
    return _sharedObjects;
}
