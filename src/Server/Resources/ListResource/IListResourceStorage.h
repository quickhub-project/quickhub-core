/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef ILISTRESOURCESTORAGE_H
#define ILISTRESOURCESTORAGE_H

#include <QObject>
#include "qhcore_global.h"

class COREPLUGINSHARED_EXPORT IListResourceStorage : public QObject
{
    Q_OBJECT

    /*!
        \class IListResourceStorage
        \brief This class encapsulates the interface to the actual list storage (both temporary and persistent).
        In the default implementation of the resources provided by QuickHub the derived
        class ListResourceFileSystemStorage is used.

        This class can be derived to provide alternative storages. For example, if you want
        the resources provided by QuickHub to be loaded to or from a database.

        In addition to the derivation the class IListResourceStorageFactory must be implemented.
        If such a factory is located in the ObjectPool, the ListResourceFactory will use the
        thereby provided storages

        \sa IListResourceStorageFactory, ListResourceFileSystemStorage, ListResourceFactory
    */
public:

    /*!
        \struct ItemUID
        This type is a tuple of uuid and index position.
        It is used to uniquely identify a list entry.
        If a client is not up to date because of a lost
        message the comparison between index position and
        item UID can reveal this error. If UUID and index do
        not match, the operation is applied to the item with
        the matching UUID.
    */
    struct ItemUID
    {
        QString uuid;
        int index;
    };

    explicit IListResourceStorage(QObject* parent = nullptr) : QObject(parent){}

    virtual ~IListResourceStorage(){}

    /*------ modifier */

   /*!
         \fn bool IListResourceStorage::appendItem(QVariant data)
         Appends an item at the end of the list.
         The argument is typically used as a QVariantMap.
    */
    virtual bool appendItem(QVariant data) = 0;

    /*!
          \fn bool IListResourceStorage::insertAt(QVariant data, ItemUID item) = 0;
          Inserts an item at the given position / before the Item with the given UUID.
          The argument data is typically used as a QVariantMap.
     */
    virtual bool insertAt(QVariant data, ItemUID item) = 0;

    /*!
          \fn bool IListResourceStorage::appendList(QVariantList data);
          Appends an list at the end of the list.
          The argument data is typically used as a QList<QVariantMap>
     */
    virtual bool appendList(QVariantList data) = 0;

    /*!
          \fn bool IListResourceStorage::removeItem(ItemUID item) = 0;
          Removes the item at the given position / the Item with the given UUID.
     */
    virtual bool removeItem(ItemUID item) = 0;

    /*!
          \fn bool IListResourceStorage::deleteList()
          Deletes the list. (Should also delete its storage representation [file, database entry,...]).
    */
    virtual bool deleteList() = 0;

    /*!
          \fn bool IListResourceStorage::clearList() = 0;
          Removes all entries from the list.
    */
    virtual bool clearList() = 0;

    /*!
          \fn bool IListResourceStorage::set(QVariant data, ItemUID item)
          Overwrites the item at the given index / with the given uuid
    */
    virtual bool set(QVariant data, ItemUID item) = 0;

    /*!
          \fn bool IListResourceStorage::setProperty(QString property, QVariant data, ItemUID item) = 0;
          Sets the property value of the item at the given index / with the given uuid to <data>.
    */
    virtual bool setProperty(QString property, QVariant data, ItemUID item) = 0;

    /*!
        \fn bool IListResourceStorage::sync() = 0; // to write unsaved
        Write unsaved changes. In some situations it is better to write down all changes directly.
        Especially if the derived class communicates with databases and is in a larger context with other entities.
    */
    virtual bool sync() = 0; // to write unsaved

    // stores an extra metadata object
    // the user can add aditional data which is related to the list content.
    // e.g. data description

    virtual bool    setMetadata(QVariant metadata) = 0;


    /*------   getter */

    /*!
        \fn QVariantList  IListResourceStorage::getList() const = 0
        Returns the complete list. (A QList<QVariantMap> is expected behind the scenes)
    */
    virtual QVariantList    getList() const = 0;

    /*!
        \fn QVariant  IListResourceStorage::getItem(ItemUID uid) const = 0;
        Returns the item at the given index / with the given uuid. (A QVariantMap is expected behind the scenes)
    */
    virtual QVariant        getItem(ItemUID uid) const = 0;

    /*!
        \fn QVariant IListResourceStorage::getMetadata() const = 0;
        Returns the metadata object.
    */
    virtual QVariant        getMetadata() const = 0;

    /*!
        \fn int IListResourceStorage::getCount() const = 0;
        Returns the number of list entries
    */
    virtual int             getCount() const = 0;

    /*!
        \fn bool IListResourceStorage::isReady() const = 0;
        Returns true when the storage is ready to use.
    */
    virtual bool            isReady() const = 0;

signals:
    void ready(bool ready);

};

#endif // ILISTRESOURCESTORAGE_H
