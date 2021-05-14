/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/

#ifndef QUICKENGINE_H
#define QUICKENGINE_H

#include <QObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QHash>

class AutomationRule;
class AutomationEngine : public QObject
{
    Q_OBJECT

public:
    explicit AutomationEngine(QObject *parent = 0);
    QObject* instanciateRule(QString qmlCode);

private:
    QQmlEngine*                     _engine;
    QHash<QString, AutomationRule*> _rules;

signals:

public slots:
};

#endif // QUICKENGINE_H
