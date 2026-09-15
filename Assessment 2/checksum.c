#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

void corrupt(uint16_t* message, int word_idx, int bit_idx) {
    message[word_idx] ^= (1 << bit_idx);
}

uint16_t eval_checksum(int n, uint16_t* message) {
    uint16_t checksum = 0; 
    // Calculate checksum with end-around carry; 
    for (int i = 0; i < n; i++) {
        checksum = ((checksum + message[i]) & (UINT16_MAX)) + ((checksum > UINT16_MAX - message[i]) ? 1 : 0); 
    }
    return ~checksum; 
}

void transmit(int n, uint16_t* message) {
    uint16_t checksum = eval_checksum(n, message); 
    message[n] = checksum;
}

int receive(int n, uint16_t* message) {
    uint16_t c = eval_checksum(n + 1, message); 
    return (c == 0);
}

int main(void) {
    int n; // Number of 16-bit words.
    scanf("%d", &n); 
    uint16_t message[n + 1]; // Extra 2 bytes to store the checksum 
    for (int i = 0; i < n; i++) {
        scanf("%" SCNu16, &message[i]); 
    } 
    transmit(n, message);  
    //corrupt(message, 2, 2); 
    int status = receive(n, message); 
    if (status) {
        printf("Received correctly!\n");
    }
    else {
         printf("Error detected!\n");
    }
    return 0; 
}