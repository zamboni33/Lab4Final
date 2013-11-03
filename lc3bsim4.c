/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND3, COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    
    /* Ld.RegMux1	Ld.RegMux0	PSRMux	LD.PSR	LD.Int	LD.SSP	LD.USP	IEMUX	GATE.PCMinus	
     * Gate.PSR	LD.DRMux1	LD.SRMux1	Gate.Vector	LD.Vector	SR2Mux.2	MemCk	EXC1	
     * EXC0	LD.EXCV */
	 REGMUX1,
	 REGMUX0,
	 PSRMUX,
	 LD_PSR,
	 LD_INT,
	 LD_SSP,
	 LD_USP,
	 IEMUX,
	 GATE_PCMINUS2,
	 GATE_PSR,
	 DRMUX1,
	 SR1MUX1,
	 GATE_VECTOR,
	 LD_VECTOR,
	 SR2MUX2,
	 MEMCHK,
	 EXC1,
	 EXC0,
	 LD_EXCV,	
    
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }

int GetCOND3(int *x)          { return(x[COND3]); }
int GetCOND2(int *x)          { return(x[COND2]); }

int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX1] << 1) + x[SR1MUX]; }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */


int GetREGMUX(int *x)			{ return((x[REGMUX1] << 1) + x[REGMUX0]);}
int GetPSRMUX(int *x)			{ return(x[PSRMUX	]); }
int GetLD_PSR(int *x)			{ return(x[LD_PSR	]); }
int GetLD_INT(int *x)			{ return(x[LD_INT	]); }
int GetLD_SSP(int *x)			{ return(x[LD_SSP	]); }
int GetLD_USP(int *x)			{ return(x[LD_USP	]); }
int GetIEMUX(int *x)				{ return(x[IEMUX	]); }
int GetGATE_PCMINUS2(int *x)	{ return(x[GATE_PCMINUS2]); }
int GetGATE_PSR(int *x)			{ return(x[GATE_PSR	]); }
/* int GetLD_DRMUX1(int *x)	{ return(x[	]) } */
/* int GetLD_SRMUX1(int *x)	{ return(x[	]) } */
int GetGATE_VECTOR(int *x)		{ return(x[GATE_VECTOR	]); }
int GetLD_VECTOR(int *x)		{ return(x[LD_VECTOR	]); }
int GetSR2MUX2(int *x)			{ return(x[SR2MUX2	]); }
int GetMEMCHK(int *x)			{ return(x[MEMCHK	]); }
int GetEXC(int *x)				{ return((x[EXC1] << 1) + x[EXC0]);}
int GetLD_EXCV(int *x)			{ return(x[LD_EXCV	]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */
    
int    PSR15;   /* PSR[15] bit */
    
int INTERRUPT;

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

 int memoryCount 	= 0;
  int memoryFlag		= 0;
  int marGate 			= 0;
  int PCGate			= 0;
  int ALUGate			= 0;
  int SHFGate			= 0;
  int MDRGate			= 0;
  
  int WE0 				= 0;
  int WE1 				= 0;
  
  int adderOut			= 0;
  
  int inMux				= 0;
  
  int cycleCount		= 0;

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */




	int nextStateAddy 			= 0;
	int conditionBits 			= 0;
	int COND0					= 0;
	int COND1					= 0;
	int IRD 					= 0;
	
	IRD 				=  GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
	if(IRD){
		nextStateAddy = Low16bits(( (CURRENT_LATCHES.IR >> 12) & 0x000F));
		memcpy				(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextStateAddy], 
								4*CONTROL_STORE_BITS);
		NEXT_LATCHES.STATE_NUMBER = Low16bits(nextStateAddy);
	}
	else{
		nextStateAddy 	= 	GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
		
		conditionBits 	= 	GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
		COND0 = Low16bits((conditionBits & 0x0001));
		COND1 = Low16bits((conditionBits & 0x0002) >> 1);
		
		
		if( CURRENT_LATCHES.BEN && COND1 && !COND0 ) {
			nextStateAddy += 4;
			NEXT_LATCHES.BEN = 0;
			printf("Taking the branch %d.\n", nextStateAddy);
		}
		
		if( CURRENT_LATCHES.READY && !COND1 && COND0 ) {
			
			nextStateAddy += 2;
			NEXT_LATCHES.READY = 0;
		}
		
		if( (CURRENT_LATCHES.IR & 0x0800 ) && COND1 && COND0 ) {
			nextStateAddy += 1;
		}

		NEXT_LATCHES.STATE_NUMBER = Low16bits(nextStateAddy);
		/*
		if( (NEXT_LATCHES.STATE_NUMBER == 33 ||
			 NEXT_LATCHES.STATE_NUMBER == 28 ||
			 NEXT_LATCHES.STATE_NUMBER == 29 ||
			 NEXT_LATCHES.STATE_NUMBER == 25 ||
		 	 NEXT_LATCHES.STATE_NUMBER == 16 ||
			 NEXT_LATCHES.STATE_NUMBER == 17) && !memoryFlag){
				memoryFlag = 1;
				memoryCount = 0;
			}
		*/
	}

	memcpy							(NEXT_LATCHES.MICROINSTRUCTION, 
										 CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], 
										 sizeof(int)*CONTROL_STORE_BITS);
}



