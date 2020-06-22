#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#include <unistd.h>

#include "types.h"

#define timeout 5000

class SerialPort
{
public:
    string m_device;
    int m_ttyfd;
    bool m_isopen;

public:
    SerialPort() {}
    ~SerialPort()
    {
        ::close(m_ttyfd);
        m_isopen = false;
        m_device.clear();
    }

    bool open(const string &path);
    bool open();
    void setUart();
    bool isOpen() const;
    int write(const void *data, int length);
    int read(void *data, int length);
    int read(void *data, int length, int *reallen);
    void close();
};

#endif // __SERIALPORT_H__
