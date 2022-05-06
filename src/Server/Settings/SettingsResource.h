#ifndef SETTINGSOBJECTRESOURCE_H
#define SETTINGSOBJECTRESOURCE_H

#include "Server/Resources/ObjectResource/ObjectResource.h"
#include <QObject>

class SettingsResource : public ObjectResource
{
    Q_OBJECT

public:
    explicit SettingsResource(IObjectResourceStorage* storage, QObject *parent = nullptr);
    virtual bool isPermittedToRead(QString token) const override;
    virtual bool isPermittedToWrite(QString token) const override;
    void setProperty(QString name, const QVariant &value) ;
    void setPubliclyReadable(bool readable);

private:
    bool _publiblyReadable = false;

signals:

};

#endif // SETTINGSOBJECTRESOURCE_H
