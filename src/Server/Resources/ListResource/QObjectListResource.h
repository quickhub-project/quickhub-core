#ifndef QOBJECTLISTRESOURCE_H
#define QOBJECTLISTRESOURCE_H

#include <QObject>
#include <QMetaProperty>
#include "ListResource.h"

class QObjectListResource : public ListResource
{
    Q_OBJECT


public:
    explicit QObjectListResource(QObject *parent = nullptr);
    explicit QObjectListResource(QList<QObject *> objects, QObject *parent = nullptr);

    bool                            appendObject(QObject* object);
    bool                            removeObject(QObject* object);
    virtual int                     getCount() const override;
    virtual QVariantMap             getMetadata() const override;
    virtual QVariantList            getListData() const override;
    QVariant                        getItem(int idx, QString uuid = "") const override;
    virtual ModificationResult      setProperty(QString property, QVariant data, int index, QString uuid, QString token) override;
    QList<QObject*>                 getObjects() const;
	QObject*						getObject(int idx, QString uuid = "") const;

protected: 
    QVariantMap                     toVariant(QObject* object) const;

private:
    void                            init(QObject* firstObject);
    void                            connectObject(QObject* object);
    void                            disconnectObject(QObject* object);

    bool                            _initialized = false;
    QList<QObject*>                 _items;
    QMap<int, QMetaProperty>        _propertiesByIndex;
    QMap<QString, QMetaProperty>    _propertiesByName;
    QMetaMethod                     _changedSlot;
    QString                         _className;

private slots:
    void objectPropertyChanged();
    void objectDestroyed(QObject *object);

signals:


};

#endif // QOBJECTLISTRESOURCE_H
