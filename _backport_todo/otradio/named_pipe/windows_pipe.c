#include <windows.h>
#include <stdio.h>
#include "radio_pipe.h" // platform header
#include "radio_named_pipe.h"
#include "encode.h"
#include <m2/radio.h>
#include <unistd.h>

/**************** public variables... **************************/
unsigned char rx_buf[RX_BUF_SIZE];
int rx_buf_in_idx;
int rx_buf_out_idx;
/***************************************************************/

typedef enum {
    RS__WAITING_FOR_RXSTART = 0,    // 0
    RS__WAITING_FOR_CONNECT,        // 1
    RS__READFILE,                   // 2
    RS__EXITED,                     // 3
} dbg_rx_state_e;
volatile dbg_rx_state_e dbg_rx_state;

static OVERLAPPED goConnect;
static HANDLE ghRxPipe = NULL;
static HANDLE ghThread = NULL;
static volatile HANDLE ghRxStartEvent = NULL;

const char* pipe_name_prefix = "\\\\.\\pipe\\air";
ot_u8 gsid;
ot_u16 g_timeout_ms;

void
diag_print()
{
    printf("ghRxPipe: %p\n", ghRxPipe);
    printf("dbg_rx_state: %d\n", dbg_rx_state);
}

DWORD WINAPI
rx_thread(void *lpParameter_)
{
    DWORD dwWaitResult, e;
    char rx_pipe_name[32];
    BOOL b, bReadFail;

    for (;;) {
        int nbytes;
        DWORD NumberOfBytesRead;
        //printf("rx_thread waiting for rx start...\n");
        dbg_rx_state = RS__WAITING_FOR_RXSTART;
    
        dwWaitResult = WaitForSingleObject(ghRxStartEvent, INFINITE);
        if (dwWaitResult != WAIT_OBJECT_0) {
            printf("[41mghRxStartEvent wait fail %ld, %ld[0m\n", dwWaitResult, GetLastError());
            Sleep(50);
            continue;
        }

        //printf("rx_thread notified\n");
        ResetEvent(ghRxStartEvent);

        if (g_timeout_ms == 0) {
            /* some events on the pipe might cause spurious event */
            //printf("rx_thread: zero timeout\n");
            radio_pipe_close();
            continue;
        }

        sprintf(rx_pipe_name, "%s%02x", pipe_name_prefix, gsid);
create_pipe:
        ghRxPipe = CreateNamedPipe(
            /*LPCTSTR lpName*/rx_pipe_name,
            /*DWORD dwOpenMode*/PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            /*DWORD dwPipeMode*/PIPE_TYPE_MESSAGE,
            /*DWORD nMaxInstances*/1,  // XXX each receiver on its own pipe XXX
            /*DWORD nOutBufferSize*/0,
            /*DWORD nInBufferSize*/632,
            /*DWORD nDefaultTimeOut*/g_timeout_ms,
            /*LPSECURITY_ATTRIBUTES lpSecurityAttributes*/NULL
        );
        if (ghRxPipe == INVALID_HANDLE_VALUE) {
            e = GetLastError();
            ghRxPipe = NULL;
            if (e == ERROR_PIPE_BUSY) {
                //printf("CreateNamedPipe(): somebody else already receiving  %d\n", g_timeout_ms);
                if (g_timeout_ms > 20) {
                    g_timeout_ms -= 20;
                    Sleep(20);
                    goto create_pipe;
                }
            } else {
                if (e == ERROR_INVALID_HANDLE) {
                    printf("[41mCreateNamedPipe(): ERROR_INVALID_HANDLE[0m\n");
                } else {
                    printf("[41mCreateNamedPipe(): %ld[0m\n", e);
                }
                radio.evtdone(RM2_ERR_GENERIC, 0);
                g_timeout_ms = 0;
            }
            continue;
        }

        // create manual reset event object
        goConnect.hEvent = CreateEvent(
            /*LPSECURITY_ATTRIBUTES lpEventAttributes*/NULL,
            /*BOOL bManualReset*/TRUE,
            /*BOOL bInitialState*/FALSE,
            /*LPCTSTR lpName*/"foo"
        );
        if (goConnect.hEvent == NULL) {
            printf("CreateEvent() %ld\n", GetLastError());
            break;
        }

        b = ConnectNamedPipe(
            /*HANDLE hNamedPipe*/ghRxPipe,
            /*LPOVERLAPPED lpOverlapped*/&goConnect
        );
        if (b != 0) {
            printf("%d = ConnectNamedPipe()\n", b); // unexpected
            break;
        }

#ifdef RADIO_DEBUG
        printf("rx_thread wait %d for connect %s\n", g_timeout_ms, rx_pipe_name);
#endif
        dbg_rx_state = RS__WAITING_FOR_CONNECT;
        dwWaitResult = WaitForSingleObject(goConnect.hEvent, g_timeout_ms);
        if (dwWaitResult == WAIT_TIMEOUT) {
            /* nobody transmitted within timeout period */
#ifdef RADIO_DEBUG
            printf("rx_thread ConnectNamedPipe() WAIT_TIMEOUT\n");
#endif
            radio_pipe_close();
            continue;
        } else if (dwWaitResult != WAIT_OBJECT_0) {
            printf("[41mconnect wait failed %ld[0m\n", GetLastError());
            radio_pipe_close();
            continue;
        }

        /* transmitting side has connected */

        bReadFail = FALSE;
        nbytes = em2_remaining_bytes();
        do { // while (nbytes > 0)...
            if (nbytes > (RX_BUF_SIZE-1))
                nbytes = (RX_BUF_SIZE-1);

            dbg_rx_state = RS__READFILE;
            /* this read is blocking: we're in our own thread */
            b = ReadFile(
                /*HANDLE hFile*/ghRxPipe,
                /*LPVOID lpBuffer*/&rx_buf,
                /*DWORD nNumberOfBytesToRead*/nbytes,
                /*LPDWORD lpNumberOfBytesRead*/&NumberOfBytesRead,
                /*LPOVERLAPPED lpOverlapped*/&goConnect
            );

            if (b == 0) {
                DWORD nbt;
                DWORD e = GetLastError();
                if (e == ERROR_PIPE_LISTENING) {
                    bReadFail = TRUE;
                    printf("ReadFile(): write side disconnected[0m\n");
                    radio_pipe_close();
                    break;
                } else if (e == ERROR_IO_PENDING) {
                    /* read operation is completing asynchronously */
                    b = GetOverlappedResult(
                        /*HANDLE hFile*/ghRxPipe,
                        /*LPOVERLAPPED lpOverlapped*/&goConnect,
                        /*LPDWORD lpNumberOfBytesTransferred*/&nbt,
                        /*BOOL bWait*/TRUE /* blocking */
                    );
                    if (b) {
                        rx_buf_in_idx = nbt;
                    } else {
                        e = GetLastError();
                        printf("[41mGetOverlappedResult() failed %ld[0m\n", e);
                        radio_pipe_close();
                        bReadFail = TRUE;
                        break;
                    }
                } else {
                    if (e == ERROR_INVALID_HANDLE) {
                        /* this likely occurs because rx pipe was closed */
                        printf("[41mReadFile() ERROR_INVALID_HANDLE[0m\n");
                        radio_pipe_close();
                        bReadFail = TRUE;
                        break;
                    } else {
                        printf("[41mReadFile() failed %ld[0m\n", e);
                        radio_pipe_close();
                        bReadFail = TRUE;
                        break;
                    }
                }
            } else {
                // read operation completed synchronously
                rx_buf_in_idx = NumberOfBytesRead;
            }

            rx_buf_out_idx = 0;
            em2_decode_data();

            nbytes = em2_remaining_bytes();
        } while (nbytes > 0);

        if (bReadFail)
            radio.evtdone(RM2_ERR_GENERIC, 0);
        else
            rx_done_isr(0);

        if (ghRxPipe != NULL) {
            printf("thread: rx handle open\n");
            break;
        }
    } // ...for (;;)

    dbg_rx_state = RS__EXITED;
    printf("[41mrx_thread stop[0m\n");
    return -1;
#if 0
#endif /* #if 0 */

}


