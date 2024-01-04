#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>


int main(void) {
	uint32_t * p;
	int i=0;
	int fd = open("/dev/mem", O_RDWR);
	p = (uint32_t*)mmap(NULL, 4, PROT_WRITE|PROT_READ, MAP_SHARED,fd, 0xFF203000);
	
	while (1)
	{
		for (i=1;i<10;i++)
		{
			*p = (1<<i);
			usleep(500000);
			*p = (0<<i);
		}
	}
	
	return 0;
	
}
