#include<stdio.h>
#include<stdlib.h>

long offsetStart=-1,offsetEnd=-1;

int main()
{
	system("insmod process_module.ko");
	
	FILE *fp;
	char ch;
	char buffer[100];

	if((fp=fopen("/var/log/kern.log","r"))==NULL)
	{
		printf("File open error!\n");
		exit(0);
	}


	fseek(fp,0L,SEEK_SET);

	while(!feof(fp))
		if((ch=fgetc(fp))=='@'&&(ch=fgetc(fp))=='@'&&(ch=fgetc(fp))=='@')
			offsetStart=ftell(fp)-3;
	rewind(fp);

	while(!feof(fp))
		if((ch=fgetc(fp))=='!'&&(ch=fgetc(fp))=='!'&&(ch=fgetc(fp))=='!')
			offsetEnd=ftell(fp)-3;
	rewind(fp);

	//printf("%ld %ld\n",offsetStart,offsetEnd);


	fseek(fp,offsetStart,SEEK_SET);

	while(ftell(fp)<offsetEnd)
	{
		fgets(buffer,100,fp);
		printf("%s",buffer);
	}


	if(fclose(fp))
	{
		printf("Can not close the file!\n");
		exit(0);
	}


	system("rmmod process_module.ko");
	return 0;
}