static int
create_thread()
{
    ghRxStartEvent = CreateEvent(
        /*LPSECURITY_ATTRIBUTES lpEventAttributes*/NULL,
        /*BOOL bManualReset*/TRUE, // FALSE = auto-reset
        /*BOOL bInitialState*/FALSE,
        /*LPCTSTR lpName*/"rx_start_lock"
    );
    if (ghRxStartEvent == NULL) {
        printf("CreateEvent() fail: %ld\n", GetLastError());
        return -1;
    }

    ghThread = CreateThread(
        /*LPSECURITY_ATTRIBUTES lpThreadAttributes*/NULL,
        /*SIZE_T dwStackSize*/0,    // 0 = default
        /*LPTHREAD_START_ROUTINE lpStartAddress*/rx_thread,
        /*LPVOID lpParameter*/NULL, // pointer passed to thread function
        /*DWORD dwCreationFlags*/0, // 0 = run immediately
        /*LPDWORD lpThreadId*/NULL
    );
    if (ghThread == NULL) {
        printf("CreateThread() fail: %ld\n", GetLastError());
        return -1;
    }

    // give a little time for thread to actually start
    Sleep(20);  

    return 0;
}

void
radio_pipe_close()
{
    CloseHandle(ghRxPipe);
    CloseHandle(goConnect.hEvent);

    g_timeout_ms = 0;
    ghRxPipe = NULL;
}

