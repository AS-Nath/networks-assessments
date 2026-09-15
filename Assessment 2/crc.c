#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int locate_msb(uint64_t n) {
    int p = 0; 
    while (n > 1) {
        p += 1; 
        n >>= 1; 
    }
    return p; 
}

void print_binary(uint64_t n) {
    int idx = locate_msb(n); 
    while (idx > -1) {
        printf("%" PRIu64, ((n >> idx) & 1));
        idx -= 1;  
    }
    printf("\n"); 
}

uint16_t generate_remainder(uint64_t message, uint32_t generator) {
    int msb = locate_msb(message); 
    int window = msb - 16; 
    uint64_t temp = message >> window; 
    while (window > 0) {
        if (message & (1ULL << msb)) {
            temp ^= generator;
        }
        msb -= 1;
        window -= 1; 
        temp = ((temp << 1) | ((message >> window) & 1));
    }
    return (uint16_t)temp; 
}

int receive(uint64_t message, uint32_t generator) {
    uint16_t rem = generate_remainder(message, generator); 
    return !rem;
}

uint64_t transmit(uint64_t message, uint32_t generator) {
    message |= generate_remainder(message, generator);
    return message; 
}

void corrupt(uint64_t* n, int bit) {
    uint64_t t = *n;
    t ^= (1ULL << bit); 
    *n = t;  
}

int main(void) {
    uint64_t message; 
    uint32_t generator; 
    scanf("%" SCNu64, &message); 
    scanf("%" SCNu32, &generator); 
    message <<= 16; // CRC-16 code
    print_binary(message); 
    message = transmit(message, generator);  
    print_binary(message); 
    //corrupt(&message, 2); 
    int status = receive(message, generator); 
    if (status) {
        printf("Received!\n"); 
    }
    else {
        printf("Error detected!\n"); 
    }
}