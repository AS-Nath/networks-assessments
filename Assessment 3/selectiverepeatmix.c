#include <stdio.h>

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

int main(void) {
    const char *name_line = "Aditya Shankar Nath 25BCE2202";
    int N;
    int window_size;
    int timeout;
    int frame_losses[64];
    int ack_losses[64];
    int frame_used[64] = {0};
    int ack_used[64] = {0};
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
    printf("Enter timeout value: ");
    if (scanf("%d", &timeout) != 1 || timeout <= 0) {
        fprintf(stderr, "Invalid timeout value.\n");
        return 1;
    }
    printf("Frame numbers are 0-based, 0..%d.\n", N - 1);
    int num_frame_losses = read_loss_list("frame", frame_losses, 64);
    if (num_frame_losses < 0) {
        return 1;
    }
    int num_ack_losses = read_loss_list("ACK", ack_losses, 64);
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
            if (consume_loss(frame_losses, num_frame_losses, frame_used, next_to_send)) {
                printf("[Time %d] Frame %d lost in transit\n", currentTime, next_to_send);
            }
            else {
                int arrive_time = currentTime + 2;
                printf("[Time %d] Receiver: Frame %d received -> Buffered\n", arrive_time, next_to_send);
                received[next_to_send] = 1;
                if (consume_loss(ack_losses, num_ack_losses, ack_used, next_to_send)) {
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
                if (consume_loss(frame_losses, num_frame_losses, frame_used, i)) {
                    printf("[Time %d] Frame %d lost in transit\n", currentTime, i);
                    continue;
                }
                int arrive_time = currentTime + 2;
                if (!received[i]) {
                    printf("[Time %d] Receiver: Frame %d received -> Buffered\n", arrive_time, i);
                    received[i] = 1;
                }
                else {
                    printf("[Time %d] Receiver: Duplicate Frame %d received -> Discarding, Resending ACK %d\n", arrive_time, i, i);
                }
                if (consume_loss(ack_losses, num_ack_losses, ack_used, i)) {
                    printf("[Time %d] ACK %d lost in transit\n", arrive_time, i);
                    if (arrive_time > currentTime) {
                        currentTime = arrive_time;
                    }
                    continue;
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