#include "write_data.h"



WriteData::WriteData(Error * error_)
{
	error = error_;
}

WriteData::~WriteData()
{
}

int WriteData::command(int argc, char * argv[], System & sys)
{
	int n = 1;
	while (n < argc)
	{
		if (strncmp(argv[n], "hint", 4) == 0) {
			int ncomm = 1;
			char** commd = new char*[ncomm];

			for (int i = 0; i < ncomm; i++) {
				n++;
				if (!check_arg(argv, "hint", n, argc)) return error->message("", 1);
				commd[i] = argv[n];
			}

			if (strncmp(commd[0], "y", 1) == 0 || strncmp(commd[0], "yes", 3) == 0) {
				printf("WriteData::write(): %d\n", write(argv[0], 1, sys));
			}
			else if (strncmp(commd[0], "n", 1) == 0 || strncmp(commd[0], "no", 2) == 0) {
				printf("WriteData::write(): %d\n", write(argv[0], 0, sys));
			}
			else return error->message("Invalid write argument: %s", 2, commd[0]);
		}

		n++;
	}
	return 0;
}

bool WriteData::check_arg(char ** arg, const char * flag, int num, int argc)
{
	if (num >= argc) {
		printf("Missing argument for \"%s\" flag\n", flag);
		return false;
	}

	return true;
}

int WriteData::write(char * filename_, int hint_flag, System & sys)
{

	sprintf(buffer, "%s", filename_);
	file = fopen(buffer, "w");
	if (file == NULL) {
		return error->message("Cannot open \"%s\" file\n", 1, filename_);
	}

	time_t localtime;
	time(&localtime);
	fprintf(file, "LAMMPS data file generated by CSH4LMP " STL2LMP_VERSION " on %s\n", ctime(&localtime));

	fprintf(file, " %6lld atoms\n", sys.natoms);
	fprintf(file, " %6lld bonds\n", sys.nbonds);
	fprintf(file, " %6lld angles\n", sys.nangles);

	fprintf(file, "\n");

	fprintf(file, " %3d atom types\n", sys.no_atom_types);
	if (sys.no_bond_types > 0)
		fprintf(file, " %3d bond types\n", sys.no_bond_types);
	if (sys.no_angle_types > 0)
		fprintf(file, " %3d angle types\n", sys.no_angle_types);

	fprintf(file, "\n");

	if (sys.triclinic_flag == 0) {
		fprintf(file, " %16.9e %16.9e xlo xhi\n", sys.box[0][0], sys.box[0][1]);
		fprintf(file, " %16.9e %16.9e ylo yhi\n", sys.box[1][0], sys.box[1][1]);
		fprintf(file, " %16.9e %16.9e zlo zhi\n", sys.box[2][0], sys.box[2][1]);
	}
	else {
		fprintf(file, " %16.9e %16.9e xlo xhi\n", sys.box[0][0], sys.box[0][1]);
		fprintf(file, " %16.9e %16.9e ylo yhi\n", sys.box[1][0], sys.box[1][1]);
		fprintf(file, " %16.9e %16.9e zlo zhi\n", sys.box[2][0], sys.box[2][1]);
		fprintf(file, " %16.9e %16.9e %16.9e xy xz yz\n", sys.box[2][2], sys.box[1][2], sys.box[0][2]);
	}

	// Section : Masses
	fprintf(file, "\nMasses\n\n");
	for (int i = 0; i < sys.no_atom_types; i++) {
		fprintf(file, " %3d %10.6f", i + 1, sys.atomTypes[i].mass);

		if (hint_flag) fprintf(file, " # %s\n",	sys.atomTypes[i].element);
		else fprintf(file, "\n");
	}

	// Section : Pair Coeffs
	fprintf(file, "\nPair Coeffs\n\n");
	for (int i = 0; i < sys.no_atom_types; i++) {
		fprintf(file, " %3d %14.10e %14.10e", i + 1, sys.atomTypes[i].coeff[0], sys.atomTypes[i].coeff[1]);

		if (hint_flag) fprintf(file, " # %s\n", sys.atomTypes[i].element);
		else fprintf(file, "\n");
	}

	// Section : Bond Coeffs
	if (sys.no_bond_types > 0) {
		fprintf(file, "\nBond Coeffs\n\n");
		for (int i = 0; i < sys.no_bond_types; i++) {
			fprintf(file, " %3d %10.4e %10.4e",
				i + 1,
				sys.bondTypes[i].coeff[0],
				sys.bondTypes[i].coeff[1]);

			if (hint_flag) fprintf(file, " # %s-%s\n",
				sys.atomTypes[sys.bondTypes[i].IJType[0] - 1].element,
				sys.atomTypes[sys.bondTypes[i].IJType[1] - 1].element);
			else fprintf(file, "\n");
		}
	}

	// Section : Angle Coeffs
	if (sys.no_angle_types > 0) {
		fprintf(file, "\nAngle Coeffs\n\n");
		for (int i = 0; i < sys.no_angle_types; i++) {
			fprintf(file, " %3d %10.4e %10.4e",
				i + 1,
				sys.angleTypes[i].coeff[0],
				sys.angleTypes[i].coeff[1]);

			if (hint_flag) fprintf(file, " # %s-%s-%s\n",
				sys.atomTypes[sys.angleTypes[i].types[0] - 1].element,
				sys.atomTypes[sys.angleTypes[i].types[1] - 1].element,
				sys.atomTypes[sys.angleTypes[i].types[2] - 1].element);
			else fprintf(file, "\n");
		}
	}

	// Section : Atoms
	if (hint_flag) fputs("\nAtoms # full\n\n", file);
	else fputs("\nAtoms\n\n", file);
	for (auto a : sys.atoms) {
		fprintf(file, " %6lld %6d %3d %16.9e %16.9e %16.9e %16.9e %3d %3d %3d",
			a.id, a.molecule, a.type, a.q,
			a.x[0], a.x[1], a.x[2], a.n[0], a.n[1], a.n[2]
		);

		if (hint_flag) fprintf(file, " # %s\n", sys.atomTypes[a.type - 1].element);
		else fprintf(file, "\n");
	}


	// Section : Bonds
	if (sys.nbonds > 0) {
		fprintf(file, "\nBonds\n\n");
		for (auto b : sys.bonds)
			fprintf(file, " %6lld %3d %6lld %6lld\n",
				b.id, b.type, b.ij[0]->id, b.ij[1]->id);
	}

	// Section : Angles
	if (sys.nangles > 0) {
		fprintf(file, "\nAngles\n\n");
		for (auto a : sys.angles)
			fprintf(file, " %6lld %3d %6lld %6lld %6lld\n",
				a.id, a.type, a.ijk[0]->id, a.ijk[1]->id, a.ijk[2]->id);
	}

	fclose(file);

	printf("\n\tWrite data to file: %s\n\n", filename_);

	return 0;
}