int
open_pipe_for_rx(int timeout, ot_u8 sid)
{
    if (ghThread == NULL) {
        if (create_thread() != 0)
            return -1;
    }

#ifdef RADIO_DEBUG
    printf("open_pipe_for_rx(%d, 0x%02x)    %d\n", timeout, sid, dbg_rx_state);
#endif
    if (dbg_rx_state != RS__WAITING_FOR_RXSTART) {
        /* start receiver, while already receiving */
        printf("(rx) dbg_rx_state=%d\n", dbg_rx_state);
        if (gsid == sid) {  // receive (again) on same sid
            g_timeout_ms = (int)(timeout * 0.976);  // 1024Hz to milliseconds
            return 0;
        } else {
            // @todo: not quite so simple to change spectrum ID while RX is running
            if (ghRxPipe != NULL) {
                printf("re-open rx\n");
                radio_pipe_close();
                Sleep(50);
            }
        }
    }

    g_timeout_ms = (int)(timeout * 0.976);  // 1024Hz to milliseconds
    gsid = sid;

    if (! SetEvent(ghRxStartEvent) ) {
        printf("SetEvent failed (%ld)\n", GetLastError());
        return -1;
    }

    return 0;
}

int
open_pipe_for_tx(unsigned char *txdata, int data_len, ot_u8 sid)
{
    BOOL b;
    HANDLE h_tx_pipe;
    char pipe_name[32];
    DWORD NumberOfBytesWritten;

#ifdef RADIO_DEBUG
    printf("open_pipe_for_tx(, %d, 0x%02x)  %d\n", data_len, sid, dbg_rx_state);
#endif

    if (ghRxPipe != NULL) {
        printf("for_tx: rx handle open\n");
        return -1;
    }
    if (dbg_rx_state != RS__WAITING_FOR_RXSTART) {
        printf("(tx) [41mdbg_rx_state=%d[0m\n", dbg_rx_state);
        return -1;
    }

    sprintf(pipe_name, "%s%02x", pipe_name_prefix, sid);

    h_tx_pipe = CreateFile(
        /*LPCTSTR lpFileName*/pipe_name,
        /*DWORD dwDesiredAccess*//*GENERIC_READ | */GENERIC_WRITE,
        /*DWORD dwShareMode*/FILE_SHARE_READ | FILE_SHARE_WRITE,
        /*LPSECURITY_ATTRIBUTES lpSecurityAttributes*/NULL,
        /*DWORD dwCreationDisposition*/OPEN_EXISTING,
        /*DWORD dwFlagsAndAttributes*/0,//FILE_FLAG_OVERLAPPED,
        /*HANDLE hTemplateFile*/NULL
    );

    if (h_tx_pipe == INVALID_HANDLE_VALUE) {
        DWORD e = GetLastError();
        if (e == ERROR_FILE_NOT_FOUND) {
            /* equivalent to transmitting with no receivers in range */
#ifdef RADIO_DEBUG
            printf("(no receiving side)\n");
#endif
            return 0;
        }
        printf("[41m(tx) CreateFile(): ");
        if (e == ERROR_PIPE_BUSY)    // all pipe instances are busy
            printf("ERROR_PIPE_BUSY[0m    %d\n", dbg_rx_state);
        else
            printf("%ld[0m    %d\n", e, dbg_rx_state);
        return -1;
    }

    b = WriteFile(
        /*HANDLE hFile*/h_tx_pipe,
        /*LPCVOID lpBuffer*/txdata,
        /*DWORD nNumberOfBytesToWrite*/data_len,
        /*LPDWORD lpNumberOfBytesWritten*/&NumberOfBytesWritten,
        /*LPOVERLAPPED lpOverlapped*/NULL
    );

    if (b == 0) {
        DWORD e = GetLastError();
        /* WriteFile failed */
        if (e == ERROR_NO_DATA)
            printf("[41mWriteFile: rx side closing[0m\n");
        else
            printf("[41mWriteFile: %ld[0m\n", e);
        return -1;
    }

    if (NumberOfBytesWritten != data_len) {
        printf("NumberOfBytesWritten: %ld (data_len=%d)\n", NumberOfBytesWritten, data_len);
        return -1;
    }

    CloseHandle(h_tx_pipe);

    return 0;
}

