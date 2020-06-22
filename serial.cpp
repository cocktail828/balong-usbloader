#include <iostream>

#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/epoll.h>

#include "serial.h"
#include "types.h"
#include "log.h"

bool SerialPort::open(const string &path)
{
    m_device = path;
    return open();
}

bool SerialPort::open()
{
    m_ttyfd = ::open(m_device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_ttyfd < 0)
    {
        m_isopen = false;
        return false;
    }

    m_isopen = true;
    setUart();
    return true;
}

void SerialPort::setUart()
{
    struct termios tio;
    struct termios settings;
    int retval;
    memset(&tio, 0, sizeof(tio));
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
    tio.c_lflag = 0;
    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 5;
    cfsetospeed(&tio, B115200); // 115200 baud
    cfsetispeed(&tio, B115200); // 115200 baud
    tcsetattr(m_ttyfd, TCSANOW, &tio);
    retval = tcgetattr(m_ttyfd, &settings);
    if (-1 == retval)
    {
        LOGE << "setUart error" << endl;
        return;
    }
    cfmakeraw(&settings);
    settings.c_cflag |= CREAD | CLOCAL;
    tcflush(m_ttyfd, TCIOFLUSH);
    tcsetattr(m_ttyfd, TCSANOW, &settings);
}

bool SerialPort::isOpen() const
{
    return m_isopen;
}

int SerialPort::write(const void *data, int length)
{
    int ret;
    struct epoll_event ev;
    struct epoll_event events[1];
    int epfd = epoll_create(1);
    if (epfd < 0)
    {
        LOGE << "epoll create failed" << endl;
        return -1;
    }

#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif
    ev.events = EPOLLOUT | EPOLLRDHUP;
    ev.data.fd = m_ttyfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, m_ttyfd, &ev);
    int num = epoll_wait(epfd, events, 1, timeout);
    if (num > 0)
    {
        if (events[0].events & EPOLLOUT)
        {
            ret = ::write(m_ttyfd, data, length);
            if (ret == length)
                ret = 0;
            else
            {
                LOGI << "write incomplete or encounter error, length=" << length << ", ret = " << ret << endl;
                ret = -1;
            }
        }
        else
        {
            LOGI << "epoll event " << hex << events[0].events << dec << endl;
            ret = -1;
        }
    }
    else if (num == 0)
    {
        /* TIMEOUT */
        LOGI << "epoll wait timeout" << endl;
        ret = -1;
    }
    else
    {
        LOGI << "epoll_wait error" << endl;
        ret = -1;
    }

    ::close(epfd);
    return ret;
}

int SerialPort::read(void *data, int length)
{
    return read(data, length, NULL);
}

int SerialPort::read(void *data, int length, int *reallen)
{
    int ret;
    struct epoll_event ev;
    struct epoll_event events[1];
    int epfd = epoll_create(1);
    if (epfd < 0)
    {
        LOGE << "epoll create failed" << endl;
        return -1;
    }

    if (reallen)
        *reallen = 0;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = m_ttyfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, m_ttyfd, &ev);
    int num = epoll_wait(epfd, events, 1, timeout);
    if (num > 0)
    {
        if (events[0].events & EPOLLIN)
        {
            ret = ::read(m_ttyfd, data, length);
            if (reallen)
                *reallen = ret;
            if (ret == length)
                ret = 0;
            else
            {
                LOGI << "warnning: want read " << length << " bytes actually read " << ret << " bytes" << std::endl;
                ret = -1;
            }
        }
        else
        {
            LOGI << "epoll event " << hex << events[0].events << dec << endl;
            ret = -1;
        }
    }
    else if (num == 0)
    {
        /* TIMEOUT */
        LOGI << "epoll wait timeout" << endl;
        ret = -1;
    }
    else
    {
        LOGI << "epoll_wait error" << endl;
        ret = -1;
    }

    ::close(epfd);
    return ret;
}

void SerialPort::close()
{
    ::close(m_ttyfd);
    m_isopen = false;
}
