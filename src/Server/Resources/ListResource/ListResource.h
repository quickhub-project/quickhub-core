/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef LISTRESOURCE_H
#define LISTRESOURCE_H

#include <QObject>
#include <QVariant>
#include <QFile>
#include <QReadWriteLock>

#include "../ResourceManager/IResource.h"
#include "../../Authentication/User.h"
#include "ListResourceFactory.h"

/*!
    \class ListResource
    \brief This class is the one and only ListResource.

    The ListResource implements the interface and logic for the
    synchronization of list-based data. From this class can be derived to
    add own list implementations via plugin. For a custom ListResource you
    need an additional IListResourceFactory implementation.

    The individual modifiers of this class are available twice. Once in a public
    and once in a protected variant. Override the public functions and use the
    protected API to change the behavior of the list.

    Use ListResourceTemporaryStorage for list implementations loaded from own data
    structures or databases.
*/


class User;
class IListResourceStorage;
class ResourceManager;
class ListResource : public IResource
{
    Q_OBJECT

    friend class ListResourceFactory;

protected:
    explicit ListResource(IListResourceStorage* storage, QObject *parent = nullptr);

public:
    ~ListResource() override;

    /*!
        \fn qint64 ListResource::lastAccess() const override
        Returns a unix timestamp of the last user access.
    */
    qint64                      lastAccess() const override;

    /*!
        \fn QString ListResource::getResourceType() const override
        Returns the resource type as string.
    */
    QString const               getResourceType() const override;

    /*!
        \fn virtual QVariantList ListResource::getListData() const;
        Returns the complete list.
        \note QVariantList -> QList<QVariantMap>
    */
    virtual QVariantList        getListData() const;

    /*!
        \fn QVariant ListResource::getItem(int idx, QString uuid = "") const;
        Returns the item at the given index / with the given uuid
    */
    virtual QVariant            getItem(int idx, QString uuid = "") const;

    /*!
        \fn QVariantMap ListResource::getMetadata() const;
        Returns the metadata-blob of the list. This can contain additional information about the list data
    */
    virtual QVariantMap         getMetadata() const;

    /*!
       \fn int ListResource::getCount() const
       Returns the number of items in the list.
    */
    virtual int                 getCount() const;

    // ########   Modifier -- public API. Overwrite these functions to customize the behavior of the list

    /*!
        \fn ModificationResult ListResource::appendItem(QVariant data, QString token);
        Appends an item at the end of the list.
        \sa IResource::ModificationResult
        \note QVariant -> QVariantMap
    */
    virtual ModificationResult appendItem(QVariant data, QString token);

    /*!
        \fn ModificationResult ListResource::insertAt(QVariant data, int index, QString token)
        Insert the object at the given index position.

        \sa IResource::ModificationResult
        \note QVariant -> QVariantMap
    */
    virtual ModificationResult insertAt(QVariant data, int index, QString token);

    /*!
        \fn ModificationResult ListResource::appendList(QVariantList data, QString token)
        Append the list at the end of the list.

        \sa IResource::ModificationResult
        \note QVariantList -> QList<QVariantMap>
    */
    virtual ModificationResult appendList(QVariantList data, QString token);

    /*!
        \fn ModificationResult ListResource::removeItem(QString uuid, QString token, int index = -1);
        Removes and deletes the item with the given uuid. If the resource doesn't provide object uuids
        the index can be used to identify the object to be deleted.

        \sa IResource::ModificationResult
    */
    virtual ModificationResult removeItem(QString uuid, QString token, int index = -1);

    /*!
        \fn ModificationResult ListResource::deleteList(QString token);
        Deletes the list. This will result in the complete list including the persistence being deleted.
        \note If you only want to delete the items in the list without deleting the list itself use clearList() instead.

        \sa ListResource::clearList(QString token), IResource::ModificationResult
    */
    virtual ModificationResult deleteList(QString token);

    /*!
        \fn ModificationResult ListResource::clearList(QString token);
        This will remove and delete all entries in the list. The list itself and the metadata are preserved.
        \note If you  want to delete the whole list use deleteList() instead.

        \sa ListResource::deleteList(QString token), IResource::ModificationResult
    */
    virtual ModificationResult clearList(QString token);

