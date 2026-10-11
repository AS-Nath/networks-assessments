#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 9091
#define MSS 12
#define ACK_DELAY_MS 500.0
#define IDLE_MS 3000.0
#define MAXPEND 64

typedef struct {
    uint32_t seq;
    uint32_t len;
    char data[MSS];
} seg_t;

typedef struct {
    uint32_t seq;
} ack_t;

typedef struct {
    uint32_t seq;
    double due;
    struct sockaddr_in addr;
} pend_t;

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(void) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_in me;
    memset(&me, 0, sizeof me);
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY);
    me.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr *)&me, sizeof me) < 0) {
        perror("bind");
        return 1;
    }

    pend_t pend[MAXPEND];
    int npend = 0;
    int segs = 0;
    int bytes = 0;
    int slen = 0;
    char stream[512];
    double t0 = 0;
    double last = now_ms();

    printf("===25BCE2202 : Aditya Shankar Nath===\n");
    printf("[receiver] UDP port %d, ACK delay %.0f ms\n", PORT, ACK_DELAY_MS);
    for (;;) {
        double now = now_ms();
        double wait = 200.0;
        for (int i = 0; i < npend; i++) {
            if (pend[i].due - now < wait) {
                wait = pend[i].due - now;
            }
        }
        if (wait < 0) {
            wait = 0;
        }
        if (segs > 0 && npend == 0 && now - last > IDLE_MS) {
            break;
        }

        fd_set rf;
        FD_ZERO(&rf);
        FD_SET(sock, &rf);
        struct timeval tv;
        tv.tv_sec = (long)(wait / 1000.0);
        tv.tv_usec = (long)((wait - tv.tv_sec * 1000.0) * 1000.0);
        if (select(sock + 1, &rf, NULL, NULL, &tv) > 0) {
            seg_t s;
            struct sockaddr_in cli;
            socklen_t cl = sizeof cli;
            if (recvfrom(sock, &s, sizeof s, 0, (struct sockaddr *)&cli, &cl) == (ssize_t)sizeof s) {
                uint32_t len = ntohl(s.len);
                uint32_t seq = ntohl(s.seq);
                if (t0 == 0) {
                    t0 = now_ms();
                }
                last = now_ms();
                printf("[t=%7.1f ms] RX seg#%u  %2u bytes \"%.*s\"\n", last - t0, seq, len, (int)len, s.data);
                if (len <= MSS && slen + (int)len < (int)sizeof stream) {
                    memcpy(stream + slen, s.data, len);
                    slen += (int)len;
                }
                segs += 1;
                bytes += (int)len;
                if (npend < MAXPEND) {
                    pend[npend].seq = seq;
                    pend[npend].due = last + ACK_DELAY_MS;
                    pend[npend].addr = cli;
                    npend += 1;
                }
            }
        }
        now = now_ms();
        for (int i = 0; i < npend; ) {
            if (pend[i].due <= now) {
                ack_t a;
                a.seq = htonl(pend[i].seq);
                sendto(sock, &a, sizeof a, 0, (struct sockaddr *)&pend[i].addr, sizeof pend[i].addr);
                printf("[t=%7.1f ms] ACK seg#%u sent\n", now - t0, pend[i].seq);
                npend -= 1;
                pend[i] = pend[npend];
            }
            else {
                i += 1;
            }
        }
    }
    printf("\n[receiver] %d segment(s), %d bytes. Re-assembled stream: \"%.*s\"\n", segs, bytes, slen, stream);
    close(sock);
    return 0;
}