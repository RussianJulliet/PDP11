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
struct Flags flags;

#define pc reg[7]
#define REG 1
#define MEM 0
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD (1 << 1) //2
#define HAS_NN (1 << 2) //4
#define HAS_XX (1 << 3) //8
#define sign(w, is_byte) (is_byte ? ((w)>>7)&1 : ((w)>>15)&1 )

word w_read  (adr a);
void w_write (adr a, word val);
byte b_read  (adr a);  
void b_write (adr a, byte val);
void get_nn (word w);
void get_xx (word w);
void print_reg();

void load_file (char * file);
struct P_Command create_command (word w);
void run (adr pc0, char ** argv);

void do_halt (struct P_Command PC);
void do_movb(struct P_Command PC);
void do_mov (struct P_Command PC);
void do_add (struct P_Command PC);
void do_sob (struct P_Command PC);
void do_br(struct P_Command PC);
void do_beq(struct P_Command PC);
void do_tstb(struct P_Command PC);
void do_bpl(struct P_Command PC);
void do_jsr(struct P_Command PC);
void do_rts(struct P_Command PC);
void do_mul(struct P_Command PC);
void do_dec(struct P_Command PC);
void do_tst(struct P_Command PC);
void do_unknown (struct P_Command PC);
void do_clr (struct P_Command PC);

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

