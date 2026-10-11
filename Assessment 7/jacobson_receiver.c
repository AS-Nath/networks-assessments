#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define NPKT 8

typedef struct {
    uint32_t seq;
    uint32_t attempt;
} packet_t;

typedef struct {
    uint32_t seq;
} ack_t;

static const int rtt_profile_ms[NPKT] = {1050, 1280, 1750, 1420, 980, 1560, 1180, 890};
static const int drop_first_copy[NPKT] = {0, 0, 1, 0, 0, 1, 0, 0};

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
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

    printf("===25BCE2202 : Aditya Shankar Nath\n===");
    printf("[receiver] UDP port %d, waiting for packets...\n", PORT);
    double t0 = 0;
    int acked = 0;
    while (acked < NPKT) {
        packet_t p;
        struct sockaddr_in cli;
        socklen_t cl = sizeof cli;
        ssize_t n = recvfrom(sock, &p, sizeof p, 0, (struct sockaddr *)&cli, &cl);
        if (n != (ssize_t)sizeof p) {
            continue;
        }
        if (t0 == 0) {
            t0 = now_ms();
        }
        uint32_t seq = ntohl(p.seq);
        uint32_t att = ntohl(p.attempt);
        if (seq < 1 || seq > NPKT) {
            continue;
        }

        if (att == 1 && drop_first_copy[seq - 1]) {
            printf("[t=%8.1f ms] P%u attempt %u  -> LOST in network (dropped)\n", now_ms() - t0, seq, att);
            continue;
        }
        printf("[t=%8.1f ms] P%u attempt %u  received, ACK delayed %d ms\n", now_ms() - t0, seq, att, rtt_profile_ms[seq - 1]);
        sleep_ms(rtt_profile_ms[seq - 1]);
        ack_t a;
        a.seq = htonl(seq);
        sendto(sock, &a, sizeof a, 0, (struct sockaddr *)&cli, cl);
        printf("[t=%8.1f ms] P%u ACK sent\n", now_ms() - t0, seq);
        acked += 1;
    }
    printf("[receiver] all %d packets acknowledged, exiting.\n", NPKT);
    close(sock);
    return 0;
}