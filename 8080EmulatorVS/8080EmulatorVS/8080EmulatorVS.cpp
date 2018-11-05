// 8080EmulatorVS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <SDL.h>
#undef main

using namespace std;

typedef struct ConditionCodes {
	uint8_t z : 1;
	uint8_t s : 1;
	uint8_t p : 1;
	uint8_t cy : 1;
	uint8_t ac : 1;
	uint8_t pad : 3;
} ConditionCodes;

typedef struct State8080 {
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint16_t sp;
	uint16_t pc;
	uint8_t *memory;
	struct ConditionCodes cc;
	uint8_t int_enable;
} State8080;

int PrintAssembly(unsigned char*, int);
void Emulate(State8080* state);



int main(int argc, char** argv)
{
	// Open ROM file
	FILE *f;
	int errorcode = fopen_s(&f, argv[1], "rb");
	if (errorcode != 0)
	{
		printf("Error: Could not open file %s\n", argv[1]);
		exit(1);
	}

	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	// Allocate space in memory for the ROM
	unsigned char *buffer = (unsigned char*)malloc(fsize);

	// Create the registers and memory of the 8080
	State8080* state = new State8080();

	// Allocate memory to store the ROM
	state->memory = (uint8_t*)malloc(fsize);
	
	fread(state->memory, fsize, 1, f);
	fclose(f);

	//// Print ROM assembly instructions
	//int pc = 0;
	//while (pc < fsize)
	//{
	//	pc += PrintAssembly(buffer, pc);
	//}

	// Emulate 100 instructions
	for (int i = 0; i < 100; i++)
	{
		Emulate(state);
	}
	

	return 0;
}

void Emulate(State8080* state)
{
	unsigned char *opcode = &state->memory[state->pc];
	printf("Currently running instruction 0x%x at address 0x%x\n", *opcode, state->pc);

	uint16_t answer = 0;
	uint16_t offset = 0;
	switch (*opcode)
	{
	case 0x00: 
		break;
	case 0x01: 
		state->c = opcode[2];
		state->b = opcode[1];
		state->pc += 2;
		break;
	case 0x06:
		state->b = opcode[1];
		state->pc += 1;
		break;
	case 0x31:
		state->sp = opcode[2] * 256 + opcode[1];
		state->pc += 2;
	case 0x41:
		state->b = state->c;
		break;
	case 0x42:
		state->b = state->d;
		break;
	case 0x43:
		state->b = state->e;
		break;
	case 0x80:
		answer = (uint16_t)state->a + (uint16_t)state->b;
		// TODO: put this in function
		state->cc.z = ((answer & 0xff) == 0);
		state->cc.s = ((answer & 0x80) != 0);
		state->cc.cy = (answer > 0xff);
		state->a = answer & 0xff;
		break;
	case 0x81:
		answer = (uint16_t)state->a + (uint16_t)state->c;
		state->cc.z = ((answer & 0xff) == 0);
		state->cc.s = ((answer & 0x80) != 0);
		state->cc.cy = (answer > 0xff);
		state->a = answer & 0xff;
		break;
	case 0x86:
		offset = state->h * 256 + state->l;
		answer = (uint16_t)state->a + state->memory[offset];
		state->cc.z = ((answer & 0xff) == 0);
		state->cc.s = ((answer & 0x80) != 0);
		state->cc.cy = (answer > 0xff);
		state->a = answer & 0xff;
		break;
	case 0xc2:
		if (state->cc.z == 0)
		{
			state->pc = opcode[2] * 256 + opcode[1] - 1;
		}
		else
		{
			state->pc += 2;
		}
	case 0xc3:
		state->pc = opcode[2] * 256 + opcode[1] - 1;
		break;
	case 0xc6:
		answer = (uint16_t)state->a + (uint16_t)opcode[1];
		state->cc.z = ((answer & 0xff) == 0);
		state->cc.s = ((answer & 0x80) != 0);
		state->cc.cy = (answer > 0xff);
		state->a = answer & 0xff;
		break;
	default: printf("Error: Instruction 0x%x not implemented.", *opcode); exit(1);
	}
	state->pc += 1;
}

int PrintAssembly(unsigned char* codebuffer, int pc)
{
	unsigned char *code = &codebuffer[pc];
	int opbytes = 1;
	printf("%04x ", pc);
	switch (*code)
	{
	case 0x00: printf("NOP"); break;
	case 0x01: printf("LXI B,#$%02x%02x", code[2], code[1]); opbytes = 3; break;
	case 0x02: printf("STAX B"); break;
	case 0x03: printf("INX B"); break;
	case 0x04: printf("INR B"); break;
	case 0x05: printf("DCR B"); break;
	case 0x06: printf("MVI B,#$%02x", code[1]); opbytes = 2; break;
	case 0x07: printf("RLC"); break;
	case 0x08: printf("NOP"); break;
	case 0x3e: printf("MVI A,#0x%02x", code[1]); opbytes = 2; break;
	case 0xc3: printf("JMP $%02x%02x", code[2], code[1]); opbytes = 3; break;
	default: printf("Instruction not implemented.");
	}

	printf("\n");

	return opbytes;
}

/*

void PutPixel32_nolock(SDL_Surface*, int, int, Uint32);

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		cout << "SDL_Init Error: " << SDL_GetError() << endl;
		return 1;
	}
	
	SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr)
	{
		SDL_DestroyWindow(win);
		cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	SDL_Surface *surface = SDL_CreateRGBSurface(0, 640, 480, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	if (surface == nullptr)
	{
		SDL_DestroyWindow(win);
		cout << "Error: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	for (int i = 0; i < 640; i++)
	{
		for (int j = 0; j < 480; j++)
		{
			PutPixel32_nolock(surface, i, j, 0x00000000);
		}		
	}
	

    return 0;
}

void PutPixel32_nolock(SDL_Surface * surface, int x, int y, Uint32 color)
{
	Uint8 * pixel = (Uint8*)surface->pixels;
	pixel += (y * surface->pitch) + (x * sizeof(Uint32));
	*((Uint32*)pixel) = color;
}

*/