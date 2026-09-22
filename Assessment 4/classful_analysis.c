/*
 Class
• Default mask
• Size of the network
• Network address
• Broadcast address
• First usable host address
• Last usable host address
*/

#include <stdio.h>

int main(void) {
    int a;
    int b;
    int c;
    int d; 
    scanf("%d.%d.%d.%d", &a, &b, &c, &d); 
    int prefix = 0; 
    if (a < 128) {
        // Class A
        printf("Class A\n"); 
        printf("/8 mask\n"); 
        printf("Network Address : %d.0.0.0\n", a); 
        printf("Broadcast Address : %d.255.255.255\n", a); 
        printf("First Usable Host : %d.0.0.1\n", a);
        printf("Last Usable Host : %d.255.255.254\n", a);  
    }
    else if (a < 192) {
        // Class B
        printf("Class B\n"); 
        printf("/16 mask\n"); 
        printf("Network Address : %d.%d.0.0\n", a, b);
        printf("Broadcast Address : %d.%d.255.255\n", a, b); 
        printf("First Usable Host : %d.%d.0.1\n", a, b);
        printf("Last Usable Host : %d.%d.255.254\n", a, b); 
    }
    else {
        // Class C, goes till 223 
        // Ignoring everything above. 
        printf("Class C\n");
        printf("/24 mask\n");  
        printf("Network Address : %d.%d.%d.0\n", a, b, c); 
        printf("Broadcast Address : %d.%d.%d.255\n", a, b, c);
        printf("First Usable Host : %d.%d.%d.1\n", a, b, c);  
        printf("Last Usable Host : %d.%d.%d.254\n", a, b, c);
    }
    return 0; 
}