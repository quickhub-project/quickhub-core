#ifndef SHAREDQOBJECTLISTRESOURCE_H
#define SHAREDQOBJECTLISTRESOURCE_H

#include "QObjectListResource.h"
#include <QObject>

class SharedQObjectListResource : public QObjectListResource
{
    Q_OBJECT
public:
    explicit SharedQObjectListResource(QObject* parent = nullptr);
    bool                                    appendSharedObject(const QSharedPointer<QObject> &object);
    bool                                    removeSharedObject(const QSharedPointer<QObject> &object);
    QSharedPointer<QObject>                 getSharedObject(int idx, QString uuid = "") const;
    const QList<QSharedPointer<QObject>> &  getSharedObjects() const;

private:
    QList<QSharedPointer<QObject>>  _sharedObjects;
    using QObjectListResource::appendObject;
    using QObjectListResource::removeObject;
    using QObjectListResource::getObject;
    using QObjectListResource::getObjects;

};

#endif // SHAREDQOBJECTLISTRESOURCE_H
