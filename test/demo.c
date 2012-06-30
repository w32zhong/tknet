#include "../tknet.h"

int main(int n,char **args)
{
	return tkNetMain(n,args);
}

void
ON_CONNECT()
{
	printf("ON_CONNECT() called.\n");
}
