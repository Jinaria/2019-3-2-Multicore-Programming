#include <stdio.h>

bool *flag = new bool(true);

int main(){
	bool* t = new bool(true);
	bool* k = new bool;
	printf("%d\n",*flag);
	printf("%d\n",*k);
}
