/*
• Prefix length
• Subnet mask
• Total number of addresses
• Total number of usable hosts
• Network address
• Broadcast address
• First usable IP address
• Last usable IP address
10.25.67.140/21
*/
#include <stdio.h>

int main(void) {
    int a;
    int b;
    int c;
    int d;
    int prefix;
    scanf("%d.%d.%d.%d/%d", &a, &b, &c, &d, &prefix); 
    printf("Prefix Length : %d\n", prefix); 
    printf("Subnet Mask : "); 
    int copy = prefix;
    int count = 0;
    while (copy >= 8) { 
        copy -= 8; 
        count += 1; 
        printf("255."); 
    }
    printf("%d", 256 - (1 << (8 - copy))); 
    count += 1;
    while (count < 4) {
        printf(".0"); 
        count += 1;
    }
    printf("\n"); 
    printf("Total Number of Addresses == %d\n", (1 << (32 - prefix)));
    printf("Total Number of Usable Hosts == %d\n", (1 << (32 - prefix)) - 2); 
}