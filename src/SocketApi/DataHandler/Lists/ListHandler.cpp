/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#include "ListHandler.h"
#include "IList.h"

ListHandler::ListHandler(IList *list, QObject *parent) : IResourceHandler("list", parent),
    _list(list)
{
    QObject::connect(_list,SIGNAL(itemAdded(QVariant,int)), this, SLOT(itemAdded(QVariant,int)));
    QObject::connect(_list, SIGNAL(itemRemoved(int)), this, SLOT(itemRemoved(int)));
    QObject::connect(_list,SIGNAL(itemChanged(QVariant,int)), this, SLOT(itemChanged(QVariant,int)));
    QObject::connect(_list, SIGNAL(propertyChanged(QString,QVariant,int)), this, SLOT(propertyChanged(QString,QVariant,int)));
}

void ListHandler::handleMessage(QVariant message, ISocket *handle)
{
   _list->handleMessage(message, handle);
}

void ListHandler::initHandle(ISocket *handle)
{
    QVariantMap msg;
    msg["command"] = "list:dump";
    QVariantMap parameters;
    parameters["data"] = _list->getListData();
    msg["parameters"] = parameters;
    handle->sendVariant(msg);
}

bool ListHandler::isPermitted(QString token) const
{
    return _list->isPermitted(token);
}

void ListHandler::propertyChanged(QString property, QVariant data, int index)
{
    QVariantMap msg;
    msg["command"] = "list:property:set";
    QVariantMap parameters;
    parameters["index"] = index;
    parameters["property"] = property;
    parameters["data"] = data;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void ListHandler::itemChanged(QVariant item, int index)
{
    QVariantMap msg;
    msg["command"] = "list:set";
    QVariantMap parameters;
    parameters["index"] = index;
    parameters["data"] = item;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void ListHandler::itemAdded(QVariant item, int index)
{
    QVariantMap msg;
    msg["command"] = "list:insertat";
    QVariantMap parameters;
    parameters["index"] = index;
    parameters["data"] = item;
    msg["parameters"] = parameters;
    deployToAll(msg);
}

void ListHandler::itemRemoved(int index)
{
    QVariantMap msg;
    msg["command"] = "list:remove";
    QVariantMap parameters;
    parameters["index"] = index;
    msg["parameters"] = parameters;
    deployToAll(msg);
}
