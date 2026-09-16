#include <stdio.h>

#define MAX_FRAMES 64
#define MAX_LOSSES 64

/*
 * Marks index `idx` as "not yet consumed" in a used-flags array of the
 * given losses list. Returns 1 and consumes it if idx is present and
 * unused; returns 0 otherwise (idx not in the list, or already used).
 */
int consume_loss(const int *losses, int num_losses, int *used, int idx) {
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
        printf("  %s number to lose (occurrence %d of %d): ", label, i + 1, n);
        if (scanf("%d", &losses[i]) != 1 || losses[i] < 0) {
            fprintf(stderr, "Invalid %s number.\n", label);
            return -1;
        }
    }
    return n;
}

int send_frame(int frame_num, int send_time, const int *frame_losses, int num_frame_losses, int *frame_used) {
    printf("[Time %d] Sender: Sending Frame %d\n", send_time, frame_num);

    if (consume_loss(frame_losses, num_frame_losses, frame_used, frame_num)) {
        printf("[Time %d] Frame %d lost in transit\n", send_time, frame_num);
        return -1;
    }

    int arrive_time = send_time + 2;
    printf("[Time %d] Receiver: Frame %d received\n", arrive_time, frame_num);
    return arrive_time;
}

int send_ack(int ack_num, int send_time, const int *ack_losses, int num_ack_losses, int *ack_used) {
    printf("[Time %d] Receiver: Sending ACK %d (cumulative)\n", send_time, ack_num);
    if (consume_loss(ack_losses, num_ack_losses, ack_used, ack_num)) {
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
    int frame_losses[MAX_LOSSES];
    int ack_losses[MAX_LOSSES];
    int frame_used[MAX_LOSSES] = {0};
    int ack_used[MAX_LOSSES] = {0};
    int arrival[MAX_FRAMES];
    printf("%s\n", name_line);
    printf("Enter number of frames: ");
    if (scanf("%d", &N) != 1 || N <= 0 || N > MAX_FRAMES) {
        fprintf(stderr, "Invalid number of frames (max %d).\n", MAX_FRAMES);
        return 1;
    }
    printf("Enter window size: ");
    if (scanf("%d", &window_size) != 1 || window_size <= 0) {
        fprintf(stderr, "Invalid window size.\n");
        return 1;
    }
    printf("Enter timeout value: ");
    if (scanf("%d", &timeout) != 1 || timeout <= 0) {
        fprintf(stderr, "Invalid timeout value.\n");
        return 1;
    }
    printf("Frame numbers are 0-based, 0..%d.\n", N - 1);
    printf("ACK numbers are the cumulative frame number being acknowledged, 0..%d.\n", N - 1);
    int num_frame_losses = read_loss_list("frame", frame_losses, MAX_LOSSES);
    if (num_frame_losses < 0) {
        return 1;
    }
    int num_ack_losses = read_loss_list("ACK", ack_losses, MAX_LOSSES);
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
    int base = 0;
    int next_to_send = 0;
    int currentTime = 0;
    while (base < N) {
        while (next_to_send < N && next_to_send < base + window_size) {
            arrival[next_to_send] = send_frame(next_to_send, currentTime, frame_losses, num_frame_losses, frame_used);
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
            int ack_arrive = send_ack(highest_in_order, currentTime, ack_losses, num_ack_losses, ack_used);
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