/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2011, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include "constants.h"
#include "vpiano.h"
#if defined(RAWKBD_SUPPORT)
#include "rawkeybdapp.h"
#endif
#include <QtGui/QApplication>
#include <getopt.h>
#include <iostream>
#include <cstring>
#if defined(Q_OS_SYMBIAN)
// to lock orientation in Symbian
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif

const char* const short_options = "m:";
const struct option long_options[] = {
    { "rtmidi-backend",     1, NULL, 'm' },
    { NULL,       0, NULL, 0   }   /* Required at end of array.  */
};

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QSTR_DOMAIN);
    QCoreApplication::setOrganizationDomain(QSTR_DOMAIN);
    QCoreApplication::setApplicationName(QSTR_APPNAME);
#if defined(RAWKBD_SUPPORT)
    RawKeybdApp a(argc, argv);
#else
    QApplication a(argc, argv);
#endif
    // vmpk options
    const char *rtmidi_backend = NULL;
    int next_option;
    do {
        next_option = getopt_long (argc, argv, short_options, long_options, NULL);
        switch (next_option) {
        case 'm':
            rtmidi_backend=optarg;
            if (
#if defined(__LINUX_ALSASEQ__)
                (strcmp(rtmidi_backend, "alsa")) &&
#endif
#if defined(__LINUX_JACK__)
                (strcmp(rtmidi_backend, "jack")) &&
#endif
#if defined(__IRIX_MD__)
                (strcmp(rtmidi_backend, "irix")) &&
#endif
#if defined(__MACOSX_CORE__)
                (strcmp(rtmidi_backend, "coremidi")) &&
#endif
#if defined(__WINDOWS_MM__)
                (strcmp(rtmidi_backend, "winmm")) &&
#endif
#if defined(NETWORK_MIDI)
                (strcmp(rtmidi_backend, "net")) &&
#endif
                true) {
                std::cerr << "Unknown or unavailable RtMidi backend '" << rtmidi_backend << "'!\n";
                return EXIT_FAILURE;
            }
            break;
        default:
            break;
        }
    } while (next_option != -1);
#if defined(Q_OS_LINUX)
    a.setWindowIcon(QIcon(":/vpiano/vmpk_32x32.png"));
#endif //Q_OS_LINUX
#if defined(Q_OS_SYMBIAN)
    // Lock orientation to portrait in Symbian
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
    TRAP_IGNORE(
        if(appUi) {
            appUi->SetOrientationL(CAknAppUi::EAppUiOrientationLandscape);
        }
    );
#endif //Q_OS_SYMBIAN
    VPiano w(rtmidi_backend);
    if (w.isInitialized()) {
#if defined(SMALL_SCREEN)
        w.showMaximized();
#else
        w.show();
#endif
        return a.exec();
    }
    return EXIT_FAILURE;
}
