#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <endian.h>

#include "xmodem.h"
#include "log.h"

#ifndef le32toh
#define le32toh letoh32
#endif

/*
    usbloader.bin examples
    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    00 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00
    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    00 00 00 00 01 00 00 00 54 0e 00 00 00 00 00 00  <raminit>
    54 00 00 00 02 00 00 00 f4 2d 04 00 00 00 90 57  <onchip>
    a8 0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    00 00 00 00
*/

void show_hex(string desc, uint8_t *arr, int len)
{
    if (len <= 0)
        return;

    if (!desc.empty())
        LOGI << desc << "(" << dec << len << ")" << endl;

    for (int idx = 0; idx < len; idx++)
    {
        if (!(idx % 8) && idx)
            LOGI << "  ";
        if (!(idx % 16) && idx)
            LOGI << endl;
        LOGI << hex << std::setw(2) << std::setfill('0') << (uint32_t)arr[idx] << " ";
    }
    LOGI << dec << endl
         << endl;
}

static void status_bar(int byte, int byte_all)
{
    if (byte >= byte_all)
    {
        LOGI << endl;
        LOGI << "status: " << byte << "/" << byte_all << endl;
        return;
    }
    LOGI << ".";
}

static void set_member(struct binhdr *st, const char *name)
{
    st->pad0 = le32toh(st->pad0);
    st->index = le32toh(st->index);
    st->length = le32toh(st->length);
    st->magic = le32toh(st->magic);
    LOGI << name << endl
         << "\tpad0 " << st->pad0
         << "\tindex " << st->index
         << "\tlength " << st->length << "(" << hex << st->length << ")" << dec
         << "\tmagic " << st->magic << "(" << hex << st->magic << ")" << dec << endl;
}

int Xmodem::parser()
{
    LOGI << "try to parser file header" << endl;
    ifstream fin(filename);
    if (!fin.is_open())
    {
        LOGE << "cannot open " + filename << endl;
        return -1;
    }

    fin.seekg(16 * 2, ios::beg);
    char *buf = (char *)&raminit;
    fin.read(buf, 16);

    set_member(&raminit, "raminit.bin");

    buf = (char *)&onchip;
    fin.read(buf, 16);
    set_member(&onchip, "onchip.bin");

    return 0;
}

uint16_t Xmodem::calc_crc16(char *ptr, int count)
{
    int crc16;
    char i;

    crc16 = 0;
    while (--count >= 0)
    {
        crc16 = crc16 ^ (int)*ptr++ << 8;
        i = 8;
        do
        {
            if (crc16 & 0x8000)
                crc16 = crc16 << 1 ^ 0x1021;
            else
                crc16 = crc16 << 1;
        } while (--i);
    }

    return (crc16 & 0xffff);
}

void Xmodem::set_crc16(int length)
{
    uint16_t crc16 = calc_crc16((char *)send_buffer, length);
    *(uint16_t *)(send_buffer + length) = htobe16(crc16);
}

int Xmodem::send_async()
{
    if (!serial.isOpen())
    {
        LOGE << serial.m_device + " is not opened!!!" << endl;
        return -1;
    }

    // show_hex(">>>", send_buffer, send_len);
    if (serial.write(send_buffer, send_len))
    {
        LOGE << "serial write error" << endl;
        return -1;
    }

    if (!serial.read(recv_buffer, 1, &recv_len))
    {
        if (recv_len == 1 && recv_buffer[0] == ACK)
            return 0;
        else
        {
            show_hex("<<<", recv_buffer, recv_len);
        }
    }
    else
        LOGE << "serial read error, recv_len " << recv_len << endl;
    return -1;
}

void Xmodem::compose_start_frame(int idx)
{
    struct binhdr *hdr = NULL;
    if (idx == 0)
    {
        hdr = &raminit;
    }
    else
    {
        hdr = &onchip;
        onchip.length += 1;
    }

    cmdframe->command_id = STX;
    cmdframe->command_idx = 0;
    cmdframe->complement = 0xff;
    cmdframe->image_idx = hdr->index;
    cmdframe->image_length = htobe32(hdr->length);
    cmdframe->image_magic = htobe32(hdr->magic);
    set_crc16(sizeof(struct commandframe));
    send_len = sizeof(struct commandframe) + 2;
}

void Xmodem::compose_data_frame(int len)
{
    char *ptr = NULL;

    dataframe->command_idx += 1;
    dataframe->complement = ~(cmdframe->command_idx);

    if (len)
    {
        dataframe->command_id = DTX;
        ptr = (char *)(dataframe->data);
        fin.read(ptr, len);
        // append END_OF_IMAGE flag */
        if (len < 1024)
            *(ptr + len - 1) = EOI;
    }
    else
    {
        dataframe->command_id = EOT;
    }
    set_crc16(len + 3);
    send_len = len + 5;
}

int Xmodem::burn_raminit()
{
    int remain_len;
    int data_offset;

    LOGI << "try to burn raminit.bin" << endl;
    compose_start_frame(0);
    if (send_async())
    {
        goto exit;
    }

    remain_len = raminit.length;
    data_offset = 16 * 5 + 4;
    fin.seekg(data_offset, ios::beg);
    while (remain_len > 0)
    {
        int length = remain_len > XMODEM_MAX_TX ? XMODEM_MAX_TX : remain_len;
        compose_data_frame(length);
        if (send_async())
        {
            goto exit;
        }
        remain_len -= length;
        status_bar(raminit.length - remain_len, raminit.length);
    }

    compose_data_frame(0);
    if (send_async())
    {
        goto exit;
    }

    LOGI << __func__ << " finished" << endl;
    return 0;
exit:
    LOGE << "burn raminit failed" << endl;
    return -1;
}

int Xmodem::burn_onchip()
{
    int remain_len;
    int data_offset;

    LOGI << "try to burn onchip.bin" << endl;
    compose_start_frame(1);
    if (send_async())
    {
        goto exit;
    }

    remain_len = onchip.length;
    data_offset = 16 * 5 + 4 + raminit.length;
    fin.seekg(data_offset, ios::beg);
    while (remain_len > 0)
    {
        int length = remain_len > XMODEM_MAX_TX ? XMODEM_MAX_TX : remain_len;
        compose_data_frame(length);
        if (send_async())
        {
            goto exit;
        }
        remain_len -= length;
        status_bar(onchip.length - remain_len, onchip.length);
    }

    compose_data_frame(0);
    if (send_async())
    {
        goto exit;
    }

    LOGI << __func__ << " finished" << endl;
    return 0;
exit:
    LOGE << "burn onchip failed" << endl;
    return -1;
}

int Xmodem::burn_bootloader()
{
    LOGI << "try to burn bootloader(" + filename + ")" << endl;
    if (parser())
    {
        LOGE << "parser " + filename + " failed" << endl;
        return -1;
    }

    if (!serial.isOpen())
    {
        LOGE << "serial " << serial.m_device << " is not opened" << endl;
        return -1;
    }

    return burn_raminit() || burn_onchip();
}