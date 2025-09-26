#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
int main(int argc, char** argv)
{
	int opCode = 0;
	int* numBuffer;
		
		
	//Check arguments count	
	if(argc < 2) 
	{
		printf("Insufficient arguments.\r\n");
		fflush(stdout);
		return 1;
	}
	

	 //Check for opcode (Read or Write)
	if(*(argv[1]) == 'W')
		opCode = 1;
	else if (*(argv[1]) == 'R')
		opCode = 2;
	else
	{
		printf("Unknown opcode.\r\n");
		fflush(stdout);
		return 1;
	}
	
	
	//Check for opcode arguments
	if (argc == 2)   
	{
		if((*(argv[1]) == 'R')) //Number of readings should be specified
			printf("Number of readings is not specified.\r\n");
		else if((*(argv[1]) == 'W')) //List of values should be specified
			printf("Nothing to write.\r\n");
		fflush(stdout);
		return 1;
	}
	

	
	int count = 0;
	
	//Count values to read or write
	if(opCode == 1) //Write
	{
		count = (argc - 2);
		numBuffer = (int*)malloc( count * sizeof(int) ); //Allocate temp buffer
		if(!numBuffer)
		{
			printf("Memory allocation error.\r\n");
			fflush(stdout);
			return 1;
		}
		
		for(int i=0; i<(argc - 2); ++i)
		{
			sscanf(argv[2 + i], "%d", &numBuffer[i]);
		}
	}
	else if(opCode == 2) //Read
	{
		sscanf(argv[2], "%d", &count); //Get number of readings
		
		if(count < 1)
		{
			printf("Invalid readings count.\r\n");
			fflush(stdout);
			return 1;
		}
		
		
		numBuffer = (int*)malloc( count * sizeof(int) ); //Allocate temp buffer
		if(!numBuffer)
		{
			printf("Memory allocation error.\r\n");
			fflush(stdout);
			return 1;
		}
	}
	


	int f ;
	
	if(opCode == 1)
	{
		f = open("/dev/stackdev", O_WRONLY); //Open device for writing
	}
	else if(opCode == 2)
	{
		f = open("/dev/stackdev", O_RDONLY); //Open device for reading
	}
	
	if(f < 0)
	{
		printf("Cannot open device.\r\n");
		fflush(stdout);
		free(numBuffer);
		return 1;
	}
	
	printf("Device opened successfully.\r\n");
	
	
	int value;
	
	
	if(opCode == 1) //Write
	{
		int res = write(f, (char*)numBuffer, count * sizeof(int)); //Write buffered values
		
		if(res < 0)
		{
			close(f);
			
			if(errno == ENOMEM)
			{
				printf("Cannot write to the device. Device buffer is full.\r\n");
			}
			else if(errno == ENOSYS)
			{
				printf("Cannot write to the device. Invalid data size.\r\n");
			}
			else
			{
				printf("Cannot write to the device. Unknown error.\r\n");
			}
			
			fflush(stdout);
			free(numBuffer);
			return 2;
		}
		else //At least one value has been written
		{
			printf("%d value(s) written successfully: ", res / sizeof(int));
			for(int i=0; i < res / sizeof(int); ++i)
			{
				printf("%d ", numBuffer[i]);
			}
			
			printf("\r\n");
		}
	}
	else if(opCode == 2) //Read
	{
		int res;
		
		res = read(f, (char*)numBuffer, count * sizeof(int)); //Read values to the buffer
		
		if(res < 0) //Module error
		{
			close(f);
			
			if(errno == EINVAL)
			{
				printf("Device buffer is empty.\r\n");
			}
			else if(errno == ENOSYS)
			{
				printf("Cannot read from the device. Invalid data size.\r\n");
			}
			else
			{
				printf("Cannot read from the device. Unknown error.\r\n");
			}
			
			fflush(stdout);
			free(numBuffer);
			return 2;
		}
		else //At least one value has been read
		{
			printf("%d value(s) read successfully: ", res / sizeof(int));
			for(int i=0; i < (res / sizeof(int)); ++i)
			{
				printf("%d ", numBuffer[i]);
			}
			
			printf("\r\n");
		}
		
	}
	
	free(numBuffer); //Free allocated memory

	close(f); //Close device
	
	
	fflush(stdout); //flush stdout

	
	return 0;
	
	
}