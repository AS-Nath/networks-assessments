/*
• Subnet mask used
• Size of each subnet
• Network address of each subnet
• First valid host address
• Last valid host address
• Broadcast address
• Number of usable hosts per subnet
192.168.75.0
*/
#include <stdio.h>

int nextpower(int n) {
    int bits = 0;
    while (n > 0) {
        bits += 1;
        n >>= 1; 
    }
    return bits;
}

int main(void) {
    int a;
    int b;
    int c;
    int d;
    scanf("%d.%d.%d.%d", &a, &b, &c, &d);
    int subnetbits = nextpower(5); // as given
    int prefix = 24 + subnetbits; 
    int hostbits = 32 - prefix; 
    int add = (1 << hostbits);  
    int i = 0; 
    int limit = 0; 
    while (limit < 256) {
        printf("Subnet %d\n", i);
        printf("Subnet Size == %d\n", (1 << hostbits)); 
        printf("Network Address : %d.%d.%d.%d\n", a, b, c, limit);  
        printf("First Valid Host Address : %d.%d.%d.%d\n", a, b, c, limit + 1);  
        limit += add; 
        printf("Last Valid Host Address : %d.%d.%d.%d\n\n", a, b, c, limit - 1);  
        i += 1;
    }
    return 0; 
}