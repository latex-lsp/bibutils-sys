/*
 * ebi2xml.c
 * 
 * Copyright (c) Chris Putnam 2004-2020
 *
 * Program and source code released under the GPL version 2
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "bibutils.h"
#include "bibformats.h"
#include "tomods.h"
#include "bibprog.h"

char help1[] =  "Converts an EBI XML file into MODS XML\n\n";
char help2[] = "ebi_file";

const char progname[] = "ebi2xml";

int
main( int argc, char *argv[] )
{
	param p;
	ebiin_initparams( &p, progname );
	modsout_initparams( &p, progname );
	tomods_processargs( &argc, argv, &p, help1, help2 );
	bibprog( argc, argv, &p );
	bibl_freeparams( &p );
	return EXIT_SUCCESS;
}