    /*!
        \fn ModificationResult ListResource::set(QVariant data, int index, QString uuid, QString token);
        This will replace and overwrite the item at the given position / with the given uuid.
        \note QVariant -> QVariantMap, If you only want to modify one property of a object use setProperty() instead.
        \sa setProperty(QString property, QVariant data, int index, QString uuid, QString token), IResource::ModificationResult
    */

    virtual ModificationResult set(QVariant data, int index, QString uuid, QString token);
    /*!
      \fn ModificationResult ListResource::setProperty(QString property, QVariant data, int index, QString uuid, QString token)
      Sets a specific property of the list entry with the given index / the given uuid to data.
      \sa IResource::ModificationResult
    */
    virtual ModificationResult setProperty(QString property, QVariant data, int index, QString uuid, QString token);

    /*!
      \fn ModificationResult ListResource::setMetadata(QVariant metadata)
      Sets the metadata for the list. This is a QVariantMap which can contain generic meta information about the list.
      \sa IResource::ModificationResult
    */
    virtual ModificationResult setMetadata(QVariant metadata);

    /*!
      \fn bool ListResource::setFilter(QVariantMap query)
      Applies a filter to the list. This function is not implemented by this base class. It can be overwritten to provide
      filtering query/features. The idea is to make it possible to query data that meet certain properties

      \note Don't forget to call IRresource::setDynamicContent(true) in the constructor of your derived list class when you
      provide filtering.
      \sa IResource::setDynamicContent(bool enabled)
    */
    virtual bool               setFilter(QVariantMap query);

    /*!
        \fn bool ListResource::isPermittedToRead(QString token) const
        Overwrite this function and return false when the appropriate user is not permitted to read the data
        in this resource. If false is returned
    */
    virtual bool               isPermittedToRead(iIdentityPtr identity) const;

    /*!
        \fn bool ListResource::isPermittedToWrite(QString token) const
        Overwrite this function and return false when the appropriate user is not permitted to modify / write the data
        in this resource. You should 
        \note This function is currently not yet used. It is not checked at any point and has no effect at the moment!
    */
    virtual bool               isPermittedToWrite(iIdentityPtr identity) const;

signals:
    void itemAppended(QVariant data, iIdentityPtr user);
    void itemInserted(QVariant data, int index, iIdentityPtr user);
    void listAppended(QVariantList data, iIdentityPtr user);
    void itemRemoved(int index, QString uuid, iIdentityPtr user);
    void listDeleted(iIdentityPtr user);
    void listCleared(iIdentityPtr user);
    void itemSet(QVariant data, int index, QString uuid, iIdentityPtr user);
    void propertySet(QString property, QVariant data, int index, QString uuid, iIdentityPtr user, qint64 timestamp);
    void metadataChanged();
    void reset();

private:
    QVariantMap     prepareTemplate(iIdentityPtr user) const;
    QString         createUUID() const;
    bool                    _allowUserAccess = true;
    qint64                  _lastAccess;
    IListResourceStorage*   _listStorage;

protected:
    mutable QReadWriteLock  _mutex;
    ModificationResult appendItem(QVariant data, iIdentityPtr user = iIdentityPtr(nullptr));
    ModificationResult insertAt(QVariant data, int index, iIdentityPtr user = iIdentityPtr(nullptr));
    ModificationResult appendList(QVariantList data, iIdentityPtr user = iIdentityPtr(nullptr));
    void resetData(QVariantList data, iIdentityPtr user = iIdentityPtr(nullptr));
    ModificationResult removeItem(int index, iIdentityPtr user = iIdentityPtr(nullptr), QString uuid = "");
    ModificationResult deleteList(iIdentityPtr user = iIdentityPtr(nullptr));
    ModificationResult clearList(iIdentityPtr user = iIdentityPtr(nullptr));
    ModificationResult set(QVariant data, int index, iIdentityPtr user = iIdentityPtr(nullptr),  QString uuid = "");
    ModificationResult setProperty(QString property, QVariant data, int index, iIdentityPtr user = iIdentityPtr(nullptr), QString uuid = "");

    void setStorage(IListResourceStorage* storage);
    void setAllowUserAccess(bool enabled);

public slots:
};

#endif // LISTRESOURCE_H
