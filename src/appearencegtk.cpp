/* KDE GTK Configuration Module
 * 
 * Copyright 2011 José Antonio Sanchez Reynaga <joanzare@gmail.com>
 * Copyright 2011 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDirIterator>

#include "abstractappearance.h"
#include "appearencegtk.h"
#include "appearancegtk2.h"
#include "appearancegtk3.h"

AppearenceGTK::AppearenceGTK()
{
    m_app << new AppearanceGTK2;
    m_app << new AppearanceGTK3;
}

AppearenceGTK::~AppearenceGTK()
{
    qDeleteAll(m_app);
}

QString AppearenceGTK::getTheme() const
{
    return gtk2Appearance()->getTheme();
}

void AppearenceGTK::setTheme(const QString& name)
{
    return gtk2Appearance()->setTheme(name);
}

QString AppearenceGTK::getThemeGtk3() const
{
    return gtk3Appearance()->getTheme();
}

void AppearenceGTK::setThemeGtk3(const QString& name)
{
    return gtk3Appearance()->setTheme(name);
}

bool AppearenceGTK::loadFileConfig()
{
    bool correct = false;
    for(AbstractAppearance *app : m_app) {
        bool c = app->loadSettings();
        correct = correct || c;
    }
    return correct;
}

bool AppearenceGTK::saveFileConfig()
{
    bool correct = true;
    for(AbstractAppearance *app : m_app) {
        bool c = app->saveSettings();
        correct = correct && c;
    }
    return correct;
}
