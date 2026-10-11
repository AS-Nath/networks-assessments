#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 9091
#define MSS 12
#define NMSG 8
#define GAP_MS 50.0

typedef struct {
    uint32_t seq;
    uint32_t len;
    char data[MSS];
} seg_t;

typedef struct {
    uint32_t seq;
} ack_t;

static const char *messages[NMSG] = {"AB", "CDE", "FGHI", "JK", "LMNOP", "QR", "STU", "VWXYZ"};

static int sock;
static int nagle_on = 1;
static struct sockaddr_in srv;
static char buf[256];
static int buflen = 0;
static int outstanding = 0;
static int ntx = 0;
static int bytes_tx = 0;
static uint32_t next_seq = 1;
static double sent_at[64];
static double t0;
static double srtt = 1200.0;
static double rttvar = 350.0;
static double rto = 2600.0;

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void send_segment(int len, const char *why) {
    seg_t s;
    memset(&s, 0, sizeof s);
    s.seq = htonl(next_seq);
    s.len = htonl((uint32_t)len);
    memcpy(s.data, buf, len);
    sendto(sock, &s, sizeof s, 0, (struct sockaddr *)&srv, sizeof srv);
    sent_at[next_seq] = now_ms();
    printf("[t=%7.1f ms]   TX seg#%u  %2d bytes \"%.*s\"   <- %s\n", sent_at[next_seq] - t0, next_seq, len, len, buf, why);
    memmove(buf, buf + len, buflen - len);
    buflen -= len;
    outstanding += 1;
    next_seq += 1;
    ntx += 1;
    bytes_tx += len;
}

static void nagle_try_send(void) {
    while (buflen > 0) {
        if (buflen >= MSS) {
            send_segment(MSS, "full MSS segment (allowed even with ACK outstanding)");
        }
        else if (outstanding == 0) {
            send_segment(buflen, "no unacked data -> small segment allowed");
        }
        else {
            printf("[t=%7.1f ms]   HOLD %d byte(s) \"%.*s\" buffered, %d segment(s) unACKed\n", now_ms() - t0, buflen, buflen, buf, outstanding);
            break;
        }
    }
}

static void handle_ack(void) {
    ack_t a;
    if (recvfrom(sock, &a, sizeof a, 0, NULL, NULL) != (ssize_t)sizeof a) {
        return;
    }
    uint32_t seq = ntohl(a.seq);
    double R = now_ms() - sent_at[seq];
    double nv = 0.75 * rttvar + 0.25 * fabs(R - srtt);
    double ns = 0.875 * srtt + 0.125 * R;
    rttvar = nv;
    srtt = ns;
    rto = srtt + 4.0 * rttvar;
    outstanding -= 1;
    printf("[t=%7.1f ms]   ACK seg#%u  RTT=%.1f ms  -> SRTT=%.2f RTTVAR=%.2f RTO=%.2f  (unACKed now: %d)\n", now_ms() - t0, seq, R, srtt, rttvar, rto, outstanding);
    if (nagle_on && buflen > 0) {
        printf("                  ACK arrived -> buffer may be flushed\n");
        nagle_try_send();
    }
}

static void pump(double until) {
    for (;;) {
        double rem = until - now_ms();
        if (rem <= 0) {
            return;
        }
        fd_set rf;
        FD_ZERO(&rf);
        FD_SET(sock, &rf);
        struct timeval tv;
        tv.tv_sec = (long)(rem / 1000.0);
        tv.tv_usec = (long)((rem - tv.tv_sec * 1000.0) * 1000.0);
        if (select(sock + 1, &rf, NULL, NULL, &tv) > 0) {
            handle_ack();
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "off") == 0) {
        nagle_on = 0;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    memset(&srv, 0, sizeof srv);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

    printf("===25BCE2202 : Aditya Shankar Nath===\n");
    printf("=== Nagle sender, MSS=%d, Nagle %s ===\n\n", MSS, nagle_on ? "ENABLED" : "DISABLED");
    t0 = now_ms();
    for (int m = 0; m < NMSG; m++) {
        pump(t0 + m * GAP_MS);
        int len = (int)strlen(messages[m]);
        int before = ntx;
        printf("[t=%7.1f ms] APP write #%d \"%s\" (%d bytes)\n", now_ms() - t0, m + 1, messages[m], len);
        memcpy(buf + buflen, messages[m], len);
        buflen += len;
        if (nagle_on) {
            nagle_try_send();
        }
        else {
            while (buflen > 0) {
                send_segment(buflen > MSS ? MSS : buflen, "Nagle OFF -> sent immediately");
            }
        }
        if (ntx == before) {
            printf("                  => BUFFERED (waiting for ACK)\n");
        }
        else if (buflen > 0) {
            printf("                  => PARTLY SENT (full MSS reached), %d byte(s) still buffered\n", buflen);
        }
        else {
            printf("                  => TRANSMITTED IMMEDIATELY\n");
        }
    }

    double deadline = now_ms() + 15000.0;
    while ((buflen > 0 || outstanding > 0) && now_ms() < deadline) {
        pump(now_ms() + 10.0);
    }

    printf("\n=== RESULT (Nagle %s) ===\n", nagle_on ? "ENABLED" : "DISABLED");
    printf("Application writes : %d (26 bytes of data)\n", NMSG);
    printf("Network transmissions: %d segments, %d data bytes\n", ntx, bytes_tx);
    close(sock);
    return 0;
}