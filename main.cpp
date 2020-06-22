#include <iostream>

#include <unistd.h>

#include "types.h"
#include "xmodem.h"

// #define MAJOR 1
// #define MINOR 0
// #define REVISION 0
// #define STRINGIFY_HELPER(v) #v
// #define STRINGIFY(v) STRINGIFY_HELPER(v)
// #define VERSION_STRING() \
//     STRINGIFY(MAJOR)     \
//     "." STRINGIFY(MINOR) "." STRINGIFY(REVISION)

void usage(const string &progname)
{
    LOGI << "usage: " + progname + " usbloader.bin /dev/ttyUSB0" << endl;
}

int main(int argc, char **argv)
{
    int ret = -1;

    if (argc < 3)
    {
        usage(argv[0]);
        return ret;
    }

    string usbloader = argv[1];
    string ttydev = argv[2];

    LOGI << "bootloader: " << usbloader << ", device " << ttydev << endl;
    if (access(usbloader.c_str(), F_OK | R_OK) ||
        access(ttydev.c_str(), F_OK | R_OK))
    {
        LOGE << "either " + usbloader + " or " + ttydev + " cannot be accessed" << endl;
        return ret;
    }

    Xmodem xmodem(usbloader, ttydev);
    return xmodem.burn_bootloader();
}
