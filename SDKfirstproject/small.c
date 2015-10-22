#include <stdio.h>
#include <string.h>

int main(){
	char str[1024]="";
	strncpy(str, "Hello World", sizeof(str));
	printf("%s\n", str);
	return(0); 
}