void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
	
	if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)){
		memoryCount += 1;
		if(memoryCount == 5){
			int MAR0 = (CURRENT_LATCHES.IR & 0x01);
			/*printf("Ready bit now set.\n");*/
		
	
	
			int readWrite = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
			int dataSize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
			
			if (!readWrite && !dataSize && !MAR0){				inMux = MEMORY[CURRENT_LATCHES.MAR >> 1][MAR0];}
			else if (!readWrite && !dataSize && MAR0){ 			inMux = (MEMORY[CURRENT_LATCHES.MAR >> 1][MAR0] << 8) & 0xFF00;}
			else if (!readWrite && dataSize){ 					inMux = (MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8) + MEMORY[CURRENT_LATCHES.MAR >> 1][0];}
				
			else if (readWrite && !dataSize && !MAR0){			MEMORY[CURRENT_LATCHES.MAR >> 1][MAR0] 	= CURRENT_LATCHES.MDR;}
			else if (readWrite && !dataSize && MAR0){			MEMORY[CURRENT_LATCHES.MAR >> 1][MAR0] 	= (CURRENT_LATCHES.MDR >> 8);}
			else if (readWrite && dataSize && !MAR0){			
						MEMORY[CURRENT_LATCHES.MAR >> 1][0] 	= (CURRENT_LATCHES.MDR & 0x00FF);
						MEMORY[CURRENT_LATCHES.MAR >> 1][1] 	= ((CURRENT_LATCHES.MDR >> 8) & 0x00FF);
			}
			memoryCount = 0;
			NEXT_LATCHES.READY = 0;
		}
	}

}
	
	/* memoryCount = 0; */
	/* NEXT_LATCHES.READY = 0; */
	
	/*
	if(GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) & !MAR0){WE0 = 0x01;}
	if( (GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) && !MAR0) || 
		GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) && 
		GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) ){
			WE1 = 0x01;}

	/* Implement reads and writes here. 
			inMux = 0;
			inMux |= MEMORY[CURRENT_LATCHES.MAR >> 1][1];
			inMux = inMux << 8; 
			inMux |= MEMORY[CURRENT_LATCHES.MAR >> 1][0]; 
	
	if(!WE1 && !WE0){ 
		/*
		if(!GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) && (0x01 & CURRENT_LATCHES.MAR)){
			 printf("Load Byte\n");
			inMux = 0;
			inMux |= (MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8);
		} else if(!GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) && !(0x01 & CURRENT_LATCHES.MAR)){
			 printf("Load Byte\n");
			inMux = 0;
			inMux |= (MEMORY[CURRENT_LATCHES.MAR >> 1][0]);
		} else{
		*/	
		
		

	/* Watch for the shift right here on stb 
	else if(!WE1 && WE0){ 
		MEMORY[CURRENT_LATCHES.MAR >> 1][0] = 0x00FF & CURRENT_LATCHES.MDR;}
	else if(WE1 && !WE0){ 
		int tempVar = Low16bits ((0xFF00 & CURRENT_LATCHES.MDR) >> 8);
		MEMORY[CURRENT_LATCHES.MAR >> 1][1] = tempVar;
	}else{ 
		MEMORY[CURRENT_LATCHES.MAR >> 1][0] = 0;
		MEMORY[CURRENT_LATCHES.MAR >> 1][0] |= Low16bits(CURRENT_LATCHES.MDR & 0x00FF);
		MEMORY[CURRENT_LATCHES.MAR >> 1][1] = 0;
		MEMORY[CURRENT_LATCHES.MAR >> 1][1] |= Low16bits(((CURRENT_LATCHES.MDR & 0xFF00) >> 8));
	}

		*/


