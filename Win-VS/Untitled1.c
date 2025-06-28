#include <stdio.h>
#include <stdlib.h>

int main(){
	int base='A';
	int i =0;
	for(i=0;i<26; i++)
	{
		printf(" %c=%d\r", base+i, base+i);
		sleep(1);	
	}
}
