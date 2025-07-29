
#include "8080emu.h"
#include "shared_vars.h"

#define PRINTOPS 0

uint8_t inputport1 = 0x8;
int vertical = 0;

int main (int argc, char**argv)
{
	system("clear");
#if PRINTOPS
#else
	initscr();
	noecho();
	nl();
#endif

	timeout(0);

	int done = 0;
	int cycles = 0;
	State8080* state = Init8080();
	
	ReadFileIntoMemoryAt(state, "invaders.h", 0);
	ReadFileIntoMemoryAt(state, "invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "invaders.e", 0x1800);

	int nextInterrupt = 1;
	int timems = 0;
	int dtime = 0;

	char ch = 0;
	bool coindown = false;
	bool p1button = false;
	bool p2button = false;
	
	while (!done)
	{
		ch = getch();
		if(ch == 'v')	vertical = (vertical + 1) % 3;
		if(ch == 'q')	done = 1;
		if(ch == 'c')	coindown = !coindown;
		if(ch == '1')	p1button = !p1button;
		if(ch == '2')	p2button = !p2button;

		if(coindown)
		{
			inputport1 ^= 1;
			coindown = false;
		}
		if(p1button)
		{
			inputport1 ^= 4;
			p1button = false;
		}
		if(p2button)
		{
			inputport1 ^= 2;
			p2button = false;
		}

		cycles = 0;
		timems = currentTime();
		while(cycles < (2000000 / 30))	//8080 ran at 2MHz.  2 million cycles per second.  video interrupts are every 1/30 of a second.
		{
			cycles += Emulate8080Op(state);
		}

		do
		{
			cycles += Emulate8080Op(state);
		}while(!state->int_enable);
		
		dtime = currentTime() - timems;
		if(dtime < 1000000/30)
		{
			usleep( 1000000/30 - dtime );
			//printf("sleeping %d us.\n", ( 1000000/30 - dtime ));
		}
		if (state->int_enable)
		{
			GenerateInterrupt(state, nextInterrupt);
			if(nextInterrupt == 2)	nextInterrupt = 1;
			else nextInterrupt = 2;
		}
		if(!vertical)
			DrawScreenMem(state, 0);
		else
			DrawScreenMem(state, 0x20 * 80 * vertical);
	}
	endwin();
	return 0;
}
