#include <stdio.h>

main(int argc, char **argv)
{
FILE	*fp;
char	line[100];
char	c;

	fp = stdin;
	while(!feof(fp)){
	c = getchar();
	fprintf(stderr,"%c",c);
	}
}
