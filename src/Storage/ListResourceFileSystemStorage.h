/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef LISTRESOURCEFILESYSTEMSTORAGE_H
#define LISTRESOURCEFILESYSTEMSTORAGE_H

#include <QObject>
#include <QVariant>
#include <QFile>
#include <QFileInfo>

#include "../Server/Resources/ListResource/IListResourceStorage.h"

class ListResourceFileSystemStorage : public IListResourceStorage
{
    Q_OBJECT

    /*!
        \class ListResourceFileSystemStorage
        \brief This class implements the QuickHub default list storage.
        This class persists List in the form of JSON files on disk.
        \sa IListResourceStorageFactory, IListResourceStorage, ListResourceFactory
    */

public:
    explicit ListResourceFileSystemStorage(QString path, QObject* parent = nullptr);
    bool appendItem(QVariant data) override;
    bool insertAt(QVariant data, ItemUID item) override;
    bool appendList(QVariantList data) override;
    bool removeItem(ItemUID item) override;
    bool deleteList() override;
    bool clearList() override;
    bool set(QVariant data, ItemUID item) override;
    bool setProperty(QString property, QVariant data, ItemUID item) override;
    bool sync() override; // to write unsaved

    // stores an extra metadata object
    // the user can add aditional data which is related to the list content.
    // e.g. data description

    bool    setMetadata(QVariant metadata) override;

    /*------   getter */

    QVariantList    getList() const override;
    QVariant        getItem(ItemUID uid) const override;
    QVariant        getMetadata() const override;
    int             getCount() const override;
    bool            isReady() const override;

private:
    int checkAndCorrectIndex(ItemUID uid) const;
    bool save();
    void load();

    QFile           _file;
    QString         _qualifiedResourceName;
    QVariantList    _listData;
    QVariantMap     _metadata;
};

#endif // LISTRESOURCEFILESYSTEMSTORAGE_H
