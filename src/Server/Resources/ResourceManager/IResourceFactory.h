/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */


#ifndef IRESOURCEFACTORY_H
#define IRESOURCEFACTORY_H
#include <QDebug>
#include <QObject>
#include "../../Authentication/AuthentificationService.h"
#include "../../Resources/ResourceManager/ResourceManager.h"
#include "IResource.h"
#include "qhcore_global.h"
#include <QJsonDocument>

class COREPLUGINSHARED_EXPORT IResourceFactory : public QObject
{
    Q_OBJECT
    friend class ResourceManager;

    /*!
        \class IResourceFactory
        \ingroup Resources
        \brief This class manages the life cycle of resources and is the interface between the QuickHub resource system and the resources provided by your plugin.
        This class must be subclassed and implemented for each resource your plugin provides. An instance of this class must be registered at the ResourceManager.
        Everytime a resource is requested by a client, the resource manager will look for factories that are able to provide an instance of the appropriate resource.

        The developer of a resource can decide whether the instance of a resource is kept or released after use.
        Resources are always managed internally with SmartPointern. Accordingly, the release is done automatically as soon
        as the last client has detached itself from the resource. Alternatively, the factory can keep its own SmartPointer and always
        return it instead of creating a new instance each time.

        \sa ResourceManager
    */
public:
    //ctor
    IResourceFactory(QObject* parent = nullptr):QObject(parent){}
    //dtor
    virtual ~IResourceFactory(){}

    /*!
        \fn QString IResourceFactory::getResourceID(QString descriptor, QString token = "")
        Normally this function does not need to be overwritten.  This function ensures that the Device Manager can distinguish
        between resources that differ despite having the same descriptors

        For example, "home/myList" will contain different data depending on who wants to access the resource.
        In order for the ResourceManager to be able to distinguish between these resources, a unique string (some kind of uuid) must
        be generated from the descriptor. (this is what generateQualifiedResourceName will do in the default implementation)

        You can overwrite the behavior to make sure that different resources for the same descriptor can be distinguished.
    */
    virtual QString getResourceID(QString descriptor, QString token = "") const
    {
        return generateQualifiedResourceName(descriptor, token);
    }

    /*!
        \fn QString IResourceFactory::getResourceType() const = 0;
        Return the type of your resource. QuickHub core supports
        "synclist"  for list resources
        "object"    for key value pairs
        "imgcoll"   for imagecollections

        Possibly other plugins are used, which provide access to additional resource types
    */
    virtual QString getResourceType() const = 0;

    /*!
        \fn  QString IResourceFactory::getDescriptorPrefix()
        The descriptor prefix allows the device manager to find the right factory for a requested resource.
        When a client requests a resource of a certain type, the Device Manager searches for a factory that is able
        to provide an instance of this resource.  Assuming its resource provides access to user objects. Then a reasonable
        prefix would be "myplugin/users".

        A request for the resource "myplugin/users/#4711" would result in the corresponding resource factory being called.
        In consequence, the createResource() function would be called with the appropriate descriptor.
    */
    virtual QString getDescriptorPrefix() const {return "";}

private:
    /*!
        \fn resourcePtr IResourceFactory::createResource( QString descriptor, QString token ="", QObject* parent = nullptr)
        This is the most important function. It is responsible for returning an instance to the appropriate resource. It's okay
        to return a nullpointer when the parameters are invalid or the resource can't be found.
    */
    virtual resourcePtr createResource( QString descriptor, QString token ="", QObject* parent = nullptr) = 0;

public:
    //TODO: refactor this weird constrution
    virtual QString generateQualifiedResourceName(QString descriptor, iIdentityPtr  user) const
    {
        descriptor = descriptor.replace(".","/");
        QStringList tokens = descriptor.split("/",  SKIP_EMPTY_PARTS);
        if(tokens[0] == "home" && !user.isNull())
        {
            tokens.insert(1, user->identityID());
        }

        descriptor = tokens.join("/");
        return descriptor +"_"+getResourceType();
    }

    virtual QString generateQualifiedResourceName(QString descriptor, QString token) const
    {
        iIdentityPtr  user = AuthenticationService::instance()->validateToken(token);
        return generateQualifiedResourceName(descriptor, user);
    }

    static QVariantMap parseParameters(QString descriptor)
    {
        QVariantMap propertieMap;
        //resource: my/fancyresource?sortby=name&secondparam=foo
        if(descriptor.contains("?"))
        {
            QString args =  descriptor.split("?").at(1);
            QStringList properties = args.split("&") ;
            for(int i = 0; i < properties.count(); i++)
            {
                QStringList propTokens = properties.at(i).split("=");
                if(propTokens.count()!=2)
                    continue;

                propertieMap.insert(propTokens.at(0),propTokens.at(1));
            }
        }
        //resource: my/fancyresource:{"sortby":"name"; "secondparam":"foo"}
        else if(descriptor.contains(":"))
        {
            int firstIndex = descriptor.indexOf(":");
            QString json = descriptor.remove(0,firstIndex+1 );
            propertieMap = QJsonDocument::fromJson(json.toUtf8()).toVariant().toMap();
        }
        return propertieMap;
    }
};

#endif // IRESOURCEFACTORY_H

