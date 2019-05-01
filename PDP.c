#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem [64 * 1024];

word w_read  (adr a);
void w_write (adr a, word val);
byte b_read  (adr a);  
void b_write (adr a, byte val);

void load_file (char * file);

byte b_read(adr a)
{
	return mem[a];
}

void b_write (adr a, byte val)
{
    mem[a] = val;
}

word w_read (adr a)
{
    assert(a % 2 == 0);
    word wrd1= mem[a];
    word wrd2 = mem[a + 1];
    wrd2 <<= 8;
    return wrd1 + wrd2;
}


void w_write (adr a, word val)
{

	assert((a % 2) == 0);
	mem[a] = (byte) val;
	mem[a + 1] = (byte) (val >> 8);
}



void load_file(char * file)
{
	int size = strlen(file);
	if(size == 0)
		exit(0);

	unsigned int a, b, val;
	int i = 0;
	FILE * f = fopen(file, "r");
	if (f == NULL) {
		perror(file);
		exit(1);
	}
	while(fscanf(f, "%x%x", &a, &b) == 2)
	{
		for(i = a; i < (a + b); i++)
		{
			fscanf(f, "%x", &val);
			b_write(i, val);
		}
	}
	fclose(f);
}

int main (int argc, char ** argv)
{
	mem[0177564] = -1;
	fprintf(stderr, "PDP is ready\n");

    load_file(argv[1]);
   // run(01000, argv);
    return 0;
}
