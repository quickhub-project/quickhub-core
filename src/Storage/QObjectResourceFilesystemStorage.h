#ifndef QOBJECTRESOURCEFILESYSTEMSTORAGE_H
#define QOBJECTRESOURCEFILESYSTEMSTORAGE_H

#include "ObjectResourceFilesystemStorage.h"
#include <QMetaMethod>

class QObjectResourceFilesystemStorage : public ObjectResourceFilesystemStorage
{
Q_OBJECT
public:
    explicit            QObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *object) ;
    explicit            QObjectResourceFilesystemStorage(QString qualifiedResourceName, QObject *object, QObject *parent);
    virtual bool        serializeQObject(QObject* object);
    virtual bool        deserializeQObject(QObject* object);
    bool                registerQObject(QObject *object);


private:
    bool _initialized = false;
    QMap<int, QMetaProperty>        _propertiesByIndex;
    QMap<QString, QMetaProperty>    _propertiesByName;
    QObject*                        _object = nullptr;;
    QMetaMethod                     _changedSlot;
    QString                         _className;

private slots:
    void objectPropertyChanged();

signals:
    void initComplete();

};

#endif // QOBJECTRESOURCEFILESYSTEMSTORAGE_H
