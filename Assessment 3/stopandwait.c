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

int simulate_frame(int sequence, int expected, int timeout, Scenario scenario, int *frame_lost_used, int *ack_lost_used, int *currentTime) {
    int t = *currentTime;
    printf("\n[Time %d] Sender: Sending Frame %d, Timer started (T=%d)\n", t, sequence, timeout);
    if (scenario == SCENARIO_LOST_FRAME && !*frame_lost_used) {
        *frame_lost_used = 1;
        printf("[Time %d] Frame %d lost in transit\n", t, sequence);
        t += timeout;
        printf("[Time %d] Sender: Timer expired (No ACK)\n", t);
        printf("[Time %d] Sender: Retransmitting Frame %d, Timer restarted (T=%d)\n", t, sequence, timeout);
        t += 2;
    }
    else {
        t += 2;
    }
    int new_expected = 1 - expected;
    printf("[Time %d] Receiver: Frame %d received -> Sending ACK %d\n", t, sequence, new_expected);
    if (scenario == SCENARIO_LOST_ACK && !*ack_lost_used) {
        *ack_lost_used = 1;
        printf("[Time %d] ACK %d lost in transit\n", t, new_expected);
        t += timeout;
        printf("[Time %d] Sender: Timer expired (No ACK)\n", t);
        printf("[Time %d] Sender: Retransmitting Frame %d, Timer restarted (T=%d)\n", t, sequence, timeout);
        t += 2;
        printf("[Time %d] Receiver: Duplicate Frame %d received -> Discarding -> Resending ACK %d\n", t, sequence, new_expected);

        t += 1;
        printf("[Time %d] Sender: ACK %d received, Timer stopped\n", t, new_expected);
    }
    else {
        t += 1;
        printf("[Time %d] Sender: ACK %d received, Timer stopped\n", t, new_expected);
    }
    *currentTime = t;
    return new_expected;
}

int main(void) {
    const char *name_line = "Aditya Shankar Nath 25BCE2202";
    int N;
    int timeout;
    char scenario_input[20];
    printf("%s\n", name_line);
    printf("Enter number of frames: ");
    if (scanf("%d", &N) != 1 || N <= 0) {
        fprintf(stderr, "Invalid number of frames.\n");
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
    int sequence = 0;
    int expected = 0;
    int delivered = 0;
    int currentTime = 0;
    int frame_lost_used = 0;
    int ack_lost_used = 0; 
    while (delivered < N) {
        expected = simulate_frame(sequence, expected, timeout, scenario, &frame_lost_used, &ack_lost_used, &currentTime);
        delivered += 1;
        sequence = 1 - sequence;
    }
    printf("\nAll %d frames successfully delivered.\n", N);
    printf("%s\n", name_line);
    return 0;
}