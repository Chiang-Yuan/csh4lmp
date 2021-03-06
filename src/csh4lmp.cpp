﻿/* -*- C++ -*------------------------------------------------------------
 *
 *	csh4lmp.exe
 *
 *	USAGE
 *
 *	LIMITATION
 *		- This program temporarily support only atoms, bonds, and angles
 *
 *	DEPENDENCY
 *
 *  Copyright version 6.0 (4/Oct/2020) Chiang Yuan
 *
 *		v_6.0	|	support dreierketten rule
 *		v_5.6	|	add 29Si NMR check
 *		v_5.4	|	adjust initialization
 *		v_5.3	|	adjust topology & charge balnce
 *		v_5.2	|	Bugfix
 *		v_5.1	|	establish two modes deleting SiO2 and two modes add H
 *		v_5.0	|	add feature of appending H atoms
 *		v_4.1	|	fix bugs and add BT/PT ratio
 *		v_4.0	|	add batch run feature
 *		v_3.1	|	fix bugs and add progress bar
 *		v_3.0	|	improve algorithm of deleting atoms and bonds and enhance
 *					the topology robustness, and add the benchmark of cutting
 *					C-S-H chains
 *		v_2.1	|	bug fixed
 *      v_1.0   |   build this script
 *		v_2.0	|	add angles
 * ---------------------------------------------------------------------- */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "stl2lmp.h"
#include "system.h"
#include "error.h"
#include "read_data.h"
#include "write_data.h"

#include "modify.h"
#include "change_box.h"

#include "affine_transform.h"


using namespace std;

int count_words(const char* line) {
	int n = strlen(line) + 1;
	char *copy = new char[n];
	strcpy(copy, line);

	char *ptr;
	if ((ptr = strchr(copy, '#'))) *ptr = ' ';

	if (strtok(copy, WHITESPACE) == NULL) {
		delete[] copy;
		return 0;
	}

	n = 1;
	while (strtok(NULL, WHITESPACE)) n++;
	delete[] copy;
	return n;
}

bool check_arg(char **arg, const char *flag, int num, int argc) {

	if (num >= argc) {
		printf("Missing argument for \"%s\" flag\n", flag);
		return false;
	}

	if (arg[num][0] == '-' || arg[num][0] == '<' || arg[num][0] == '>') {
		printf("Incorrect argument to \"%s\" flag: %s", flag, arg[num]);
		return false;
	}
	return true;
}

