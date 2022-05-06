#include "SettingsResource.h"
#include "Server/Resources/ObjectResource/IObjectResourceStorage.h"
#include <QDateTime>

SettingsResource::SettingsResource(IObjectResourceStorage* storage, QObject *parent)
    : ObjectResource(storage, parent)
{
}

bool SettingsResource::isPermittedToRead(QString token) const
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    return !user.isNull() && (user->isAuthorizedTo(IS_ADMIN) || _publiblyReadable);
}


void  SettingsResource::setProperty(QString name, const QVariant &value)
{
    QVariantMap data;
    data["data"] = value;
    data["lastupdate"] = QDateTime::currentMSecsSinceEpoch();
    _mutex.lockForWrite();
    _lastAccess = QDateTime::currentMSecsSinceEpoch();
    _storage->insertProperty(name, data);
    Q_EMIT propertyChanged(name, value, nullptr);
    _mutex.unlock();
}

void SettingsResource::setPubliclyReadable(bool readable)
{
    _publiblyReadable = readable;
}

bool SettingsResource::isPermittedToWrite(QString token) const
{
    iIdentityPtr user = AuthenticationService::instance()->validateToken(token);
    if(user.isNull())
        return false;

    return user->isAuthorizedTo(IS_ADMIN);
}
