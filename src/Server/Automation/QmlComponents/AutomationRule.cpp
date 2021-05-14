/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/
#include "AutomationRule.h"

AutomationRule::AutomationRule(QObject *parent) : QObject(parent)
{

}

QString AutomationRule::ruleName() const
{
    return _ruleName;
}

void AutomationRule::setRuleName(const QString &ruleName)
{
    _ruleName = ruleName;
    Q_EMIT ruleNameChanged();
}

QQmlListProperty<QObject> AutomationRule::children()
{
    return QQmlListProperty<QObject>(this, _children);
}
