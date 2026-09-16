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

int send_frame(int frame_num, int send_time, Scenario scenario, int loss_target, int *frame_lost_used) {
    printf("[Time %d] Sender: Sending Frame %d\n", send_time, frame_num);
    if (scenario == SCENARIO_LOST_FRAME && frame_num == loss_target && !*frame_lost_used) {
        *frame_lost_used = 1;
        printf("[Time %d] Frame %d lost in transit\n", send_time, frame_num);
        return -1;
    }
    int arrive_time = send_time + 2;
    printf("[Time %d] Receiver: Frame %d received\n", arrive_time, frame_num);
    return arrive_time;
}

int send_ack(int ack_num, int send_time, Scenario scenario, int loss_target, int *ack_lost_used) {
    printf("[Time %d] Receiver: Sending ACK %d (cumulative)\n", send_time, ack_num);
    if (scenario == SCENARIO_LOST_ACK && ack_num == loss_target && !*ack_lost_used) {
        *ack_lost_used = 1;
        printf("[Time %d] ACK %d lost in transit\n", send_time, ack_num);
        return -1;
    }
    int arrive_time = send_time + 1;
    printf("[Time %d] Sender: ACK %d received, base advanced to %d\n", arrive_time, ack_num, ack_num + 1);
    return arrive_time;
}

int main(void) {
    const char *name_line = "Aditya Shankar Nath 25BCE2202";
    int N;
    int window_size;
    int timeout;
    char scenario_input[20];
    printf("%s\n", name_line);
    printf("Enter number of frames: ");
    if (scanf("%d", &N) != 1 || N <= 0) {
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
    int base = 0;       
    int next_to_send = 0; 
    int currentTime = 0;
    int frame_lost_used = 0;
    int ack_lost_used = 0;
    int arrival[64];      
    if (N > 64) {
        fprintf(stderr, "This simulator supports at most 64 frames.\n");
        return 1;
    }
    for (int i = 0; i < N; i++) {
        arrival[i] = -1;
    }
    while (base < N) {
        while (next_to_send < N && next_to_send < base + window_size) {
            arrival[next_to_send] = send_frame(next_to_send, currentTime, scenario, loss_target, &frame_lost_used);
            currentTime += 2;
            next_to_send += 1;
        }
        int highest_in_order = base - 1;
        int i = base;
        while (i < next_to_send && arrival[i] != -1) {
            highest_in_order = i;
            i += 1;
        }
        if (highest_in_order >= base) {
            int ack_arrive = send_ack(highest_in_order, currentTime, scenario, loss_target, &ack_lost_used);
            currentTime += 1;
            if (ack_arrive != -1) {
                base = highest_in_order + 1;
                continue;
            }
        }
        currentTime += timeout;
        printf("[Time %d] Sender: Timer expired for Frame %d, going back to Frame %d\n", currentTime, base, base);
        next_to_send = base;
        for (int j = base; j < N; j++) {
            arrival[j] = -1;
        }
    }

    printf("\nAll %d frames successfully delivered.\n", N);
    printf("%s\n", name_line);
    return 0;
}