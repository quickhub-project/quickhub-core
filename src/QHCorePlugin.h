/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef QHCOREPLUGIN_H
#define QHCOREPLUGIN_H

#include "QObject"
#include <IPlugin.h>

class QHCorePlugin : public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid FILE "corePlugin.json")
    Q_INTERFACES(IPlugin)

public:
    QHCorePlugin(QObject* parent = 0);
    virtual bool init(QVariantMap parameters);
    virtual bool shutdown();
    virtual QString getPluginName();
};

#endif // QHCOREPLUGIN_H
