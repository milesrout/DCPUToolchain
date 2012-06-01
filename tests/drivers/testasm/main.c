/**

	File:		main.c

	Project:	DCPU-16 Tools
	Component:	Assembler Test Suite

	Authors:	James Rhodes

	Description:	Main entry point.

**/

#include <stdlib.h>
#include <stdio.h>
#include <argtable2.h>
#include <bstring.h>
#include <bfile.h>
#include <version.h>
#include <debug.h>

int main(int argc, char* argv[])
{
	bstring ldargs = bfromcstr("");
	int i, result;
	unsigned int match = 0, unmatch = 0;
	char ca, ce;
	BFILE* expect;
	BFILE* actual;

	// Define arguments.
	struct arg_lit* show_help = arg_lit0("h", "help", "Show this help.");
	struct arg_lit* gen_relocatable = arg_lit0("r", "relocatable", "Generate relocatable code.");
	struct arg_lit* gen_intermediate = arg_lit0("i", "intermediate", "Generate intermediate code for use with the linker.");
	struct arg_file* input_file = arg_file1(NULL, NULL, "<file>", "The input assembly file.");
	struct arg_file* expect_file = arg_file0("e", "expect", "<file>", "The output file that contains expected output.");
	struct arg_file* actual_file = arg_file1("a", "actual", "<file>", "The output file where actual output will be placed.");
	struct arg_file* symbols_file = arg_file0("s", "debug-symbols", "<file>", "The debugging symbol output file.");
	struct arg_lit* fail_opt = arg_lit0("f", "fail", "The assembler is expected to fail and the actual output file should not exist on completion.");
	struct arg_lit* verbose = arg_litn("v", NULL, 0, LEVEL_EVERYTHING - LEVEL_DEFAULT, "Increase verbosity.");
	struct arg_lit* quiet = arg_litn("q", NULL,  0, LEVEL_DEFAULT - LEVEL_SILENT, "Decrease verbosity.");
	struct arg_end* end = arg_end(20);
	void* argtable[] = { show_help, gen_relocatable, gen_intermediate, symbols_file, input_file, expect_file, actual_file, fail_opt, verbose, quiet, end };

	// Parse arguments.
	int nerrors = arg_parse(argc, argv, argtable);

	version_print(bautofree(bfromcstr("Assembler Test Driver")));
	if (nerrors != 0 || show_help->count != 0 || (fail_opt->count == 0 && (expect_file->count == 0 || actual_file->count == 0)))
	{
		if (show_help->count != 0 && fail_opt->count == 0 && (expect_file->count == 0 || actual_file->count == 0))
			printd(LEVEL_ERROR, "error: you must provide either -f or -e and -a.\n");
		if (show_help->count != 0)
			arg_print_errors(stderr, end, "testasm");

		printd(LEVEL_DEFAULT, "syntax:\n    testasm");
		arg_print_syntax(stderr, argtable, "\n");
		printd(LEVEL_DEFAULT, "options:\n");
		arg_print_glossary(stderr, argtable, "	  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	// Set verbosity level.
	debug_setlevel(LEVEL_DEFAULT + verbose->count - quiet->count);

	// Generate the argument list for the assembler.
	ldargs = bfromcstr(argv[0]);
	bfindreplace(ldargs, bfromcstr("testasm"), bfromcstr("dtasm"), 0);
	binsertch(ldargs, 0, 1, '"');
	bconchar(ldargs, '"');
	bconchar(ldargs, ' ');

	// Verbosity options.
	if (verbose->count > 0)
	{
		bconchar(ldargs, '-');
		for (i = 0; i < verbose->count; i++) bconchar(ldargs, 'v');
		bconchar(ldargs, ' ');
	}
	if (quiet->count > 0)
	{
		bconchar(ldargs, '-');
		for (i = 0; i < quiet->count; i++) bconchar(ldargs, 'q');
		bconchar(ldargs, ' ');
	}

	// Literal options.
	if (gen_relocatable->count > 0)
	{
		bconchar(ldargs, '-');
		for (i = 0; i < gen_relocatable->count; i++) bconchar(ldargs, 'r');
		bconchar(ldargs, ' ');
	}
	if (gen_intermediate->count > 0)
	{
		bconchar(ldargs, '-');
		for (i = 0; i < gen_intermediate->count; i++) bconchar(ldargs, 'i');
		bconchar(ldargs, ' ');
	}

	// Unlink the actual file so that if we are expecting
	// failure, we won't return incorrectly.
	unlink(actual_file->filename[0]);

	// Output file.
	bcatcstr(ldargs, "-o \"");
	bcatcstr(ldargs, actual_file->filename[0]);
	bcatcstr(ldargs, "\" ");

	// Input file.
	bcatcstr(ldargs, "\"");
	bcatcstr(ldargs, input_file->filename[0]);
	bcatcstr(ldargs, "\" ");

	// Windows needs the whole command wrapped in quotes and slashes to be correct.
	// See http://stackoverflow.com/questions/2642551/windows-c-system-call-with-spaces-in-command.
#ifdef _WIN32
	binsertch(ldargs, 0, 1, '"');
	bconchar(ldargs, '"');
#endif

	// Now run the assembler!
	result = system(ldargs->data);
	if (result != 0 && fail_opt->count == 0)
	{
		// Assembler returned error exit code.
		printd(LEVEL_ERROR, "error: expected success but assembler returned non-zero exit code (%i).\n", result);
		return 1;
	}
	else if (result == 0 && fail_opt->count >= 1)
	{
		// Assembler returned zero when failure was expected.
		printd(LEVEL_ERROR, "error: expected failure but assembler returned zero exit code.\n");
		return 1;
	}
	else if (result != 0 && fail_opt->count >= 1)
	{
		// Assembler failed and we expected it to.  Return success only
		// if the output file does not exist.
		actual = bopen(actual_file->filename[0], "rb");
		if (actual != NULL)
		{
			printd(LEVEL_ERROR, "error: expected failure but actual output file exists.\n");
			bclose(actual);
			return 1;
		}
		return 0;
	}

	// Open expect data.
	expect = bopen(expect_file->filename[0], "rb");
	if (expect == NULL)
	{
		// The expect file was not provided.
		printd(LEVEL_ERROR, "error: path to expect file does not exist.\n");
		return 1;
	}

	// Open actual data.
	actual = bopen(actual_file->filename[0], "rb");
	if (actual == NULL)
	{
		// The expect file was not provided.
		bclose(expect);
		printd(LEVEL_ERROR, "error: expected data but actual output file does not exist after running assembler.\n");
		return 1;
	}

	// Now compare raw bytes.
	while (true)
	{
		if (!beof(actual) && !beof(expect))
		{
			ca = bgetc(actual);
			ce = bgetc(expect);
			if (ca == ce)
				match++;
			else
			{
				printd(LEVEL_WARNING, "warning: byte at 0x%04X is different (got 0x%02X, expected 0x%02X)!\n", btell(actual), ca, ce);
				unmatch++;
			}
		}
		else if (!beof(actual))
		{
			ca = bgetc(actual);
			printd(LEVEL_ERROR, "error: actual output contained trailing byte 0x%02X.\n", (unsigned char)ca);
			unmatch++;
		}
		else if (!beof(expect))
		{
			ce = bgetc(expect);
			printd(LEVEL_ERROR, "error: expected actual output to contain 0x%02X.\n", (unsigned char)ce);
			unmatch++;
		}
		else
			break;
	}
	if (unmatch > 0)
	{
		printd(LEVEL_ERROR, "error: actual output differs from expected output in content (%f%%, %i bytes different).\n", 100.f / (unmatch + match) * unmatch, unmatch);
		if (btell(actual) != btell(expect))
			printd(LEVEL_ERROR, "error: actual output differs from expected output in length (%i bytes larger).\n", btell(actual) - btell(expect));
		bclose(actual);
		bclose(expect);
		return 1;
	}

	// Close files and delete actual because we have
	// succeeded.
	bclose(actual);
	bclose(expect);
	unlink(actual_file->filename[0]);

	return 0;
}