/* Checked */
void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */
	int i = 0;

	/*
	if(GetMIO_EN){   
	*/	  
		  /* Checked */
		  if( GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION) ){
			  PCGate = CURRENT_LATCHES.PC;
		  }
		  
		  /* Checked */
		  if( GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION) ){
			  if(GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1){ 
					MDRGate = Low16bits(CURRENT_LATCHES.MDR);
				 
			/* Fuckin load a byte here instead of a word dumbass */	
				}else {
					if(CURRENT_LATCHES.MAR & 0x01){
						if ((CURRENT_LATCHES.MDR & 0x8000) == 0) {
							printf("ab wrong\n");
							MDRGate	= (0x00FF & (CURRENT_LATCHES.MDR >> 8));
						}
						else {
							printf("ab\n");
							MDRGate	= (0xFF00 | (CURRENT_LATCHES.MDR >> 8));
						}
					} else{
						if ((CURRENT_LATCHES.MDR & 0x0080) == 0) {
							printf("positive\n");
							MDRGate	= (0x00FF & (CURRENT_LATCHES.MDR));
						}
						else {
							printf("negative\n");
							MDRGate	= (0xFF00 | (CURRENT_LATCHES.MDR));
						}
					}
				
				}
			}
		  
		  /* Checked */
		  if( GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION) ){
			  /* Get the correct reg values threw the ALU */
			  /* ALU A */
			  /* ALU A here does not utilize SR1 correctness for SR on Stores */
			  int A = 0;
			  /* Checked */
			  if(! (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) ){
					A = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR >> 9) & 0x07)];
				} else {
					A = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR >> 6) & 0x07)];
				}
			  A = Low16bits(A);
			  /* ALU B */
			  int B = 0;
			  /* Immediate Value */
			  if( (CURRENT_LATCHES.IR >> 5) & 0x01){
				  /* B = SignExtend(CURRENT_LATCHES.IR, 5); */
				  	if ( !(CURRENT_LATCHES.IR & 0x0010)){
						B = (0x001F & CURRENT_LATCHES.IR);
					}
					else {
						B = (0xFFE0 | CURRENT_LATCHES.IR);
					}
				  
			  } else{
				  B = CURRENT_LATCHES.REGS[(CURRENT_LATCHES.IR & 0x07)];
			  }
			  B = Low16bits(B);
			  
			  int x = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
			  if(x == 0){
				  /* printf("Addition: %d and %d\n", A, B);  */
				  ALUGate = Low16bits(A + B);}
			  else if(x == 1){ALUGate = Low16bits(A & B);}
			  else if(x == 2){ALUGate = Low16bits(A ^ B);}
			  else if(x == 3){ALUGate = Low16bits(A);}
		  }
		  
		  /* Right half of MarMux adder */
		  int A = 0;
		  if(GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)){
			  A = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR >> 6) & 0x07)];
		  } else{
			  A = CURRENT_LATCHES.PC;
		  }
		  A = Low16bits(A);
		  
		  /* Left half of MarMux adder */
		  int B = 0;
		  int x = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
		  if(x == 0){ B = 0; }
		  else if(x == 1){ B = (SignExtend(CURRENT_LATCHES.IR, 5)); }
		  else if(x == 2){ B = (SignExtend(CURRENT_LATCHES.IR, 8)); }
		  else if(x == 3){ B = (SignExtend(CURRENT_LATCHES.IR, 10)); }
		  if(GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION)){B = B << 1;}
		  B = Low16bits(B);
		  
		  /* Adder Result */
		  adderOut = Low16bits(A + B);

		  /* Checked */
		  if( GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION) ){
			  if(GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION)){marGate = adderOut;}
			  else{marGate = (CURRENT_LATCHES.IR & 0x00FF) << 1; }
			  
			  
			  /* marmuxGate = CURRENT_LATCHE; */
		  }
		  
		  /* Checked */
		  if( GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION) ){
			  int A = CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR >> 6) & 0x07)];
			  int shiftAmount = CURRENT_LATCHES.IR & 0x0F;
			  if(! (CURRENT_LATCHES.IR & 0x0030)){
					SHFGate = Low16bits(A << shiftAmount);
				} else if( (CURRENT_LATCHES.IR & 0x0030) == 0x0010){
					SHFGate = Low16bits(A >> shiftAmount);
				} else{
					
					if(! (A & 0x8000)){
						SHFGate = Low16bits(A >> shiftAmount);
					} else{
						for(i = 0; i < shiftAmount; i += 1){
							A = A >> 1;
							A |= 0x8000;
						}
						SHFGate = Low16bits(A);
					}
				}
			}
}	  /* SHFGate = CURRENT_LATCHES; */
/* } */

