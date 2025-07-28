/* cc -g -O0 8080emu-first50.c -o 8080emu-first50 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

//#define dbgprint(...) printf(__VA_ARGS__)
#define dbgprint(...) fprintf(stderr, __VA_ARGS__);

typedef struct ConditionCodes {
	uint8_t		z:1;
	uint8_t		s:1;
	uint8_t		p:1;
	uint8_t		cy:1;
	uint8_t		ac:1;
	uint8_t		pad:3;
} ConditionCodes;

ConditionCodes CC_ZSPAC = {1,1,1,0,1};

typedef struct State8080 {
	uint8_t		a;
	uint8_t		b;
	uint8_t		c;
	uint8_t		d;
	uint8_t		e;
	uint8_t		h;
	uint8_t		l;
	uint16_t	sp;
	uint16_t	pc;
	uint8_t		*memory;
	struct ConditionCodes		cc;
	uint8_t		int_enable;
}State8080;

int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1<<size)-1));
	for (i=0; i<size; i++)
	{
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}

int Disassemble8080Op(unsigned char *codebuffer, int pc)
{
	unsigned char *code = &codebuffer[pc];
	int opbytes = 1;
	dbgprint("%04x ", pc);
	switch (*code)
	{
		case 0x00: dbgprint("NOP"); break;
		case 0x01: dbgprint("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x02: dbgprint("STAX   B"); break;
		case 0x03: dbgprint("INX    B"); break;
		case 0x04: dbgprint("INR    B"); break;
		case 0x05: dbgprint("DCR    B"); break;
		case 0x06: dbgprint("MVI    B,#$%02x", code[1]); opbytes=2; break;
		case 0x07: dbgprint("RLC"); break;
		case 0x08: dbgprint("NOP"); break;
		case 0x09: dbgprint("DAD    B"); break;
		case 0x0a: dbgprint("LDAX   B"); break;
		case 0x0b: dbgprint("DCX    B"); break;
		case 0x0c: dbgprint("INR    C"); break;
		case 0x0d: dbgprint("DCR    C"); break;
		case 0x0e: dbgprint("MVI    C,#$%02x", code[1]); opbytes = 2;	break;
		case 0x0f: dbgprint("RRC"); break;
			
		case 0x10: dbgprint("NOP"); break;
		case 0x11: dbgprint("LXI    D,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x12: dbgprint("STAX   D"); break;
		case 0x13: dbgprint("INX    D"); break;
		case 0x14: dbgprint("INR    D"); break;
		case 0x15: dbgprint("DCR    D"); break;
		case 0x16: dbgprint("MVI    D,#$%02x", code[1]); opbytes=2; break;
		case 0x17: dbgprint("RAL"); break;
		case 0x18: dbgprint("NOP"); break;
		case 0x19: dbgprint("DAD    D"); break;
		case 0x1a: dbgprint("LDAX   D"); break;
		case 0x1b: dbgprint("DCX    D"); break;
		case 0x1c: dbgprint("INR    E"); break;
		case 0x1d: dbgprint("DCR    E"); break;
		case 0x1e: dbgprint("MVI    E,#$%02x", code[1]); opbytes = 2; break;
		case 0x1f: dbgprint("RAR"); break;
			
		case 0x20: dbgprint("NOP"); break;
		case 0x21: dbgprint("LXI    H,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x22: dbgprint("SHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x23: dbgprint("INX    H"); break;
		case 0x24: dbgprint("INR    H"); break;
		case 0x25: dbgprint("DCR    H"); break;
		case 0x26: dbgprint("MVI    H,#$%02x", code[1]); opbytes=2; break;
		case 0x27: dbgprint("DAA"); break;
		case 0x28: dbgprint("NOP"); break;
		case 0x29: dbgprint("DAD    H"); break;
		case 0x2a: dbgprint("LHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x2b: dbgprint("DCX    H"); break;
		case 0x2c: dbgprint("INR    L"); break;
		case 0x2d: dbgprint("DCR    L"); break;
		case 0x2e: dbgprint("MVI    L,#$%02x", code[1]); opbytes = 2; break;
		case 0x2f: dbgprint("CMA"); break;
			
		case 0x30: dbgprint("NOP"); break;
		case 0x31: dbgprint("LXI    SP,#$%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x32: dbgprint("STA    $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x33: dbgprint("INX    SP"); break;
		case 0x34: dbgprint("INR    M"); break;
		case 0x35: dbgprint("DCR    M"); break;
		case 0x36: dbgprint("MVI    M,#$%02x", code[1]); opbytes=2; break;
		case 0x37: dbgprint("STC"); break;
		case 0x38: dbgprint("NOP"); break;
		case 0x39: dbgprint("DAD    SP"); break;
		case 0x3a: dbgprint("LDA    $%02x%02x", code[2], code[1]); opbytes=3; break;
		case 0x3b: dbgprint("DCX    SP"); break;
		case 0x3c: dbgprint("INR    A"); break;
		case 0x3d: dbgprint("DCR    A"); break;
		case 0x3e: dbgprint("MVI    A,#$%02x", code[1]); opbytes = 2; break;
		case 0x3f: dbgprint("CMC"); break;
			
		case 0x40: dbgprint("MOV    B,B"); break;
		case 0x41: dbgprint("MOV    B,C"); break;
		case 0x42: dbgprint("MOV    B,D"); break;
		case 0x43: dbgprint("MOV    B,E"); break;
		case 0x44: dbgprint("MOV    B,H"); break;
		case 0x45: dbgprint("MOV    B,L"); break;
		case 0x46: dbgprint("MOV    B,M"); break;
		case 0x47: dbgprint("MOV    B,A"); break;
		case 0x48: dbgprint("MOV    C,B"); break;
		case 0x49: dbgprint("MOV    C,C"); break;
		case 0x4a: dbgprint("MOV    C,D"); break;
		case 0x4b: dbgprint("MOV    C,E"); break;
		case 0x4c: dbgprint("MOV    C,H"); break;
		case 0x4d: dbgprint("MOV    C,L"); break;
		case 0x4e: dbgprint("MOV    C,M"); break;
		case 0x4f: dbgprint("MOV    C,A"); break;
			
		case 0x50: dbgprint("MOV    D,B"); break;
		case 0x51: dbgprint("MOV    D,C"); break;
		case 0x52: dbgprint("MOV    D,D"); break;
		case 0x53: dbgprint("MOV    D.E"); break;
		case 0x54: dbgprint("MOV    D,H"); break;
		case 0x55: dbgprint("MOV    D,L"); break;
		case 0x56: dbgprint("MOV    D,M"); break;
		case 0x57: dbgprint("MOV    D,A"); break;
		case 0x58: dbgprint("MOV    E,B"); break;
		case 0x59: dbgprint("MOV    E,C"); break;
		case 0x5a: dbgprint("MOV    E,D"); break;
		case 0x5b: dbgprint("MOV    E,E"); break;
		case 0x5c: dbgprint("MOV    E,H"); break;
		case 0x5d: dbgprint("MOV    E,L"); break;
		case 0x5e: dbgprint("MOV    E,M"); break;
		case 0x5f: dbgprint("MOV    E,A"); break;

		case 0x60: dbgprint("MOV    H,B"); break;
		case 0x61: dbgprint("MOV    H,C"); break;
		case 0x62: dbgprint("MOV    H,D"); break;
		case 0x63: dbgprint("MOV    H.E"); break;
		case 0x64: dbgprint("MOV    H,H"); break;
		case 0x65: dbgprint("MOV    H,L"); break;
		case 0x66: dbgprint("MOV    H,M"); break;
		case 0x67: dbgprint("MOV    H,A"); break;
		case 0x68: dbgprint("MOV    L,B"); break;
		case 0x69: dbgprint("MOV    L,C"); break;
		case 0x6a: dbgprint("MOV    L,D"); break;
		case 0x6b: dbgprint("MOV    L,E"); break;
		case 0x6c: dbgprint("MOV    L,H"); break;
		case 0x6d: dbgprint("MOV    L,L"); break;
		case 0x6e: dbgprint("MOV    L,M"); break;
		case 0x6f: dbgprint("MOV    L,A"); break;

		case 0x70: dbgprint("MOV    M,B"); break;
		case 0x71: dbgprint("MOV    M,C"); break;
		case 0x72: dbgprint("MOV    M,D"); break;
		case 0x73: dbgprint("MOV    M.E"); break;
		case 0x74: dbgprint("MOV    M,H"); break;
		case 0x75: dbgprint("MOV    M,L"); break;
		case 0x76: dbgprint("HLT");        break;
		case 0x77: dbgprint("MOV    M,A"); break;
		case 0x78: dbgprint("MOV    A,B"); break;
		case 0x79: dbgprint("MOV    A,C"); break;
		case 0x7a: dbgprint("MOV    A,D"); break;
		case 0x7b: dbgprint("MOV    A,E"); break;
		case 0x7c: dbgprint("MOV    A,H"); break;
		case 0x7d: dbgprint("MOV    A,L"); break;
		case 0x7e: dbgprint("MOV    A,M"); break;
		case 0x7f: dbgprint("MOV    A,A"); break;

		case 0x80: dbgprint("ADD    B"); break;
		case 0x81: dbgprint("ADD    C"); break;
		case 0x82: dbgprint("ADD    D"); break;
		case 0x83: dbgprint("ADD    E"); break;
		case 0x84: dbgprint("ADD    H"); break;
		case 0x85: dbgprint("ADD    L"); break;
		case 0x86: dbgprint("ADD    M"); break;
		case 0x87: dbgprint("ADD    A"); break;
		case 0x88: dbgprint("ADC    B"); break;
		case 0x89: dbgprint("ADC    C"); break;
		case 0x8a: dbgprint("ADC    D"); break;
		case 0x8b: dbgprint("ADC    E"); break;
		case 0x8c: dbgprint("ADC    H"); break;
		case 0x8d: dbgprint("ADC    L"); break;
		case 0x8e: dbgprint("ADC    M"); break;
		case 0x8f: dbgprint("ADC    A"); break;

		case 0x90: dbgprint("SUB    B"); break;
		case 0x91: dbgprint("SUB    C"); break;
		case 0x92: dbgprint("SUB    D"); break;
		case 0x93: dbgprint("SUB    E"); break;
		case 0x94: dbgprint("SUB    H"); break;
		case 0x95: dbgprint("SUB    L"); break;
		case 0x96: dbgprint("SUB    M"); break;
		case 0x97: dbgprint("SUB    A"); break;
		case 0x98: dbgprint("SBB    B"); break;
		case 0x99: dbgprint("SBB    C"); break;
		case 0x9a: dbgprint("SBB    D"); break;
		case 0x9b: dbgprint("SBB    E"); break;
		case 0x9c: dbgprint("SBB    H"); break;
		case 0x9d: dbgprint("SBB    L"); break;
		case 0x9e: dbgprint("SBB    M"); break;
		case 0x9f: dbgprint("SBB    A"); break;

		case 0xa0: dbgprint("ANA    B"); break;
		case 0xa1: dbgprint("ANA    C"); break;
		case 0xa2: dbgprint("ANA    D"); break;
		case 0xa3: dbgprint("ANA    E"); break;
		case 0xa4: dbgprint("ANA    H"); break;
		case 0xa5: dbgprint("ANA    L"); break;
		case 0xa6: dbgprint("ANA    M"); break;
		case 0xa7: dbgprint("ANA    A"); break;
		case 0xa8: dbgprint("XRA    B"); break;
		case 0xa9: dbgprint("XRA    C"); break;
		case 0xaa: dbgprint("XRA    D"); break;
		case 0xab: dbgprint("XRA    E"); break;
		case 0xac: dbgprint("XRA    H"); break;
		case 0xad: dbgprint("XRA    L"); break;
		case 0xae: dbgprint("XRA    M"); break;
		case 0xaf: dbgprint("XRA    A"); break;

		case 0xb0: dbgprint("ORA    B"); break;
		case 0xb1: dbgprint("ORA    C"); break;
		case 0xb2: dbgprint("ORA    D"); break;
		case 0xb3: dbgprint("ORA    E"); break;
		case 0xb4: dbgprint("ORA    H"); break;
		case 0xb5: dbgprint("ORA    L"); break;
		case 0xb6: dbgprint("ORA    M"); break;
		case 0xb7: dbgprint("ORA    A"); break;
		case 0xb8: dbgprint("CMP    B"); break;
		case 0xb9: dbgprint("CMP    C"); break;
		case 0xba: dbgprint("CMP    D"); break;
		case 0xbb: dbgprint("CMP    E"); break;
		case 0xbc: dbgprint("CMP    H"); break;
		case 0xbd: dbgprint("CMP    L"); break;
		case 0xbe: dbgprint("CMP    M"); break;
		case 0xbf: dbgprint("CMP    A"); break;

		case 0xc0: dbgprint("RNZ"); break;
		case 0xc1: dbgprint("POP    B"); break;
		case 0xc2: dbgprint("JNZ    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xc3: dbgprint("JMP    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xc4: dbgprint("CNZ    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xc5: dbgprint("PUSH   B"); break;
		case 0xc6: dbgprint("ADI    #$%02x",code[1]); opbytes = 2; break;
		case 0xc7: dbgprint("RST    0"); break;
		case 0xc8: dbgprint("RZ"); break;
		case 0xc9: dbgprint("RET"); break;
		case 0xca: dbgprint("JZ     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xcb: dbgprint("JMP    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xcc: dbgprint("CZ     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xcd: dbgprint("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xce: dbgprint("ACI    #$%02x",code[1]); opbytes = 2; break;
		case 0xcf: dbgprint("RST    1"); break;

		case 0xd0: dbgprint("RNC"); break;
		case 0xd1: dbgprint("POP    D"); break;
		case 0xd2: dbgprint("JNC    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xd3: dbgprint("OUT    #$%02x",code[1]); opbytes = 2; break;
		case 0xd4: dbgprint("CNC    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xd5: dbgprint("PUSH   D"); break;
		case 0xd6: dbgprint("SUI    #$%02x",code[1]); opbytes = 2; break;
		case 0xd7: dbgprint("RST    2"); break;
		case 0xd8: dbgprint("RC");  break;
		case 0xd9: dbgprint("RET"); break;
		case 0xda: dbgprint("JC     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xdb: dbgprint("IN     #$%02x",code[1]); opbytes = 2; break;
		case 0xdc: dbgprint("CC     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xdd: dbgprint("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xde: dbgprint("SBI    #$%02x",code[1]); opbytes = 2; break;
		case 0xdf: dbgprint("RST    3"); break;

		case 0xe0: dbgprint("RPO"); break;
		case 0xe1: dbgprint("POP    H"); break;
		case 0xe2: dbgprint("JPO    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xe3: dbgprint("XTHL");break;
		case 0xe4: dbgprint("CPO    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xe5: dbgprint("PUSH   H"); break;
		case 0xe6: dbgprint("ANI    #$%02x",code[1]); opbytes = 2; break;
		case 0xe7: dbgprint("RST    4"); break;
		case 0xe8: dbgprint("RPE"); break;
		case 0xe9: dbgprint("PCHL");break;
		case 0xea: dbgprint("JPE    $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xeb: dbgprint("XCHG"); break;
		case 0xec: dbgprint("CPE     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xed: dbgprint("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xee: dbgprint("XRI    #$%02x",code[1]); opbytes = 2; break;
		case 0xef: dbgprint("RST    5"); break;

		case 0xf0: dbgprint("RP");  break;
		case 0xf1: dbgprint("POP    PSW"); break;
		case 0xf2: dbgprint("JP     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xf3: dbgprint("DI");  break;
		case 0xf4: dbgprint("CP     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xf5: dbgprint("PUSH   PSW"); break;
		case 0xf6: dbgprint("ORI    #$%02x",code[1]); opbytes = 2; break;
		case 0xf7: dbgprint("RST    6"); break;
		case 0xf8: dbgprint("RM");  break;
		case 0xf9: dbgprint("SPHL");break;
		case 0xfa: dbgprint("JM     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xfb: dbgprint("EI");  break;
		case 0xfc: dbgprint("CM     $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xfd: dbgprint("CALL   $%02x%02x",code[2],code[1]); opbytes = 3; break;
		case 0xfe: dbgprint("CPI    #$%02x",code[1]); opbytes = 2; break;
		case 0xff: dbgprint("RST    7"); break;
	}
	
	return opbytes;
}

void LogicFlagsA(State8080* state)
{
	state->cc.cy = state->cc.ac = 0;
	state->cc.z = (state->a == 0);
	state->cc.s = (0x80 == (state->a & 0x80));
	state->cc.p = parity(state->a, 8);
}

void ArithFlagsA(State8080* state, uint16_t res)
{
	state->cc.cy = (res > 0xff);
	state->cc.z = ((res&0xff) == 0);
	state->cc.s = (0x80 == (res & 0x80));
	state->cc.p = parity(res&0xff, 8);
}

void UnimplementedInstruction(State8080* state)
{
	//pc will have advanced one, so undo that
	dbgprint ("Error: Unimplemented instruction\n");
	//return;
	state->pc--;
	Disassemble8080Op(state->memory, state->pc);
	dbgprint("\n");
	exit(1);
}

uint8_t MachineIN(State8080* state, uint8_t port)
{
	uint8_t a;
	switch(port)
	{
		case 2:
			a=0;
			//UnimplementedInstruction(state);
			break;
		case 3:
			UnimplementedInstruction(state);
//			uint16_t v = (shift1<<8) | shift0;
//			a = ((v >> (8-shift_offset)) & 0xff);
			break;
	}
	return a;
}

int Emulate8080Op(State8080* state)
{
	int cycles = 4;
	unsigned char *opcode = &state->memory[state->pc];

	Disassemble8080Op(state->memory, state->pc);
	
	state->pc+=1;	
	
	switch (*opcode)
	{
		case 0x00: break;	//NOP
		case 0x01: 							//LXI	B,word
			state->c = opcode[1];
			state->b = opcode[2];
			state->pc += 2;
			break;
		case 0x02: UnimplementedInstruction(state); break;
		case 0x03: UnimplementedInstruction(state); break;
		case 0x04: UnimplementedInstruction(state); break;
		case 0x05: 							//DCR    B
			{
			uint8_t res = state->b - 1;
			state->cc.z = (res == 0);
			state->cc.s = (0x80 == (res & 0x80));
			state->cc.p = parity(res, 8);
			state->b = res;
			}
			break;
		case 0x06: 							//MVI B,byte
			state->b = opcode[1];
			state->pc++;
			break;
		case 0x07: UnimplementedInstruction(state); break;
		case 0x08: UnimplementedInstruction(state); break;
		case 0x09: 							//DAD B
			{
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t bc = (state->b << 8) | state->c;
			uint32_t res = hl + bc;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) > 0);
			}
			break;
		case 0x0a: UnimplementedInstruction(state); break;
		case 0x0b: UnimplementedInstruction(state); break;
		case 0x0c: UnimplementedInstruction(state); break;
		case 0x0d: 							//DCR    C
			{
			uint8_t res = state->c - 1;
			state->cc.z = (res == 0);
			state->cc.s = (0x80 == (res & 0x80));
			state->cc.p = parity(res, 8);
			state->c = res;
			}
			break;
		case 0x0e: 							//MVI C,byte
			state->c = opcode[1];
			state->pc++;
			break;
		case 0x0f: 							//RRC
			{
				uint8_t x = state->a;
				state->a = ((x & 1) << 7) | (x >> 1);
				state->cc.cy = (1 == (x&1));
			}
			break;
		case 0x10: UnimplementedInstruction(state); break;
		case 0x11: 							//LXI	D,word
			state->e = opcode[1];
			state->d = opcode[2];
			state->pc += 2;
			break;
		case 0x12: UnimplementedInstruction(state); break;
		case 0x13: 							//INX    D
			state->e++;
			if (state->e == 0)
				state->d++;
			break;		
		case 0x14: UnimplementedInstruction(state); break;
		case 0x15: UnimplementedInstruction(state); break;
		case 0x16: UnimplementedInstruction(state); break;
		case 0x17: UnimplementedInstruction(state); break;
		case 0x18: UnimplementedInstruction(state); break;
		case 0x19: 							//DAD    D
			{
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t de = (state->d << 8) | state->e;
			uint32_t res = hl + de;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) != 0);
			}
			break;
		case 0x1a: 							//LDAX	D
			{
			uint16_t offset=(state->d<<8) | state->e;
			state->a = state->memory[offset];
			}
			break;
		case 0x1b: UnimplementedInstruction(state); break;
		case 0x1c: UnimplementedInstruction(state); break;
		case 0x1d: UnimplementedInstruction(state); break;
		case 0x1e: UnimplementedInstruction(state); break;
		case 0x1f: UnimplementedInstruction(state); break;
		case 0x20: UnimplementedInstruction(state); break;
		case 0x21: 							//LXI	H,word
			state->l = opcode[1];
			state->h = opcode[2];
			state->pc += 2;
			break;
		case 0x22: UnimplementedInstruction(state); break;
		case 0x23: 							//INX    H
			state->l++;
			if (state->l == 0)
				state->h++;
			break;		
		case 0x24: UnimplementedInstruction(state); break;
		case 0x25: UnimplementedInstruction(state); break;
		case 0x26:  							//MVI H,byte
			state->h = opcode[1];
			state->pc++;
			break;
		case 0x27: UnimplementedInstruction(state); break;
		case 0x28: UnimplementedInstruction(state); break;
		case 0x29: 								//DAD    H
			{
			uint32_t hl = (state->h << 8) | state->l;
			uint32_t res = hl + hl;
			state->h = (res & 0xff00) >> 8;
			state->l = res & 0xff;
			state->cc.cy = ((res & 0xffff0000) != 0);
			}
			break;
		case 0x2a: UnimplementedInstruction(state); break;
		case 0x2b: UnimplementedInstruction(state); break;
		case 0x2c: UnimplementedInstruction(state); break;
		case 0x2d: UnimplementedInstruction(state); break;
		case 0x2e: UnimplementedInstruction(state); break;
		case 0x2f: UnimplementedInstruction(state); break;
		case 0x30: UnimplementedInstruction(state); break;
		case 0x31: 							//LXI	SP,word
			state->sp = (opcode[2]<<8) | opcode[1];
			state->pc += 2;
			break;
		case 0x32: 							//STA    (word)
			{
			uint16_t offset = (opcode[2]<<8) | (opcode[1]);
			state->memory[offset] = state->a;
			state->pc += 2;
			}
			break;
		case 0x33: UnimplementedInstruction(state); break;
		case 0x34: UnimplementedInstruction(state); break;
		case 0x35:
			uint16_t memloc = (state->h << 8) | (state->l);
			uint8_t* mem = &state->memory[memloc];
			uint8_t res = *mem - 1;
			state->cc.z = (res == 0);
			state->cc.s = (0x80 == (res & 0x80));
			state->cc.p = parity(res, 8);
			*mem = res;
			break;
		case 0x36: 							//MVI	M,byte
			{					
			//AC set if lower nibble of h was zero prior to dec
			uint16_t offset = (state->h<<8) | state->l;
			state->memory[offset] = opcode[1];
			state->pc++;
			}
			break;
		case 0x37: UnimplementedInstruction(state); break;
		case 0x38: UnimplementedInstruction(state); break;
		case 0x39: UnimplementedInstruction(state); break;
		case 0x3a: 							//LDA    (word)
			{
			uint16_t offset = (opcode[2]<<8) | (opcode[1]);
			state->a = state->memory[offset];
			state->pc+=2;
			}
			break;
		case 0x3b: UnimplementedInstruction(state); break;
		case 0x3c: UnimplementedInstruction(state); break;
		case 0x3d: UnimplementedInstruction(state); break;
		case 0x3e: 							//MVI    A,byte
			state->a = opcode[1];
			state->pc++;
			break;
		case 0x3f: UnimplementedInstruction(state); break;
		case 0x40: UnimplementedInstruction(state); break;
		case 0x41: UnimplementedInstruction(state); break;
		case 0x42: UnimplementedInstruction(state); break;
		case 0x43: UnimplementedInstruction(state); break;
		case 0x44: UnimplementedInstruction(state); break;
		case 0x45: UnimplementedInstruction(state); break;
		case 0x46: UnimplementedInstruction(state); break;
		case 0x47: UnimplementedInstruction(state); break;
		case 0x48: UnimplementedInstruction(state); break;
		case 0x49: UnimplementedInstruction(state); break;
		case 0x4a: UnimplementedInstruction(state); break;
		case 0x4b: UnimplementedInstruction(state); break;
		case 0x4c: UnimplementedInstruction(state); break;
		case 0x4d: UnimplementedInstruction(state); break;
		case 0x4e: UnimplementedInstruction(state); break;
		case 0x4f: UnimplementedInstruction(state); break;
		case 0x50: UnimplementedInstruction(state); break;
		case 0x51: UnimplementedInstruction(state); break;
		case 0x52: UnimplementedInstruction(state); break;
		case 0x53: UnimplementedInstruction(state); break;
		case 0x54: UnimplementedInstruction(state); break;
		case 0x55: UnimplementedInstruction(state); break;
		case 0x56: 							//MOV D,M
			{
			uint16_t offset = (state->h<<8) | (state->l);
			state->d = state->memory[offset];
			}
			break;
		case 0x57: UnimplementedInstruction(state); break;
		case 0x58: UnimplementedInstruction(state); break;
		case 0x59: UnimplementedInstruction(state); break;
		case 0x5a: UnimplementedInstruction(state); break;
		case 0x5b: UnimplementedInstruction(state); break;
		case 0x5c: UnimplementedInstruction(state); break;
		case 0x5d: UnimplementedInstruction(state); break;
		case 0x5e: 							//MOV E,M
			{
			uint16_t offset = (state->h<<8) | (state->l);
			state->e = state->memory[offset];
			}
			break;
		case 0x5f: UnimplementedInstruction(state); break;
		case 0x60: UnimplementedInstruction(state); break;
		case 0x61: UnimplementedInstruction(state); break;
		case 0x62: UnimplementedInstruction(state); break;
		case 0x63: UnimplementedInstruction(state); break;
		case 0x64: UnimplementedInstruction(state); break;
		case 0x65: UnimplementedInstruction(state); break;
		case 0x66: 							//MOV H,M
			{
			uint16_t offset = (state->h<<8) | (state->l);
			state->h = state->memory[offset];
			}
			break;
		case 0x67: UnimplementedInstruction(state); break;
		case 0x68: UnimplementedInstruction(state); break;
		case 0x69: UnimplementedInstruction(state); break;
		case 0x6a: UnimplementedInstruction(state); break;
		case 0x6b: UnimplementedInstruction(state); break;
		case 0x6c: UnimplementedInstruction(state); break;
		case 0x6d: UnimplementedInstruction(state); break;
		case 0x6e: UnimplementedInstruction(state); break;
		case 0x6f: state->l = state->a; break; //MOV L,A
		case 0x70: UnimplementedInstruction(state); break;
		case 0x71: UnimplementedInstruction(state); break;
		case 0x72: UnimplementedInstruction(state); break;
		case 0x73: UnimplementedInstruction(state); break;
		case 0x74: UnimplementedInstruction(state); break;
		case 0x75: UnimplementedInstruction(state); break;
		case 0x76: UnimplementedInstruction(state); break;
		case 0x77: 							//MOV    M,A
			{
			uint16_t offset = (state->h<<8) | (state->l);
			state->memory[offset] = state->a;
			}
			break;
		case 0x78: UnimplementedInstruction(state); break;
		case 0x79: UnimplementedInstruction(state); break;
		case 0x7a: state->a  = state->d;  break;	//MOV D,A
		case 0x7b: state->a  = state->e;  break;	//MOV E,A
		case 0x7c: state->a  = state->h;  break;	//MOV H,A
		case 0x7d: UnimplementedInstruction(state); break;
		case 0x7e: 							//MOV A,M
			{
			uint16_t offset = (state->h<<8) | (state->l);
			state->a = state->memory[offset];
			}
			break;
		case 0x7f: UnimplementedInstruction(state); break;
		case 0x80: UnimplementedInstruction(state); break;
		case 0x81: UnimplementedInstruction(state); break;
		case 0x82: UnimplementedInstruction(state); break;
		case 0x83: UnimplementedInstruction(state); break;
		case 0x84: UnimplementedInstruction(state); break;
		case 0x85: UnimplementedInstruction(state); break;
		case 0x86: UnimplementedInstruction(state); break;
		case 0x87: UnimplementedInstruction(state); break;
		case 0x88: UnimplementedInstruction(state); break;
		case 0x89: UnimplementedInstruction(state); break;
		case 0x8a: UnimplementedInstruction(state); break;
		case 0x8b: UnimplementedInstruction(state); break;
		case 0x8c: UnimplementedInstruction(state); break;
		case 0x8d: UnimplementedInstruction(state); break;
		case 0x8e: UnimplementedInstruction(state); break;
		case 0x8f: UnimplementedInstruction(state); break;
		case 0x90: UnimplementedInstruction(state); break;
		case 0x91: UnimplementedInstruction(state); break;
		case 0x92: UnimplementedInstruction(state); break;
		case 0x93: UnimplementedInstruction(state); break;
		case 0x94: UnimplementedInstruction(state); break;
		case 0x95: UnimplementedInstruction(state); break;
		case 0x96: UnimplementedInstruction(state); break;
		case 0x97: UnimplementedInstruction(state); break;
		case 0x98: UnimplementedInstruction(state); break;
		case 0x99: UnimplementedInstruction(state); break;
		case 0x9a: UnimplementedInstruction(state); break;
		case 0x9b: UnimplementedInstruction(state); break;
		case 0x9c: UnimplementedInstruction(state); break;
		case 0x9d: UnimplementedInstruction(state); break;
		case 0x9e: UnimplementedInstruction(state); break;
		case 0x9f: UnimplementedInstruction(state); break;
		case 0xa0: UnimplementedInstruction(state); break;
		case 0xa1: UnimplementedInstruction(state); break;
		case 0xa2: UnimplementedInstruction(state); break;
		case 0xa3: UnimplementedInstruction(state); break;
		case 0xa4: UnimplementedInstruction(state); break;
		case 0xa5: UnimplementedInstruction(state); break;
		case 0xa6: UnimplementedInstruction(state); break;
		case 0xa7: state->a = state->a & state->a; LogicFlagsA(state);	break; //ANA A
		case 0xa8: UnimplementedInstruction(state); break;
		case 0xa9: UnimplementedInstruction(state); break;
		case 0xaa: UnimplementedInstruction(state); break;
		case 0xab: UnimplementedInstruction(state); break;
		case 0xac: UnimplementedInstruction(state); break;
		case 0xad: UnimplementedInstruction(state); break;
		case 0xae: UnimplementedInstruction(state); break;
		case 0xaf: state->a = state->a ^ state->a; LogicFlagsA(state);	break; //XRA A
		case 0xb0: UnimplementedInstruction(state); break;
		case 0xb1: UnimplementedInstruction(state); break;
		case 0xb2: UnimplementedInstruction(state); break;
		case 0xb3: UnimplementedInstruction(state); break;
		case 0xb4: UnimplementedInstruction(state); break;
		case 0xb5: UnimplementedInstruction(state); break;
		case 0xb6: UnimplementedInstruction(state); break;
		case 0xb7: UnimplementedInstruction(state); break;
		case 0xb8: UnimplementedInstruction(state); break;
		case 0xb9: UnimplementedInstruction(state); break;
		case 0xba: UnimplementedInstruction(state); break;
		case 0xbb: UnimplementedInstruction(state); break;
		case 0xbc: UnimplementedInstruction(state); break;
		case 0xbd: UnimplementedInstruction(state); break;
		case 0xbe: UnimplementedInstruction(state); break;
		case 0xbf: UnimplementedInstruction(state); break;
		case 0xc0: UnimplementedInstruction(state); break;
		case 0xc1: 						//POP    B
			{
				state->c = state->memory[state->sp];
				state->b = state->memory[state->sp+1];
				state->sp += 2;
			}
			break;
		case 0xc2: 						//JNZ address
			if (0 == state->cc.z)
				state->pc = (opcode[2] << 8) | opcode[1];
			else
				state->pc += 2;
			break;
		case 0xc3:						//JMP address
			state->pc = (opcode[2] << 8) | opcode[1];
			break;
		case 0xc4: UnimplementedInstruction(state); break;
		case 0xc5: 						//PUSH   B
			{
			state->memory[state->sp-1] = state->b;
			state->memory[state->sp-2] = state->c;
			state->sp = state->sp - 2;
			}
			break;
		case 0xc6: 						//ADI    byte
			{
			uint16_t x = (uint16_t) state->a + (uint16_t) opcode[1];
			state->cc.z = ((x & 0xff) == 0);
			state->cc.s = (0x80 == (x & 0x80));
			state->cc.p = parity((x&0xff), 8);
			state->cc.cy = (x > 0xff);
			state->a = (uint8_t) x;
			state->pc++;
			}
			break;
		case 0xc7: UnimplementedInstruction(state); break;
		case 0xc8:						//RZ
			if(state->cc.z == 1)
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
				state->sp += 2;
			}
			break;
		case 0xc9: 						//RET
			state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
			state->sp += 2;
			break;
		case 0xca: UnimplementedInstruction(state); break;
		case 0xcb: UnimplementedInstruction(state); break;
		case 0xcc: UnimplementedInstruction(state); break;
		case 0xcd: 						//CALL adr
			{
			uint16_t	ret = state->pc+2;
			state->memory[state->sp-1] = (ret >> 8) & 0xff;
			state->memory[state->sp-2] = (ret & 0xff);
			state->sp = state->sp - 2;
			state->pc = (opcode[2] << 8) | opcode[1];
			}
 			break;
		case 0xce: UnimplementedInstruction(state); break;
		case 0xcf: UnimplementedInstruction(state); break;
		case 0xd0: UnimplementedInstruction(state); break;
		case 0xd1: 						//POP    D
			{
				state->e = state->memory[state->sp];
				state->d = state->memory[state->sp+1];
				state->sp += 2;
			}
			break;
		case 0xd2: UnimplementedInstruction(state); break;
		case 0xd3: 
			//Don't know what to do here (yet)
			state->pc++;
			break;
		case 0xd4: UnimplementedInstruction(state); break;
		case 0xd5: 						//PUSH   D
			{
			state->memory[state->sp-1] = state->d;
			state->memory[state->sp-2] = state->e;
			state->sp = state->sp - 2;
			}
			break;
		case 0xd6: UnimplementedInstruction(state); break;
		case 0xd7: UnimplementedInstruction(state); break;
		case 0xd8: UnimplementedInstruction(state); break;
		case 0xd9: UnimplementedInstruction(state); break;
		case 0xda: UnimplementedInstruction(state); break;
		case 0xdb:
							 uint8_t port = opcode[1];
							 state->a = MachineIN(state, port);
							 state->pc++;
							 break;
		case 0xdc: UnimplementedInstruction(state); break;
		case 0xdd: UnimplementedInstruction(state); break;
		case 0xde: UnimplementedInstruction(state); break;
		case 0xdf: UnimplementedInstruction(state); break;
		case 0xe0: UnimplementedInstruction(state); break;
		case 0xe1: 					//POP    H
			{
				state->l = state->memory[state->sp];
				state->h = state->memory[state->sp+1];
				state->sp += 2;
			}
			break;
		case 0xe2: UnimplementedInstruction(state); break;
		case 0xe3: UnimplementedInstruction(state); break;
		case 0xe4: UnimplementedInstruction(state); break;
		case 0xe5: 						//PUSH   H
			{
			state->memory[state->sp-1] = state->h;
			state->memory[state->sp-2] = state->l;
			state->sp = state->sp - 2;
			}
			break;
		case 0xe6: 						//ANI    byte
			{
			state->a = state->a & opcode[1];
			LogicFlagsA(state);
			state->pc++;
			}
			break;
		case 0xe7: UnimplementedInstruction(state); break;
		case 0xe8: UnimplementedInstruction(state); break;
		case 0xe9: UnimplementedInstruction(state); break;
		case 0xea: UnimplementedInstruction(state); break;
		case 0xeb: 					//XCHG
			{
				uint8_t save1 = state->d;
				uint8_t save2 = state->e;
				state->d = state->h;
				state->e = state->l;
				state->h = save1;
				state->l = save2;
			}
			break;
		case 0xec: UnimplementedInstruction(state); break;
		case 0xed: UnimplementedInstruction(state); break;
		case 0xee: UnimplementedInstruction(state); break;
		case 0xef: UnimplementedInstruction(state); break;
		case 0xf0: UnimplementedInstruction(state); break;
		case 0xf1: 					//POP PSW
			{
				state->a = state->memory[state->sp+1];
				uint8_t psw = state->memory[state->sp];
				state->cc.z  = (0x01 == (psw & 0x01));
				state->cc.s  = (0x02 == (psw & 0x02));
				state->cc.p  = (0x04 == (psw & 0x04));
				state->cc.cy = (0x05 == (psw & 0x08));
				state->cc.ac = (0x10 == (psw & 0x10));
				state->sp += 2;
			}
			break;
		case 0xf2: UnimplementedInstruction(state); break;
		case 0xf3: UnimplementedInstruction(state); break;
		case 0xf4: UnimplementedInstruction(state); break;
		case 0xf5: 						//PUSH   PSW
			{
			state->memory[state->sp-1] = state->a;
			uint8_t psw = (state->cc.z |
							state->cc.s << 1 |
							state->cc.p << 2 |
							state->cc.cy << 3 |
							state->cc.ac << 4 );
			state->memory[state->sp-2] = psw;
			state->sp = state->sp - 2;
			}
			break;
		case 0xf6: UnimplementedInstruction(state); break;
		case 0xf7: UnimplementedInstruction(state); break;
		case 0xf8: UnimplementedInstruction(state); break;
		case 0xf9: UnimplementedInstruction(state); break;
		case 0xfa: UnimplementedInstruction(state); break;
		case 0xfb: state->int_enable = 1;  break;	//EI
		case 0xfc: UnimplementedInstruction(state); break;
		case 0xfd: UnimplementedInstruction(state); break;
		case 0xfe: 						//CPI  byte
			{
			uint8_t x = state->a - opcode[1];
			state->cc.z = (x == 0);
			state->cc.s = (0x80 == (x & 0x80));
			state->cc.p = parity(x, 8);
			state->cc.cy = (state->a < opcode[1]);
			state->pc++;
			}
			break;
		case 0xff: UnimplementedInstruction(state); break;
	}
	dbgprint("\t");
	dbgprint("%c", state->cc.z ? 'z' : '.');
	dbgprint("%c", state->cc.s ? 's' : '.');
	dbgprint("%c", state->cc.p ? 'p' : '.');
	dbgprint("%c", state->cc.cy ? 'c' : '.');
	dbgprint("%c  ", state->cc.ac ? 'a' : '.');
	dbgprint("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c,
				state->d, state->e, state->h, state->l, state->sp);
	return 0;
}

void ReadFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset)
{
	FILE *f= fopen(filename, "rb");
	if (f==NULL)
	{
		dbgprint("error: Couldn't open %s\n", filename);
		exit(1);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);
	
	uint8_t *buffer = &state->memory[offset];
	fread(buffer, fsize, 1, f);
	fclose(f);
}

State8080* Init8080(void)
{
	State8080* state = calloc(1,sizeof(State8080));
	state->memory = malloc(0x10000);  //16K
	return state;
}

void DumpScreenMem(State8080* state)
{
	int offset = 0x2400;
	uint8_t *buffer = &state->memory[offset];
	int k = 0;
	for(int i=0; i<0x4000; i += 0x20)
	{
		for(int j=0; j<0x20; j++)
		{
			printf("%02x", buffer[k++]);
		}
		printf("\n");
	}
}

void Push(State8080* state, uint8_t lw, uint8_t hw)
{
	state->memory[state->sp-1] = lw;
	state->memory[state->sp-2] = hw;
	state->sp = state->sp - 2;
}

void GenerateInterrupt(State8080* state, int interrupt_num)
{
	Push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));

	//Set the PC to the low memory vector.
	//This is identical to an "RST interrupt_num" instruction.
	state->pc = 8 * interrupt_num;
}

int main (int argc, char**argv)
{
	int done = 0;
	int vblankcycles = 0;
	int lastInterrupt = 0;
	State8080* state = Init8080();
	
	ReadFileIntoMemoryAt(state, "invaders.h", 0);
	ReadFileIntoMemoryAt(state, "invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "invaders.e", 0x1800);
	
	while (done == 0)
	{
		done = Emulate8080Op(state);
		//DumpScreenMem(state);
		if (time(0) - lastInterrupt > 1.0/60.0)  //1/60 second has elapsed
		{
			//only do an interrupt if they are enabled
			if (state->int_enable)
			{
				GenerateInterrupt(state, 2);    //interrupt 2
				lastInterrupt = time(0);
			}
		}
	}
	return 0;
}
