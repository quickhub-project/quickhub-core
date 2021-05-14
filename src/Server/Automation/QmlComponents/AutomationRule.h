/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/

#ifndef AUTOMATIONRULE_H
#define AUTOMATIONRULE_H

#include <QObject>
#include <QQmlListProperty>

class AutomationRule : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString ruleName READ ruleName WRITE setRuleName NOTIFY ruleNameChanged)
    Q_PROPERTY(QQmlListProperty<QObject> children READ children)
    Q_CLASSINFO("DefaultProperty", "children")

public:
    explicit AutomationRule(QObject *parent = 0);

    QString ruleName() const;
    void setRuleName(const QString &ruleName);

    QQmlListProperty<QObject> children();

signals:
    void ruleNameChanged();

public slots:

private:
    QString _ruleName;
    QList<QObject*> _children;
};

#endif // AUTOMATIONRULE_H