void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       
	if( GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION) ){
		BUS = Low16bits(PCGate);
	}
  
	else if( GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION) ){
		BUS = Low16bits(MDRGate);
	}
  
	else if( GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION) ){
		BUS = Low16bits(ALUGate);
	}
	else if( GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION) ){
		BUS = Low16bits(marGate);
	}
	else if( GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION) ){
		BUS = Low16bits(SHFGate);
	}
	else{ BUS = 0;}

}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */
   
   /* Checked */
   if( GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION) ){
		int pcMUX = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
		/* printf("LDPC: %d pcMux: %d with %d\n", GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION), pcMUX, adderOut); */
		if(pcMUX == 0){NEXT_LATCHES.PC += 2;}
		if(pcMUX == 1){NEXT_LATCHES.PC = BUS;}
		if(pcMUX == 2){NEXT_LATCHES.PC = adderOut;}
	}
	
	/* Checked */
	if( GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION) ){NEXT_LATCHES.IR = BUS;}
	
	/* Checked */
	if( GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION) ){
		if( BUS & 0x8000){NEXT_LATCHES.N = 1;}
			else{NEXT_LATCHES.N = 0;}
		if(!BUS){NEXT_LATCHES.Z = 1;}
			else{NEXT_LATCHES.Z = 0;}
		if(!NEXT_LATCHES.N && !NEXT_LATCHES.Z){NEXT_LATCHES.P = 1;}
			else{NEXT_LATCHES.P = 0;}
	}
	
	/* Checked */
	if( GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION) ) {
		if(!GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)){
			/* Loading Register IR[11-9] */
			int tempReg = ( (CURRENT_LATCHES.IR & 0x0E00) >> 9);
			NEXT_LATCHES.REGS[tempReg] = Low16bits(BUS);
			/* printf("LEA Load Reg: %d\n", BUS); */
		} else{
			/* Loading R7 */
			/* printf("Loading R7\n"); */
			NEXT_LATCHES.REGS[7] = Low16bits(BUS);
		}
	}
	
	/* printf("LD_MDR: %d\n", GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)); */
	printf("MDRGate: %d\n", MDRGate);
	
	
	if( GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {

		if(GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)){
			printf("MIO\n");
			NEXT_LATCHES.MDR = inMux;
			}
		else{
			if (CURRENT_LATCHES.MAR & 0x0001) {
				NEXT_LATCHES.MDR = (BUS & 0x00FF) << 8;
			}
			
			else{
				
				if(!GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)){
					NEXT_LATCHES.MDR = (BUS & 0x00FF);
				} else{
					NEXT_LATCHES.MDR = BUS;
				}
			
			}
			
		}
	}
	
	/* Checked */
	if( GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION) ) {
		NEXT_LATCHES.MAR = Low16bits(BUS);
	}
	
	/* Checked */
	if( GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION) ) {
		if( ((CURRENT_LATCHES.IR & 0x0800) && CURRENT_LATCHES.N) ||
				((CURRENT_LATCHES.IR & 0x0400) && CURRENT_LATCHES.Z) ||
				((CURRENT_LATCHES.IR & 0x0200) && CURRENT_LATCHES.P) ){
					/* printf("%d  %d  %d  %d\n", CURRENT_LATCHES.IR, CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P); */
					NEXT_LATCHES.BEN = Low16bits(1);
			} else {
				NEXT_LATCHES.BEN = 0;
			}
	}
	
	if (memoryCount == 4) {
		NEXT_LATCHES.READY = 1;
	}
	
	if(cycleCount == 299){
		NEXT_LATCHES.INTERRUPT = 1;
		NEXT_LATCHES.INTV = 1;
	}
	cycleCount += 1;
	
}

int SignExtend(int instruction, int position) {
	int A = 0x01;
	int mask = 0x00;
	A = Low16bits(A << (position - 1));
	if(instruction & A){
		if(position == 5){mask = 0xFFE0;}
		else if(position == 6){mask = 0xFFC0;}
		else if(position == 8){mask = 0xFF00;}
		else if(position == 10){mask = 0xFC00;}
		else if(position == 16){mask = 0xFFFF0000;}
	}
	else{	/* This number is positive */
		if(position == 5){mask = 0x001F;}
		else if(position == 6){mask = 0x003F;}
		else if(position == 8){mask = 0x00FF;}
		else if(position == 10){mask = 0x03FF;}
		else if(position == 16){mask = 0x0000FFFF;}
	}

	if(instruction & A){
		/* printf("SignExtend: %d\n", (mask | instruction) ); */
		return(mask | instruction);} 
	else{
		/* printf("SignExtend: %d\n", (mask & instruction) ); */
		return(mask & instruction);}
}
