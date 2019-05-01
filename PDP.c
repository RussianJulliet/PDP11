#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem [64 * 1024];

#define pc reg[7]
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1 << 1) //2
#define HAS_NN (1 << 2) //4
#define HAS_XX (1 << 3) //8

word w_read  (adr a);
void w_write (adr a, word val);
byte b_read  (adr a);  
void b_write (adr a, byte val);

void load_file (char * file);
struct P_Command create_command (word w);
void run (adr pc0, char ** argv);

struct P_Command
{
	word w;       // word
	int B;        // Byte
	word command; // opcode
	word mode_r1; //mode 1 operand
	word r1;      // 1 operand
	word mode_r2; // mode 2 operand
	word r2;      // 2 operand
};

struct Command
{
	word opcode;                       //opcode
	word mask;                         //mask
	char * name;                       //name of command
	void (* func)(struct P_Command PC);//do_command
	byte param;                        //parametr of commmand
} commands [] =
{
};

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

struct P_Command create_command(word w)
{
	struct P_Command c;
	c.w = w;
	c.B = (w >> 15);
	c.command = (w >> 12) & 15;
	c.mode_r1 = (w >> 9) & 7;
	c.r1 = (w >> 6) & 7;
	c.mode_r2 = (w >> 3) & 7;
	c.r2 = w & 7;
	return c;
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

void run(adr pc0, char ** argv)
{
	int i;
	pc = (word)pc0;                           
	while(1)
	{
		word w = w_read(pc);
		pc += 2;
		struct P_Command PC = create_command(w);
		for(i = 0; i <= sizeof(commands)/sizeof(struct Command); i ++)   
		{
			struct Command cmd = commands[i];
			if((w & commands[i].mask) == commands[i].opcode)
			{
				printf("%06o : %06o\t", pc - 2, w );
				printf("%s ", commands[i].name);
				if (cmd.param & HAS_SS)
				{
					if(commands[i].name == "mul")
					{
						ss = get_mode (PC.r2, PC.mode_r2, PC.B);
						printf (", ");
					}
					else
					{	
                        ss = get_mode (PC.r1, PC.mode_r1, PC.B);
						printf (", ");
					}
				}
				if (cmd.param & HAS_DD)
				{
					dd = get_mode (PC.r2, PC.mode_r2, PC.B);
				}
				if (cmd.param & HAS_NN)
				{
					get_nn (w);
				}
				if(cmd.param & HAS_XX)
				{
					get_xx(w);
				}
				cmd.func(PC);
				printf("\n");
				break;
			}
		}
	}
}

int main (int argc, char ** argv)
{
	mem[0177564] = -1;
	fprintf(stderr, "PDP is ready\n");

    load_file(argv[1]);
    run(01000, argv);
    return 0;
}
