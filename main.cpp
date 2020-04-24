#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include "area.h"
#include "utils.h"
#include <libconfig.h++>


using namespace std;
using namespace libconfig;

int main(int argc,char *argv[])
{
	int world_size;
	int rank;
	int volume=0;
	int initial=0;
	char configFileName[256];
	int epochs=0;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	memset(configFileName,0,255);
	if(!rank)
	{
		for(int i=1;i<argc;i++)
		{
			if(argv[i][0]=='-')
			{
				switch(argv[i][1])
				{
					case 'f'://config file
						strcpy(configFileName,argv[++i]);
						break;
					case 'e'://number of epochs
						epochs=atoi(argv[++i]);
						break;
					default://
						break;
				}
			}
		}
		
	}

	MPI_Bcast(configFileName,255,MPI_CHAR,0,MPI_COMM_WORLD);
	MPI_Bcast(&epochs,1,MPI_INTEGER,0,MPI_COMM_WORLD);

	AreaRank population;
	population.initialise(configFileName);

	for(int i=0;i<epochs;i++)
	{
		population.next_epoch();
	}
	MPI_Finalize();
	return 0;
}

