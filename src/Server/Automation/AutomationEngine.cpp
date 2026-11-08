/* Copyright (C) Friedemann Metzger - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Friedemann Metzger <friedemann.metzger@gmx.de>, 2017
*/

#include "AutomationEngine.h"

AutomationEngine::AutomationEngine(QObject *parent) : QObject(parent)
{
    _engine = new QQmlEngine(this);
}

QObject* AutomationEngine::instanciateRule(QString qmlCode)
{
    Q_UNUSED(qmlCode)
    return nullptr;
}
