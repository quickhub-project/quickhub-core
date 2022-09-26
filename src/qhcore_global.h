/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#ifndef COREPLUGIN_GLOBAL_H
#define COREPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COREPLUGIN_LIBRARY)
#  define COREPLUGINSHARED_EXPORT Q_DECL_EXPORT
#elif defined(NO_PLUGIN)
#  define COREPLUGINSHARED_EXPORT
#else
#  define COREPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // COREPLUGIN_GLOBAL_H
