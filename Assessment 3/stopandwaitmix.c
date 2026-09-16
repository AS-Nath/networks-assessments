#include <stdio.h>

#define MAX_FRAMES 64
#define MAX_LOSSES 64

static int consume_loss(const int *losses, int num_losses, int *used, int idx) {
    for (int i = 0; i < num_losses; i++) {
        if (losses[i] == idx && !used[i]) {
            used[i] = 1;
            return 1;
        }
    }
    return 0;
}

int read_loss_list(const char *label, int *losses, int max_n) {
    int n;
    printf("Enter number of %s to lose: ", label);
    if (scanf("%d", &n) != 1 || n < 0 || n > max_n) {
        fprintf(stderr, "Invalid count for %s losses.\n", label);
        return -1;
    }
    for (int i = 0; i < n; i++) {
        printf("  %s sequence number to lose (occurrence %d of %d): ", label, i + 1, n);
        if (scanf("%d", &losses[i]) != 1 || losses[i] < 0) {
            fprintf(stderr, "Invalid %s sequence number.\n", label);
            return -1;
        }
    }
    return n;
}

int simulate_frame(int frame_index, int sequence, int expected, int timeout, const int *frame_losses, int num_frame_losses, int *frame_used, const int *ack_losses, int num_ack_losses, int *ack_used, int *currentTime) {
    int t = *currentTime;
    printf("\n[Time %d] Sender: Sending Frame %d, Timer started (T=%d)\n", t, sequence, timeout);
    if (consume_loss(frame_losses, num_frame_losses, frame_used, frame_index)) {
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
    if (consume_loss(ack_losses, num_ack_losses, ack_used, frame_index)) {
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
    int frame_losses[MAX_LOSSES];
    int ack_losses[MAX_LOSSES];
    int frame_used[MAX_LOSSES] = {0};
    int ack_used[MAX_LOSSES] = {0};
    printf("%s\n", name_line);
    printf("Enter number of frames: ");
    if (scanf("%d", &N) != 1 || N <= 0 || N > MAX_FRAMES) {
        fprintf(stderr, "Invalid number of frames (max %d).\n", MAX_FRAMES);
        return 1;
    }
    printf("Enter timeout value: ");
    if (scanf("%d", &timeout) != 1 || timeout <= 0) {
        fprintf(stderr, "Invalid timeout value.\n");
        return 1;
    }
    printf("Frame indices are 0-based, 0..%d, in the order they are first sent.\n", N - 1);
    int num_frame_losses = read_loss_list("frames", frame_losses, MAX_LOSSES);
    if (num_frame_losses < 0) {
        return 1;
    }
    int num_ack_losses = read_loss_list("ACKs", ack_losses, MAX_LOSSES);
    if (num_ack_losses < 0) {
        return 1;
    }
    for (int i = 0; i < num_frame_losses; i++) {
        if (frame_losses[i] >= N) {
            fprintf(stderr, "Frame loss index %d is out of range (N=%d).\n", frame_losses[i], N);
            return 1;
        }
    }
    for (int i = 0; i < num_ack_losses; i++) {
        if (ack_losses[i] >= N) {
            fprintf(stderr, "ACK loss index %d is out of range (N=%d).\n", ack_losses[i], N);
            return 1;
        }
    }
    int sequence = 0;
    int expected = 0;
    int delivered = 0;
    int currentTime = 0;
    while (delivered < N) {
        expected = simulate_frame(delivered, sequence, expected, timeout, frame_losses, num_frame_losses, frame_used, ack_losses, num_ack_losses, ack_used, &currentTime);
        delivered += 1;
        sequence = 1 - sequence;
    }
    printf("\nAll %d frames successfully delivered.\n", N);
    printf("%s\n", name_line);
    return 0;
}