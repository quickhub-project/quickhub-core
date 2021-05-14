/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * It is part of the QuickHub framework - www.quickhub.org
 * Copyright (C) 2021 by Friedemann Metzger - mail@friedemann-metzger.de */

#include "IUser.h"
#include <QDebug>
#include <QCryptographicHash>

int IUser::instanceCount = 0;
void IUser::addToken(QString token)
{
    _tokens.insert(token);
    Q_EMIT sessionCountChanged();
    Q_EMIT dataChanged();
}

bool IUser::removeToken(QString token)
{
    bool remove =_tokens.remove(token);
    Q_EMIT sessionCountChanged();
    Q_EMIT dataChanged();
    return remove;
}

void IUser::removeAllTokens()
{
    _tokens.clear();
    Q_EMIT dataChanged();
}

int IUser::sessionCount()
{
    return _tokens.count();
}

QSet<QString> IUser::getSteadyTokens() const
{
    return QSet<QString>();
}

QString IUser::generateHash(QString pass)
{
    return QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Md5).toHex();
}


QSet<QString> IUser::getTokens() const
{
    return _tokens;
}

