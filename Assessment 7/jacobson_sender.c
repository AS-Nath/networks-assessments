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

#define PORT 9090
#define NPKT 8
#define ALPHA 0.125
#define BETA 0.25
#define KFACTOR 4.0
#define MAX_RTO 60000.0
#define INIT_SRTT 1200.0
#define INIT_RTTVAR 350.0

typedef struct {
    uint32_t seq;
    uint32_t attempt;
} packet_t;

typedef struct {
    uint32_t seq;
} ack_t;

typedef struct {
    double sample;
    double srtt;
    double rttvar;
    double rto;
    int retx;
    int used;
} row_t;

static const double trace_rtt[NPKT] = {1050, 1280, 1750, 1420, 980, 1560, 1180, 890};

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void send_pkt(int sock, struct sockaddr_in *dst, uint32_t seq, uint32_t attempt) {
    packet_t p;
    p.seq = htonl(seq);
    p.attempt = htonl(attempt);
    sendto(sock, &p, sizeof p, 0, (struct sockaddr *)dst, sizeof *dst);
}

int main(int argc, char *argv[]) {
    int use_measured = 0;
    if (argc > 1 && strcmp(argv[1], "measured") == 0) {
        use_measured = 1;
    }
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof srv);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

    double srtt = INIT_SRTT;
    double rttvar = INIT_RTTVAR;
    double rto = srtt + KFACTOR * rttvar;
    row_t rows[NPKT];
    int total_tx = 0;
    int retx_count = 0;

    printf("===25BCE2202 : Aditya Shankar Nath===\n");
    printf("=== Jacobson/Karels + Karn-Partridge sender (%s RTT feed) ===\n", use_measured ? "measured" : "trace");
    printf("Initial SRTT = %.2f ms, RTTVAR = %.2f ms\n", srtt, rttvar);
    printf("Initial RTO  = SRTT + 4*RTTVAR = %.2f + 4*%.2f = %.2f ms\n\n", srtt, rttvar, rto);

    double t0 = now_ms();
    for (int i = 0; i < NPKT; i++) {
        uint32_t seq = i + 1;
        uint32_t attempt = 1;
        int retx = 0;
        double t_first = now_ms();
        double t_last = t_first;

        printf("[t=%8.1f ms] P%u SEND    attempt 1 (ORIGINAL)        timer = RTO = %.2f ms\n", t_first - t0, seq, rto);
        send_pkt(sock, &srv, seq, attempt);
        total_tx += 1;

        for (;;) {
            double remaining = rto - (now_ms() - t_last);
            if (remaining <= 0) {
                double old = rto;
                rto *= 2.0;
                if (rto > MAX_RTO) {
                    rto = MAX_RTO;
                }
                retx = 1;
                attempt += 1;
                retx_count += 1;
                printf("[t=%8.1f ms] P%u TIMEOUT (no ACK within %.2f ms)\n", now_ms() - t0, seq, old);
                printf("                    Karn backoff: RTO = 2 x %.2f = %.2f ms\n", old, rto);
                t_last = now_ms();
                send_pkt(sock, &srv, seq, attempt);
                total_tx += 1;
                printf("[t=%8.1f ms] P%u SEND    attempt %u (RETRANSMISSION)  timer = %.2f ms\n", t_last - t0, seq, attempt, rto);
                continue;
            }
            fd_set rf;
            FD_ZERO(&rf);
            FD_SET(sock, &rf);
            struct timeval tv;
            tv.tv_sec = (long)(remaining / 1000.0);
            tv.tv_usec = (long)((remaining - tv.tv_sec * 1000.0) * 1000.0);
            if (select(sock + 1, &rf, NULL, NULL, &tv) > 0) {
                ack_t a;
                ssize_t n = recvfrom(sock, &a, sizeof a, 0, NULL, NULL);
                if (n == (ssize_t)sizeof a && ntohl(a.seq) == seq) {
                    break;
                }
            }
        }

        double t_ack = now_ms();
        double meas_last = t_ack - t_last;
        double meas_first = t_ack - t_first;
        printf("[t=%8.1f ms] P%u ACK received\n", t_ack - t0, seq);
        rows[i].retx = retx;

        if (!retx) {
            double R = use_measured ? meas_last : trace_rtt[i];
            printf("                    measured RTT = %.1f ms, sample = %.2f ms -> Karn-Partridge: VALID\n", meas_last, R);
            double new_var = (1.0 - BETA) * rttvar + BETA * fabs(R - srtt);
            double new_srtt = (1.0 - ALPHA) * srtt + ALPHA * R;
            printf("                    RTTVAR = 0.75*%.4f + 0.25*|%.4f - %.4f| = %.4f\n", rttvar, R, srtt, new_var);
            printf("                    SRTT   = 0.875*%.4f + 0.125*%.4f = %.4f\n", srtt, R, new_srtt);
            rttvar = new_var;
            srtt = new_srtt;
            rto = srtt + KFACTOR * rttvar;
            printf("                    RTO    = %.4f + 4*%.4f = %.4f ms\n", srtt, rttvar, rto);
            rows[i].sample = R;
            rows[i].used = 1;
        }
        else {
            printf("                    ambiguous ACK: %.1f ms since first send, %.1f ms since last send (trace %.0f ms)\n", meas_first, meas_last, trace_rtt[i]);
            printf("                    Karn-Partridge: sample DISCARDED, SRTT/RTTVAR unchanged,\n");
            printf("                    RTO stays backed-off at %.2f ms until next valid sample\n", rto);
            rows[i].sample = trace_rtt[i];
            rows[i].used = 0;
        }
        rows[i].srtt = srtt;
        rows[i].rttvar = rttvar;
        rows[i].rto = rto;
        printf("\n");
    }

    printf("=== SUMMARY TABLE ===\n");
    printf("Packet  RTT(ms)  Retransmitted?  RTT used?  SRTT(ms)   RTTVAR(ms)  RTO(ms)\n");
    for (int i = 0; i < NPKT; i++) {
        printf("P%-6d %-8.0f %-15s %-10s %-10.2f %-11.2f %.2f%s\n", i + 1, rows[i].sample, rows[i].retx ? "Yes" : "No", rows[i].used ? "Yes" : "No", rows[i].srtt, rows[i].rttvar, rows[i].rto, rows[i].retx ? "  (backed-off)" : "");
    }
    printf("\nTotal transmissions on the wire: %d (%d original + %d retransmissions)\n", total_tx, NPKT, retx_count);
    close(sock);
    return 0;
}