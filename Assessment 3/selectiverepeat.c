#include <stdio.h>
#include <string.h>

typedef enum {
    SCENARIO_SUCCESS,
    SCENARIO_LOST_FRAME,
    SCENARIO_LOST_ACK
} Scenario;

Scenario parse_scenario(const char *s) {
    if (strcmp(s, "lost frame") == 0) return SCENARIO_LOST_FRAME;
    if (strcmp(s, "lost ACK") == 0) return SCENARIO_LOST_ACK;
    return SCENARIO_SUCCESS;
}

int main(void) {
    const char *name_line = "Aditya Shankar Nath 25BCE2202";
    int N;
    int window_size;
    int timeout;
    char scenario_input[20];
    printf("%s\n", name_line);
    printf("Enter number of frames: ");
    if (scanf("%d", &N) != 1 || N <= 0 || N > 64) {
        fprintf(stderr, "Invalid number of frames.\n");
        return 1;
    }
    printf("Enter window size: ");
    if (scanf("%d", &window_size) != 1 || window_size <= 0) {
        fprintf(stderr, "Invalid window size.\n");
        return 1;
    }
    printf("Enter scenario (success / lost frame / lost ACK): ");
    if (scanf(" %19[^\n]", scenario_input) != 1) {
        fprintf(stderr, "Invalid scenario input.\n");
        return 1;
    }
    printf("Enter timeout value: ");
    if (scanf("%d", &timeout) != 1 || timeout <= 0) {
        fprintf(stderr, "Invalid timeout value.\n");
        return 1;
    }
    Scenario scenario = parse_scenario(scenario_input);
    int loss_target = N / 2;
    int frame_lost_used = 0;
    int ack_lost_used = 0;
    int base = 0;
    int next_to_send = 0;
    int acked[64];
    int received[64];
    int deadline[64];
    int currentTime = 0;
    for (int i = 0; i < N; i++) {
        acked[i] = 0;
        received[i] = 0;
        deadline[i] = -1;
    }
    while (base < N) {
        while (next_to_send < base + window_size && next_to_send < N) {
            printf("[Time %d] Sender: Sending Frame %d, Timer started (T=%d)\n", currentTime, next_to_send, timeout);
            deadline[next_to_send] = currentTime + timeout;

            int lose_this_frame = (scenario == SCENARIO_LOST_FRAME && next_to_send == loss_target && !frame_lost_used);

            if (lose_this_frame) {
                frame_lost_used = 1;
                printf("[Time %d] Frame %d lost in transit\n", currentTime, next_to_send);
            }
            else {
                int arrive_time = currentTime + 2;
                printf("[Time %d] Receiver: Frame %d received -> Buffered\n", arrive_time, next_to_send);
                received[next_to_send] = 1;

                int lose_this_ack = (scenario == SCENARIO_LOST_ACK && next_to_send == loss_target && !ack_lost_used);

                if (lose_this_ack) {
                    ack_lost_used = 1;
                    printf("[Time %d] ACK %d lost in transit\n", arrive_time, next_to_send);
                }
                else {
                    int ack_arrive = arrive_time + 1;
                    printf("[Time %d] Sender: ACK %d received, Timer stopped\n", ack_arrive, next_to_send);
                    acked[next_to_send] = 1;
                    deadline[next_to_send] = -1;
                }
            }
            currentTime += 1;
            next_to_send += 1;
        }
        while (base < N && acked[base]) {
            base += 1;
        }
        if (base >= N) {
            break;
        }
        int retransmit_time = -1;
        for (int i = base; i < next_to_send; i++) {
            if (!acked[i] && deadline[i] != -1) {
                if (retransmit_time == -1 || deadline[i] < retransmit_time) {
                    retransmit_time = deadline[i];
                }
            }
        }
        if (retransmit_time == -1) {
            continue;
        }
        if (retransmit_time > currentTime) {
            currentTime = retransmit_time;
        }
        for (int i = base; i < next_to_send; i++) {
            if (!acked[i] && deadline[i] == retransmit_time) {
                printf("[Time %d] Sender: Timer expired for Frame %d, Retransmitting\n", currentTime, i);
                deadline[i] = currentTime + timeout;
                int arrive_time = currentTime + 2;
                if (!received[i]) {
                    printf("[Time %d] Receiver: Frame %d received -> Buffered\n", arrive_time, i);
                    received[i] = 1;
                }
                else {
                    printf("[Time %d] Receiver: Duplicate Frame %d received -> Discarding, Resending ACK %d\n", arrive_time, i, i);
                }
                int ack_arrive = arrive_time + 1;
                printf("[Time %d] Sender: ACK %d received, Timer stopped\n", ack_arrive, i);
                acked[i] = 1;
                deadline[i] = -1;
                if (ack_arrive > currentTime) {
                    currentTime = ack_arrive;
                }
            }
        }
    }
    printf("\nAll %d frames successfully delivered.\n", N);
    printf("%s\n", name_line);
    return 0;
}