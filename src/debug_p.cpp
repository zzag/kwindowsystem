/*
 *   Copyright 2015 Christoph Cullmann <cullmann@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "debug_p.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
// logging category for this framework, default: log stuff >= warning
Q_LOGGING_CATEGORY(LOG_KWINDOWSYSTEM, "org.kde.kwindowsystem", QtWarningMsg)
Q_LOGGING_CATEGORY(LOG_KKEYSERVER_X11, "org.kde.kwindowsystem.keyserver.x11", QtWarningMsg)
#else
Q_LOGGING_CATEGORY(LOG_KWINDOWSYSTEM, "org.kde.kwindowsystem")
Q_LOGGING_CATEGORY(LOG_KKEYSERVER_X11, "org.kde.kwindowsystem.keyserver.x11")
#endif