#include<stdio.h>
#include <stdlib.h>
#include <string.h>
int myAtoi(char * s) {
    int val = 0;
    int negative = 0;
    while (*s == ' ')
        s++;
    if (*s == '+')
        s++;
    else if (*s == '-') {
        negative = 1;
        s++;
    }

    while(*s >= '0' && *s <= '9') {
        int digit = (int) (*s - '0');
        if (negative) {
            if (val < INT_MIN / 10 || (val == INT_MIN / 10 && digit > 8))
                return INT_MIN;
            val = val * 10 - digit;
        } else {
            if (val > INT_MAX / 10 || (val == INT_MAX / 10 && digit > 7))
                return INT_MAX;
            val = val * 10 + digit;
        }
        s++;
    }

    return val;
}
int main(){
    char *s = "12345";
    printf("%d",myAtoi(s));
    return 0;
}