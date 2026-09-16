#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

void corrupt(uint64_t* n, int b) {
    // b is a Hamming bit. 
    b -= 1;
    uint64_t t = *n;
    t ^= (1ULL << b); 
    *n = t; 
}

int locate_msb(uint64_t n) {
    int p = 0;
    while (n > 1) {
        p += 1;
        n >>= 1;
    }
    return p; 
}

int eval_parity(uint64_t message) {
    int m = locate_msb(message) + 1; 
    int r = 0; 
    while ((1ULL << r) < m + r + 1) {
        r += 1; 
    }
    return r; 
}

uint64_t transmit(uint64_t message) {
    int numparity = eval_parity(message); 
    int max_original = locate_msb(message);
    int original_idx = 0;
    int enc_idx = 0;
    uint64_t encoded = 0;
    int taken = 0; 
    while (original_idx <= max_original) {
        if ((enc_idx & (enc_idx + 1))) {
            encoded |= (((message >> original_idx) & 1) << enc_idx);
            original_idx += 1;
        }
        else {
            taken += 1; 
        }
        enc_idx += 1;
    }
    int totalcap = max_original + numparity; 
    int currshift = 1; 
    for (int current_place = 0; current_place < numparity; current_place++) {
        uint64_t current_xor = 0; 
        for (int i = 1; i <= totalcap + 1; i++) {
            if ((i & (1 << current_place)) && (i & (i - 1))) {
                current_xor ^= ((encoded >> (i - 1)) & 1); 
            }
        }
        encoded |= (current_xor << (currshift - 1));
        currshift <<= 1; 
    }
    return encoded; 
}

int receive(uint64_t message, int numparity, int totalcap) {
    int max_original = totalcap - numparity;  
    int currshift = 1; 
    uint8_t syndrome = 0; 
    for (int current_place = 0; current_place < numparity; current_place++) {
        uint64_t current_xor = 0; 
        for (int i = 1; i <= totalcap + 1; i++) {
            if (i & (1ULL << current_place)) {
                current_xor ^= ((message >> (i - 1)) & 1); 
            }
        }
        syndrome |= (current_xor << current_place);
        currshift <<= 1; 
    }
    if (!syndrome) {
        return 1; 
    }
    else {
        printf("Bit %d\n", syndrome); 
        message ^= (1ULL << (syndrome - 1));
        printf("%" PRIu64 "\n", message); 
        uint64_t original = 0;
        int data_idx = 0;
        for (int i = 1; i <= totalcap + 1; i++) {
            if ((i & (i - 1)) != 0) {
                original |= ((message >> (i - 1)) & 1) << data_idx;
                data_idx += 1;
            }
        }
        printf("%" PRIu64 "\n", original);
        return 0;  
    }
}

int main(void) {
    uint64_t message; 
    scanf("%" SCNu64, &message); 
    int parity = eval_parity(message); 
    int totalcap = locate_msb(message) + parity; 
    message = transmit(message); 
    corrupt(&message, 5); 
    int status = receive(message, parity, totalcap); 
    if (status) {
        printf("Received!\n"); 
    }
    else {
        printf("Error Corrected!\n"); 
    }
    return 0; 
}