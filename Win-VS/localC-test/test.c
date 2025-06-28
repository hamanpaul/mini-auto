//Pointer 
#include <stdio.h>
#include <string.h>


int main(){
    char str1[20] = "aaa";
    char str2[20] = "vbbbb  ";
    char buf[50];
    strcat(buf,str1);
    printf("out-%s\n", buf);
    strcat(buf,str2);
    printf("out-%s\n", buf);
    return 0;
}