#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
unsigned char swap(unsigned char a){

    unsigned char temp=0;
    for(int i=0;i<8;i++){
        temp |= (a&0x01)<<(7-i);
        a = a>>1;
    }
    return temp;
}


void swap_string(char *a){
    char *left = a;
    char *right = a+strlen(a)-1;
    char temp;

    while(left<right){
        temp=*left;
        *left=*right;
        *right =temp;
        left++;
        right--;
    }
}


int endian_convert(int input){

    int temp;
    int size = sizeof(input);
    while(size--){
        temp |=((input &0xFF)<<(size *8));
        input >>=8;
    }
    return temp;

}




int main(void)
{
    int a=0x1234;
    int b=endian_convert(a);
    printf("%d",b);
    return 0;
}