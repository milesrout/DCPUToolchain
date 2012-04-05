//
// DCPU Emulator by James Rhodes
//
// Main entry point.
//

#include <stdio.h>
#include <cstdlib>
#include "dcpu.h"

int main(int argc, char* argv[])
{
	// Define our variables.
	FILE* load;
	uint16_t flash[0x10000];
	unsigned int i, a;
	int cread;
	bool uread = true;

	// Zero out the flash space.
	for (i = 0; i < 0x10000; i++)
		flash[i] = 0x0;

	// See if we have the right number of arguments.
	if (argc != 2)
	{
		printf("Invalid number of arguments.  Expected memory flash filename for simulation to run.\n");
		exit(1);
	}
	
	// Open file.
	load = fopen(argv[1], "rb");
	if (load == NULL)
	{
		printf("Unable to load %s from disk.\n", argv[1]);
		exit(1);
	}

	// Read up to 0x20000 bytes (as 16-bit words).
	a = 0;
	for (i = 0; i < 0x20000; i++)
	{
		cread = fgetc(load);
		if (cread == -1) break;
		if (uread)
			cread <<= 8;
		flash[a] += cread;
		if (!uread)
			a += 1;
		uread = !uread;
	}
	fclose(load);

	// 7c010030
	uint16_t inst = INSTRUCTION_CREATE(OP_ADD, NXT, NXT_LIT);
	uint16_t a_a = INSTRUCTION_GET_A(flash[0]);
	uint16_t a_b = INSTRUCTION_GET_B(flash[0]);
	uint16_t op = INSTRUCTION_GET_OP(flash[0]);

	// And then use the VM.
	vm_t* vm = vm_create();
	vm_flash(vm, flash);
	vm_execute(vm);
	vm_free(vm);

	return 0;
}