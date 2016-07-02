#include <stdio.h>


int main(int Length, char *array[])  
{
	int i;
	printf("Length=%d\n",Length);
	for(i=0;i<Length;i++)
	{
		printf("array[%d]=%s\n",i,array[i]);
	}
}