int main(int argc, char *argv[]) {

	// system

	Error error;
	System sys(&error);
	ReadData reader(&error);
	WriteData writer(&error);

	// modifier

	Initialize init(&error);
	Topology topo(&error);
	AddH2O addH2O(&error);
	ModifyH2O rmH2O(&error);
	ModifySiO2 rmSiO2(&error);
	ModifyH addH(&error);
	Modify delt(&error);

	ChangeBox p2m(&error);

	// temporary

	AffineTransform rot(&error);

	if (argc < 2) {
		printf("\n\t|                                                                     |\n");
		printf("\t|---------------------------------------------------------------------|\n");
		printf("\t|                     CSH4LMP  (C-S-H for LAMMPS)                     |\n");
		printf("\t|---------------------------------------------------------------------|\n");
		printf("\t|                                                                     |\n");
		printf("\t|  VERSION: %18s                                        |\n", STL2LMP_VERSION);
		printf("\t|  COMMAND:                                                           |\n");
		printf("\t|     -I/O            |    < [intput file name]                       |\n");
		printf("\t|                     |    > [output file name] hint (y/n)            |\n");
		printf("\t|     -add water      |    -aw add [#] [typeO] [typeH]                |\n");
		printf("\t|     -delete water   |    -dw delete [typeO] [typeH]                 |\n");
		printf("\t|     -remove SiO2    |    -s number [typeSi] [#] [BT/PT ratio]       |\n");
		printf("\t|     -add H          |    -h valence (n/#) [typeO] [typeH] [typeO-H] |\n");
		printf("\t|     -delete atom    |    -d delete atom type [type]                 |\n");
		printf("\t|     -delete bond    |    -d delete bond type [type]                 |\n");
		printf("\t|                                                                     |\n");
		printf("\t|                 Chiang Yuan  (c)  All  Right  Reserved              |\n\n");
		//printf("usage command: < (input file name) [-w delete type #] [-w add type #] [-s number (type#) #] [-h valence n/# (typeO) (typeH) (typeO-H)] [> (output file name) hint y/n]\n");
		char* buffer = new char[MAX_STRING];
		fgets(buffer, MAX_STRING, stdin);

		argc = count_words(buffer);
		argv = new char*[argc];

		argv[0] = strtok(buffer, WHITESPACE);
		if (argv[0] == NULL) {
			return error.message("Incorrect input commnad line", 1);
		}

		for (int i = 1; i < argc; i++) {
			argv[i] = strtok(NULL, WHITESPACE);
			if (argv[i] == NULL) {
				return error.message("Missing argument in command line", 2);
			}
		}

		int n = 0;
		while (n < argc)
		{
			if (strncmp(argv[n], "<", 1) == 0) {
				int ncomm = 1;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "read_data", n, argc)) return error.message("", 3);
					commd[i] = argv[n];
				}
				printf("ReadData::command(): %d\n", reader.command(ncomm, commd, sys));

			}

			if (strncmp(argv[n], "-r", 2) == 0) {
				printf("AffineTransform::command(): %d\n", rot.command(sys));
			}

			if (strncmp(argv[n], "-t", 2) == 0) {
				printf("Topology::command(): %d\n", topo.command(sys));
			}

			if (strncmp(argv[n], "-i", 2) == 0) {
				printf("Initialize::command(): %d\n", init.command(sys));
			}

			if (strncmp(argv[n], "-aw", 3) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 4);
					commd[i] = argv[n];
				}
				printf("AddH2O::command(): %d\n", addH2O.command(ncomm, commd, sys));
				cout << sys;
			}

			if (strncmp(argv[n], "-dw", 3) == 0) {
				int ncomm = 3;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 4);
					commd[i] = argv[n];
				}
				printf("Modify::command(): %d\n", rmH2O.command(ncomm, commd, sys));
				cout << sys;
			}

			if (strncmp(argv[n], "-s", 2) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 5);
					commd[i] = argv[n];
				}
				printf("Topology::command(): %d\n", topo.command(sys));
				printf("ModifySiO2::command(): %d\n", rmSiO2.command(ncomm, commd, sys));
				printf("Topology::command(): %d\n", topo.command(sys));
			}

			if (strncmp(argv[n], "-h", 2) == 0) {
				int ncomm = 5;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 7);
					commd[i] = argv[n];
				}

				printf("ModifyH::command(): %d\n", addH.command(ncomm, commd, sys));

				cout << sys;
			}

			if (strncmp(argv[n], "-d", 2) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 8);
					commd[i] = argv[n];
				}

				printf("Modify::command(): %d\n", delt.command(ncomm, commd, sys));

				cout << sys;
			}

			if (strncmp(argv[n], "-c", 2) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "change_box", n, argc)) return error.message("", 9);
					commd[i] = argv[n];
				}

				printf("ChangeBox::command(): %d\n", p2m.command(ncomm, commd, sys));

			}

			if (strncmp(argv[n], ">", 1) == 0) {
				int ncomm = 3;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "write_data", n, argc)) return error.message("", 6);
					commd[i] = argv[n];
				}

				printf("WriteData::commd(): %d\n", writer.command(ncomm, commd, sys));
			}

			if (argv[n] == NULL) {
				return error.message("Missing argument in command line", 5);
			}
			n++;
		}
		system("pause");
	}
	else {
		time_t localtime;
		time(&localtime);
		printf("=====================================================================\n");
		printf("CSH4LAMMPS (%s) starts at %s", STL2LMP_VERSION, ctime(&localtime));
		printf("=====================================================================\n");

		int n = 1;
		while (n < argc)
		{
			if (strncmp(argv[n], "<", 1) == 0) {
				int ncomm = 1;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "read_data", n, argc)) return error.message("", 3);
					commd[i] = argv[n];
				}
				printf("End ReadData: %d\n", reader.command(ncomm, commd, sys));

			}

			if (strncmp(argv[n], "-r", 2) == 0) {
				printf("End AffineTransform: %d\n", rot.command(sys));
			}

			if (strncmp(argv[n], "-t", 2) == 0) {
				printf("End Topology: %d\n", topo.command(sys));
			}

			if (strncmp(argv[n], "-i", 2) == 0) {
				printf("End Initialize: %d\n", init.command(sys));
			}

			if (strncmp(argv[n], "-aw", 3) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 4);
					commd[i] = argv[n];
				}
				printf("End AddH2O: %d\n", addH2O.command(ncomm, commd, sys));
				cout << sys;
			}

			if (strncmp(argv[n], "-dw", 3) == 0) {
				int ncomm = 3;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 4);
					commd[i] = argv[n];
				}
				printf("End Modify: %d\n", rmH2O.command(ncomm, commd, sys));
				cout << sys;
			}

			if (strncmp(argv[n], "-s", 2) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 5);
					commd[i] = argv[n];
				}
				printf("End Topology: %d\n", topo.command(sys));
				printf("End ModifySiO2: %d\n", rmSiO2.command(ncomm, commd, sys));
				printf("End Topology: %d\n", topo.command(sys));
			}

			if (strncmp(argv[n], "-h", 2) == 0) {
				int ncomm = 5;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 7);
					commd[i] = argv[n];
				}

				printf("End ModifyH: %d\n", addH.command(ncomm, commd, sys));

				cout << sys;
			}

			if (strncmp(argv[n], "-d", 2) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "modify", n, argc)) return error.message("", 8);
					commd[i] = argv[n];
				}

				printf("End Modify: %d\n", delt.command(ncomm, commd, sys));

				cout << sys;
			}

			if (strncmp(argv[n], "-c", 2) == 0) {
				int ncomm = 4;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "change_box", n, argc)) return error.message("", 9);
					commd[i] = argv[n];
				}

				printf("End ChangeBox: %d\n", p2m.command(ncomm, commd, sys));

			}


			if (strncmp(argv[n], ">", 1) == 0) {
				int ncomm = 3;
				char** commd = new char*[ncomm];

				for (int i = 0; i < ncomm; i++) {
					n++;
					if (!check_arg(argv, "write_data", n, argc)) return error.message("", 6);
					commd[i] = argv[n];
				}

				printf("End WriteData: %d:\n", writer.command(ncomm, commd, sys));
			}

			if (argv[n] == NULL) {
				return error.message("Missing argument in command line", 5);
			}
			n++;
		}
		time(&localtime);
		printf("=====================================================================\n");
		printf("CSH4LAMMPS (%s) ends at %s", STL2LMP_VERSION, ctime(&localtime));
		printf("=====================================================================\n");
	}

	return 0;
}