#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#include "OT_types.h"
#include "radio_named_pipe.h"
#include "encode.h"
#include "radio_pipe.h"
#include "radio.h"

#include "named_pipe/board_named_pipe.h"    // includes platform_named_pipe.h

// http://linux.die.net/man/7/pipe


unsigned char rx_buf[RX_BUF_SIZE];
int rx_buf_in_idx;
int rx_buf_out_idx;

static const char* pipe_name_prefix = "../../../pipe";

int fd;

void
radio_pipe_close()
{
#ifdef RADIO_DEBUG
    printf("closing fd %d\n", em2_remaining_bytes());
#endif
    close(fd);
}

int
open_pipe_for_tx(unsigned char *txdata, int data_len, ot_u8 sid)
{
    ssize_t r;
    char pipe_name[32];

    sprintf(pipe_name, "%s%x", pipe_name_prefix, sid);
#ifdef RADIO_DEBUG
    printf("open_pipe_for_tx %d: %s ", data_len, pipe_name);
#endif


    // must open nonblocking or get EINTR
try:
    fd = open(pipe_name, O_WRONLY);// | O_NONBLOCK);
    if (fd == -1) {
        //if (errno == ENXIO) {
            /* equivalent of transmitting with nobody to receive this */
            //if you want to know.. printf("EXNIO\n");
        if (errno == ENOENT) {
            fd = mkfifo(pipe_name, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
            if (fd == -1)
                perror("mkfifo");
            else
                goto try;
        }

        if (errno == EINTR) {
            //printf("tx EINTR, nobody receiving\n");
            return 0;   // nobody on receiving side: real radio would never know
        } else {
            printf("(tx) open errno=%d\n", errno);  // some unforseen failure
            perror("open");
            return -1;
        }
    }

#ifdef RADIO_DEBUG
    printf("writing %d: ", (int)data_len);
    for (r = 0; r < data_len; r++)
        printf("%02x ", txdata[r]);
    printf("\n");
#endif

    r = write(fd, txdata, data_len);
    if (r == -1) {
        printf("(tx) write errno=%d\n", errno);
        perror("(tx) write");
        return -1;
    } else if (r != data_len) {
        printf("[41mshort write %d %d[0m\n", (int)r, (int)data_len);
    }

    // pretend it took this long to transmit
    usleep((r * 144) + 1000);
    close(fd);
    fd = -1;

    return 0;
}

//struct pollfds pfds;

void
sigio_handler(int a)
{
    int read_len;
    //int i;

    if (a == SIGIO) {
        read_len = em2_remaining_bytes();
        while (read_len > 0) {
            if (read_len > sizeof(rx_buf)-1)
                read_len = sizeof(rx_buf)-1;
            rx_buf_in_idx = read(fd, rx_buf, read_len);
            if (rx_buf_in_idx < 0) {
                if (errno == EAGAIN) {
                    /* pipe is open for writing, but is empty */
                    printf("(rx) EAGAIN\n");
                    return;
                }
                printf("(rx) read(,,%d): errno=%d\n", read_len, errno);
                perror("sigio-read");
                printf("remaining bytes: %d\n", read_len);
                radio.evtdone(RM2_ERR_GENERIC, 0);
                break;
            }
            if (rx_buf_in_idx == 0) {
                /* ? spurious SIGIO ? */
                //printf("0 = read(,,%d)\n", read_len);
                return;
            }
            /*printf("RX%d:", rx_buf_in_idx);
            for (i = 0; i < rx_buf_in_idx; i++)
                printf("%02x ", rx_buf[i]);
            printf(" (%d)\n", i);*/
            rx_buf_out_idx = 0;
            em2_decode_data();
            read_len = em2_remaining_bytes();
            //printf("remaining: %d (%d)\n", read_len, rx_buf_out_idx);
        }

        // check for any stale bytes, this fd was opened nonblocking
        do {
            rx_buf_in_idx = read(fd, rx_buf, sizeof(rx_buf));
            //printf("%d = read()\n", rx_buf_in_idx);
            if (rx_buf_in_idx > 0) {
                /* em2_remaining_bytes() might be returning bogus */
                printf("[41mstale %d[0m\n", rx_buf_in_idx);
            }
            else if (rx_buf_in_idx < 0 && errno != EAGAIN && errno != EBADF) {
                // EAGAIN means nothing is in the fifo to read
                // EBADF means writing end of pipe closed
                printf("(rx stale) read errno=%d\n", errno);
                perror("read");
            }
            // if read full amount then there is more
        } while (rx_buf_in_idx == sizeof(rx_buf));
        rx_done_isr(0);
        close(fd);
    } else
        printf("not sigio\n");
}

#if 0
static void
print_fd_flags(char* label, int fd)
{
    int ret = fcntl(fd, F_GETFL);

    printf("%s: ", label);

    if (ret & O_ASYNC)
        printf("O_ASYNC ");
    if (ret & O_NONBLOCK)
        printf("O_NONBLOCK ");

    printf("\n");
}
#endif /* #if 0 */

int
open_pipe_for_rx(int timeout, ot_u8 sid)
{
    char pipe_name[32];
    struct sigaction rx_action;

    sprintf(pipe_name, "%s%x", pipe_name_prefix, sid);
#ifdef RADIO_DEBUG
    printf("open_pipe_for_rx(%d): %s\n", timeout, pipe_name);
#endif
    rx_action.sa_handler = sigio_handler;
    sigemptyset(&rx_action.sa_mask);
    rx_action.sa_flags = 0;
    sigaction( SIGIO, &rx_action, NULL );

    // must open nonblocking or get EINTR
try:
    fd = open(pipe_name, O_RDONLY | O_NONBLOCK);//O_ASYNC );
    if (fd == -1) {
        if (errno == ENOENT) {
            fd = mkfifo(pipe_name, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
            if (fd == -1) {
                perror("mkfifo");
                return -1;
            }
            goto try;
        }
        printf("(rx) open errno=%d\n", errno);
        perror("(rx) open");
        return -1;
    } else {
        fcntl(fd, F_SETOWN, getpid());
        fcntl(fd, F_SETFL, O_ASYNC | O_NONBLOCK);
    }

    return 0;
}

