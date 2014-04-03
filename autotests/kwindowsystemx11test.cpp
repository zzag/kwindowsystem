/*
 *   Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>
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
#include "nettesthelper.h"
#include "kwindowsystem.h"
#include "netwm.h"

#include <qtest_widgets.h>
#include <QSignalSpy>
#include <QWidget>
#include <QX11Info>
Q_DECLARE_METATYPE(WId)

class KWindowSystemX11Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // needs to be first test, would fail if run after others (X11)
    void testActiveWindowChanged();
    void testWindowAdded();
    void testWindowRemoved();
    void testDesktopChanged();
    void testNumberOfDesktopsChanged();
    void testDesktopNamesChanged();
    void testShowingDesktopChanged();
    void testWorkAreaChanged();
};

void KWindowSystemX11Test::testActiveWindowChanged()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)));

    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QCOMPARE(KWindowSystem::activeWindow(), widget->winId());
}

void KWindowSystemX11Test::testWindowAdded()
{
    qRegisterMetaType<WId>("WId");
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowAdded(WId)));
    QSignalSpy stackingOrderSpy(KWindowSystem::self(), SIGNAL(stackingOrderChanged()));
    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();
    QTest::qWaitForWindowExposed(widget.data());
    QVERIFY(spy.count() > 0);
    bool hasWId = false;
    for (auto it = spy.constBegin(); it != spy.constEnd(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }
        QCOMPARE((*it).count(), 1);
        hasWId = (*it).at(0).toULongLong() == widget->winId();
        if (hasWId) {
            break;
        }
    }
    QVERIFY(hasWId);
    QVERIFY(KWindowSystem::hasWId(widget->winId()));
    QVERIFY(!stackingOrderSpy.isEmpty());
}

void KWindowSystemX11Test::testWindowRemoved()
{
    qRegisterMetaType<WId>("WId");
    QScopedPointer<QWidget> widget(new QWidget);
    widget->show();
    QTest::qWaitForWindowExposed(widget.data());
    QVERIFY(KWindowSystem::hasWId(widget->winId()));

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(windowRemoved(WId)));
    widget->hide();
    spy.wait(1000);
    QCOMPARE(spy.first().at(0).toULongLong(), widget->winId());
    QVERIFY(!KWindowSystem::hasWId(widget->winId()));
}

void KWindowSystemX11Test::testDesktopChanged()
{
    // This test requires a running NETWM-compliant window manager
    if (KWindowSystem::numberOfDesktops() == 1) {
        QSKIP("At least two virtual desktops are required to test desktop changed");
    }
    const int current = KWindowSystem::currentDesktop();

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)));
    int newDesktop = current + 1;
    if (newDesktop > KWindowSystem::numberOfDesktops()) {
        newDesktop = 1;
    }
    KWindowSystem::setCurrentDesktop(newDesktop);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::currentDesktop(), newDesktop);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), newDesktop);
    spy.clear();

    // setting to current desktop should not change anything
    KWindowSystem::setCurrentDesktop(newDesktop);

    // set back for clean state
    KWindowSystem::setCurrentDesktop(current);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::currentDesktop(), current);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), current);
}

void KWindowSystemX11Test::testNumberOfDesktopsChanged()
{
    // This test requires a running NETWM-compliant window manager
    const int oldNumber = KWindowSystem::numberOfDesktops();
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)));

    // KWin has arbitrary max number of 20 desktops, so don't fail the test if we use +1
    const int newNumber = oldNumber < 20 ? oldNumber + 1 : oldNumber - 1;

    NETRootInfo info(QX11Info::connection(), NET::NumberOfDesktops, 0);
    info.setNumberOfDesktops(newNumber);

    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::numberOfDesktops(), newNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), newNumber);
    spy.clear();

    // setting to same number should not change
    info.setNumberOfDesktops(newNumber);

    // set back for clean state
    info.setNumberOfDesktops(oldNumber);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::numberOfDesktops(), oldNumber);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toInt(), oldNumber);
}

void KWindowSystemX11Test::testDesktopNamesChanged()
{
    // This test requires a running NETWM-compliant window manager
    const QString origName = KWindowSystem::desktopName(KWindowSystem::currentDesktop());
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(desktopNamesChanged()));

    const QString testName = QStringLiteral("testFooBar");

    KWindowSystem::setDesktopName(KWindowSystem::currentDesktop(), testName);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::desktopName(KWindowSystem::currentDesktop()), testName);
    QCOMPARE(spy.count(), 1);
    spy.clear();

    QX11Info::setAppTime(QX11Info::getTimestamp());

    // setting back to clean state
    KWindowSystem::setDesktopName(KWindowSystem::currentDesktop(), origName);
    QVERIFY(spy.wait());
    QCOMPARE(KWindowSystem::desktopName(KWindowSystem::currentDesktop()), origName);
    QCOMPARE(spy.count(), 1);
}

void KWindowSystemX11Test::testShowingDesktopChanged()
{
    NETRootInfo rootInfo(QX11Info::connection(), NET::Supported | NET::SupportingWMCheck);
    if (qstrcmp(rootInfo.wmName(), "Openbox") == 0) {
        QSKIP("setting name test fails on openbox");
    }
    QX11Info::setAppTime(QX11Info::getTimestamp());
    const bool showingDesktop = KWindowSystem::showingDesktop();

    QSignalSpy spy(KWindowSystem::self(), SIGNAL(showingDesktopChanged(bool)));

    NETRootInfo info(QX11Info::connection(), 0, NET::WM2ShowingDesktop);
    info.setShowingDesktop(!showingDesktop);

    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toBool(), !showingDesktop);
    QCOMPARE(KWindowSystem::showingDesktop(), !showingDesktop);
    spy.clear();

    QX11Info::setAppTime(QX11Info::getTimestamp());

    // setting again should not change
    info.setShowingDesktop(!showingDesktop);

    // setting back to clean state
    info.setShowingDesktop(showingDesktop);
    QVERIFY(spy.wait(100));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toBool(), showingDesktop);
    QCOMPARE(KWindowSystem::showingDesktop(), showingDesktop);
}

void KWindowSystemX11Test::testWorkAreaChanged()
{
    // if there are multiple screens this test can fail as workarea is not multi screen aware
    QSignalSpy spy(KWindowSystem::self(), SIGNAL(workAreaChanged()));
    QSignalSpy strutSpy(KWindowSystem::self(), SIGNAL(strutChanged()));

    QWidget widget;
    widget.setGeometry(0, 0, 100, 10);
    widget.show();

    KWindowSystem::setExtendedStrut(widget.winId(), 10, 0, 10, 0, 0, 0, 100, 0, 100, 0, 0, 0);
    QVERIFY(spy.wait());
    QVERIFY(!spy.isEmpty());
    QVERIFY(!strutSpy.isEmpty());
}

QTEST_MAIN(KWindowSystemX11Test)

#include "kwindowsystemx11test.moc"
