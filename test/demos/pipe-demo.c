#include "tknet.h"

int main()
{
	char buff[256],*pArg0,*pArg1;
	struct pipe *pPipe0,*pPipe1;
	uint i;
	char get_c;

	PipeModuleInit();

	while(1)
	{
		fgets(buff,255,stdin);
		buff[strlen(buff)-1] = '\0';
		
		pArg0 = strtok(buff," ");
		pArg1 = strtok(NULL," ");
		
		if(pArg0 == NULL)
			continue;

		if(pArg1)
		{
			if(strcmp(pArg0,"dele") == 0)
			{
				pPipe1 = PipeFindByName(pArg1);
				
				if(pPipe1)
				{
					PipeDele(pPipe1);
				}
				else
				{
					printf("can't find %s.\n",pArg1);
				}
			}
			else
			{
				pPipe0 = PipeMap(pArg0,(char**)&buff,&i,PIPE_WRITE);
				pPipe1 = PipeFindByName(pArg1);

				if(pPipe1)
				{
					printf("only? y/n \n");
					get_c = getchar();
					if(get_c=='y')
						PipeDirectOnlyTo(pPipe0,pPipe1);
					else
						PipeDirectTo(pPipe0,pPipe1);

					printf("\n");
				}
				else
				{
					printf("can't find %s.\n",pArg1);
				}
			}
		}
		else if(strcmp(pArg0,"exit") == 0)
		{
			break;
		}
		else
		{
			PipeMap(pArg0,(char**)&buff,&i,PIPE_READ);
		}

		PipeTablePrint();
	}

	PipeModuleUninit();
	printf("unfree memory:%d \n",g_allocs);
	return 0;
}