struct Flags
{
	word C;
	word Z;
	word N;
	word V; //не реал-м 
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
	{077000,	0177000,	"sob",		do_sob,		HAS_NN			},
	{005000, 	0177700,	"clr",		do_clr,		HAS_DD			},
	{001400, 	0xFF00,		"beq", 		do_beq, 	HAS_XX			},
	{000400, 	0xFF00, 	"br", 		do_br, 		HAS_XX			},
	{0105700,	0177700,	"tstb",		do_tstb,	HAS_DD			},
	{0100000,   0xFF00,		"bpl",		do_bpl,		HAS_XX			},
	{004000, 	0177000, 	"jsr",		do_jsr,	 	HAS_DD			},
	{0000200,	0177770, 	"rts",		do_rts,		HAS_DD			},
	{0070000, 	0177000,    "mul",		do_mul,		HAS_SS			},
	{0005300, 	0177700,	"dec",		do_dec,		HAS_DD			},
	{005700,	0177700,	"tst",		do_tst, 	HAS_DD			},
	{	  0,		  0,	"unknown",	do_unknown,	NO_PARAM		}
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
/*
void b_write (adr a, byte val)
{
    mem[a] = val;
}*/

void b_write(adr a, byte val)
{
	mem[a] = val;
	if(a == 0177566)
	{
		fprintf(stderr, " %c ", mem[a]);
	}
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

void change_flag(struct P_Command PC)
{
    if (PC.B)
    {
        flags.N = (dd.res >> 7) & 1;
	}
	else 
    {
        flags.N = (dd.res >> 15) & 1;
    }
    flags.Z = (dd.res == 0);

}

void print_reg() {
    int i = 0;
    printf("\n\n");
    printf("Print registers : ");
    for (i = 0; i < 7; i++)
    {
        printf("R[%d]=%o, ", i, reg[i]);
    }
    printf("R[%d]=%o\n", i, reg[i]);

}

word byte_to_word(byte b) 
{

	word w;
	if (sign(b, 1) == 0) {
		w = 0;
		w |= b;
	} else {
		w = ~0xFF;
		w |= b;
	}
	return w;
}

void do_unknown (struct P_Command PC)
{
	printf("\n");
	exit(0);
}

void do_clr (struct P_Command PC)
{
	dd.val = 0;
	if(dd.space == REG)
	{
		reg[PC.r2] = 0;
	}
	else
	{
		w_write(dd.ad, dd.val);
	}

	flags.N = 0;
	flags.Z = 1;
	printf ("\n");
}


void do_halt (struct P_Command PC)
{
	printf("\n");
    print_reg();
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
    change_flag(PC);
}

void do_movb(struct P_Command PC)
{
	dd.res = ss.val;
	if (dd.space == REG)
	{
		reg[dd.ad] = byte_to_word(dd.res);	
	}
	else
	{
		b_write(dd.ad, (byte)dd.res);
	}
	printf ("\n");
	change_flag(PC);
}

void do_add(struct P_Command PC) {
    dd.res = dd.val + ss.val;
    if (dd.space == REG) {
        reg[dd.ad] = dd.res;
    }
    else {
        w_write(dd.ad, dd.res);
    }
    change_flag(PC);
    
}

void do_br(struct P_Command PC)
{
    pc += 2 * xx.val;
    printf("\n");
}

void do_beq(struct P_Command PC)
{
	if(flags.Z == 1)
	{
		do_br(PC);
	}
	printf("\n");
}

void do_tstb(struct P_Command PC)
{
	dd.res = dd.val;
	change_flag(PC);
	printf("\n");
}

void do_bpl(struct P_Command PC)
{
	if(flags.N == 0)
	{
		do_br(PC);
	}
	else
	{
		printf("\n");
	}
}

void do_jsr (struct P_Command PC) //запоминаем рег-ры + идем на другую команду 
{
	printf(", R%o", PC.r1);
    w_write(reg[6], reg[PC.r1]);
    reg[6]-= 2;
    reg[PC.r1] = pc;
    pc = dd.ad;
    printf("\n");
}

void do_rts(struct P_Command PC) //вспомминаем и идём дальше
{ 
	printf("R%o", PC.r2);
    reg[7] = reg[PC.r2];
    reg[6]+= 2;
    reg[PC.r2] = w_read(reg[6]);
    printf("\n");
}

void do_mul(struct P_Command PC)
{
	printf("R%o", PC.r1);
	long int res = reg[PC.r1] * ss.val;
    reg[PC.r1] = res;
	printf("\n");
}

void do_dec(struct P_Command PC)
{
	dd.val--;
	change_flag(PC);
	if (dd.space == MEM)
	{
		w_write(dd.ad, dd.val);
	}
	else
	{ 
		reg[dd.ad] = dd.val;
	}
	printf("r5 = %o\n", reg[5]);
}

void do_tst(struct P_Command PC)
{
	dd.res = dd.val;
	change_flag(PC);
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
			hh.space = MEM;
			hh.ad = reg[r];
			hh.val = b ? b_read((adr)reg[r]) : w_read ((adr) reg[r]);
            if (r == 7 || r == 6 || b == 0)   //зависит от типа инструкции : байтовая или пословная
				reg[r] += 2;
			else
				reg[r] ++;
            if (r == 7)
				printf ("(R%o)+", r);
            else
				printf ("#%o", hh.val);
			break;
        }
	    case 3:
        {
            /*
			hh.space = MEM;
			hh.ad = reg[r];
            hh.ad = w_read(hh.ad);
			hh.val = b ? b_read((adr)reg[r]) : w_read ((adr) reg[r]);
			reg[r] += 2;
            if (r == 7)
				printf ("@(R%o)+", r);
            else
				printf ("@#%o", hh.ad);
            break;*/
            //printf ("@#%o", w_read((adr) (reg[r])));
			if (r == 7 || r == 6 || b == 0)
			{
                printf ("@(R%o)+", r);
				hh.ad = w_read ((adr) reg[r]);
				hh.val = w_read ((adr) w_read ((adr) (reg[r])));
				hh.space = MEM;
				reg[r] += 2;
			}
			else
			{
                printf ("@#%o", w_read((adr) (reg[r])));
                hh.ad = w_read ((adr) reg[r]);
				hh.val = b_read ((adr) w_read ((adr) (reg[r])));
				hh.space = MEM;
				reg[r] ++;
			}
			break;
        }
	    case 4:
		{
			printf ("-(R%o)", r);
			if (r == 7 || r == 6 || b == 0)
			{
				reg[r] -= 2;
				hh.ad = reg[r];
				hh.val = w_read ((adr) reg[r]);
				hh.space = MEM;
				break;
			}
			else
			{
				reg[r] --;
				hh.ad = reg[r];
				hh.val = b_read ((adr) reg[r]);
				hh.space = MEM;
				break;
			}
		}
	    case 5:
		{
            printf ("@-(R%o)", r);
			reg[r] -= 2;
			hh.ad = w_read ((adr) reg[r]);
			hh.val = w_read ((adr) w_read ((adr) (reg[r])));
			hh.space = MEM;
			break;
		}
	    case 6:
		{

			if (r == 7)
			{
				hh.ad = 7;
				hh.val = reg[7] + 2 + w_read (reg[7]);
				reg[7] +=2;
				hh.ad = hh.val;
				printf ("%o", hh.val);
			}
			else
			{
				printf ("%o(R%o)", w_read (reg[7]), r);
				hh.ad = r;
				hh.val = w_read ((adr) reg[r] + w_read(reg[7]));
				reg[7] += 2;
			}
			break;
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
    mem[0177564] |= 128;
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
