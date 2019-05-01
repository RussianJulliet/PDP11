#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;
typedef int dword;

byte mem [64 * 1024];
word reg [8];

#define pc reg[7]
#define REG 1
#define MEM 0
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1 << 1) //2
#define HAS_NN (1 << 2) //4
#define HAS_XX (1 << 3) //8

word w_read  (adr a);
void w_write (adr a, word val);
byte b_read  (adr a);  
void b_write (adr a, byte val);
void get_nn (word w);
void get_xx (word w);


void load_file (char * file);
struct P_Command create_command (word w);
void run (adr pc0, char ** argv);

void do_halt (struct P_Command PC);
void do_movb(struct P_Command PC);
void do_mov (struct P_Command PC);
void do_add (struct P_Command PC);
void do_sob (struct P_Command PC);

struct mr get_mode (word r, word mode, word b);
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
    {	  0,	0177777,	"halt",		do_halt,	NO_PARAM		},
	{010000,	0170000,	"mov",		do_mov, 	HAS_SS | HAS_DD	},
	{0110000, 	0170000,	"movb", 	do_movb, 	HAS_SS | HAS_DD },
	{060000, 	0170000,	"add",		do_add,		HAS_DD | HAS_SS	},
	{077000,	0177000,	"sob",		do_sob,		HAS_NN			}
};

struct mr 
{
	word ad;		// address
	dword val;		// value                              
	dword res;		// result                                         
	word space; 	// address in mem[ ] or reg[ ]
} ss, dd, hh, nn;

struct sign
{
	char val;
	char sign;
}xx;

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

void do_halt (struct P_Command PC)
{
	printf("\n");
	print_beauty();
	exit(0);
}

void do_sob (struct P_Command PC)
{
	reg[nn.ad]--;
	if (reg[nn.ad] != 0)
	{
		reg[7] -= 2 * nn.val;
	}
	printf ("\n");
}

void do_mov (struct P_Command PC)
{
	dd.res = ss.val;
	if(dd.space == REG)
	{
		reg[dd.ad]= dd.res;
	}
	else
	{
		w_write(dd.ad, dd.res);
	}
	printf("\n");
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

struct mr get_mode (word r, word mode, word b)//register, mode of this register, byte 
{
	switch(mode)
	{
		case 0:
		{
            printf("R%o", r);
			hh.ad = r;
			hh.val = reg[r];
			hh.space = REG;
			break;
		}

		case 1:
		{
            printf ("@R%o", r);
			hh.ad = reg[r];
			hh.val = w_read ((adr) reg[r]);
			hh.space = MEM;
			break;
		}

		case 2:
		{
            if (r == 7 || r == 6 || b == 0)   //зависит от типа инструкции : байтовая или пословная
			{
				printf ("#%o", w_read ((adr) reg[r]));
				hh.ad = reg[r];
				hh.val = w_read ((adr) reg[r]);
				hh.space = MEM;
				reg[r] += 2;
			}
			else
			{
				printf ("(R%o)+", r);
				hh.ad =  reg[r];
				hh.val = b_read ((adr) reg[r]);
				hh.space = MEM;
				reg[r] ++;
			}
			break;
        }
	    case 3:
        {
		}
	    case 4:
		{
		}
	    case 5:
		{
		}
	    case 6:
		{
		}
	}
	return hh;
}

void get_nn (word w)
{
	nn.ad = (w >> 6) & 07;
	nn.val = w & 077;
	printf ("R%o , %o", nn.ad, pc - 2*nn.val);
}

void get_xx (word w)
{
	xx.val = w & 0xff;
	unsigned int x = pc + 2*xx.val;
	printf("%06o ", x);
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
					if(commands[i].name == "mul")  //отличается от всех, обратить внимание
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
					get_nn(w);
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
