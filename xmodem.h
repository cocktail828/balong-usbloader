#ifndef __XMODEM_H__
#define __XMODEM_H__

#include "serial.h"
#include "types.h"

/* command frame id */
#define STX 0xfe
/* data frame if */
#define DTX 0xda
/* end of data frame transfer */
#define EOT 0xed
/* response ACK */
#define ACK 0xaa
/* end of image, append this to the end of last image */
#define EOI 0xe0

struct binhdr
{
    uint32_t pad0;
    uint32_t index;
    uint32_t length;
    uint32_t magic;
};

struct commandframe
{
    uint8_t command_id;
    uint8_t command_idx;
    uint8_t complement;
    uint8_t image_idx;
    be32_t image_length;
    be32_t image_magic;
} __attribute__((aligned(1)));

struct dataframe
{
    uint8_t command_id;
    uint8_t command_idx;
    uint8_t complement;
    uint8_t data[0];
} __attribute__((aligned(1)));

#define XMODEM_MAX_TX 1024
/*  | command_id | command_idx | complement | image_idx | image_length | image_magic | crc16(2 bytes) | */
/*  | command_id | command_idx | complement | data(1024 bytes at most) | crc16(2 bytes) | */
class Xmodem
{
    string filename;
    int send_len;
    int recv_len;
    uint8_t send_buffer[2048];
    uint8_t recv_buffer[2048];
    struct binhdr raminit;
    struct binhdr onchip;
    struct commandframe *cmdframe;
    struct dataframe *dataframe;

    SerialPort serial;
    ifstream fin;

public:
    Xmodem(const string &filename, const string &ttydev)
        : filename(filename),
          send_len(0),
          recv_len(0),
          cmdframe((struct commandframe *)send_buffer),
          dataframe((struct dataframe *)send_buffer),
          fin(filename)
    {
        serial.open(ttydev);
    }
    ~Xmodem() {}

    void compose_start_frame(int idx);
    void compose_data_frame(int len);
    uint16_t calc_crc16(char *ptr, int count);
    void set_crc16(int length);
    int send_async();
    int parser();
    int burn_raminit();
    int burn_onchip();
    int burn_bootloader();
};

#endif //__XMODEM_H__
