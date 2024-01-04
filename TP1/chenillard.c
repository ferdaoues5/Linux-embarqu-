#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>


int main(void) {
	char led_enable = '1';
	char led_disable = '0';
	int i=1;
	char path[]="/sys/class/leds/fpga_led1/brightness";
	int fid[9];
	printf("Chenillard \n");
	//24
	for (i=0;i<9;i++)
	{
		path[24]=(char) (i+1+'0');
		fid[i]= open(path, O_CREAT | O_RDWR | O_APPEND | O_TRUNC , S_IRWXU);
	}

	while(1)
	{
		for (i=1;i<10;i++)
		{
			if (fid[i-1]<0)
			{
				printf("Error opening file brightness\n");
				return -1;
			}
			if(write(fid[i-1], &led_enable, sizeof(led_enable))!=sizeof(led_enable))
			{
				printf("Error writing in file brightness\n");
				return -1;
			}
			
			usleep(500000);// sleep en microsec *
			if(write(fid[i-1], &led_disable, sizeof(led_disable))!=sizeof(led_disable))
			{
				printf("Error writing in file brightness\n");
				return -1;
			}
		}
	}
	
	return 0;
}
