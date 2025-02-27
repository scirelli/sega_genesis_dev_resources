//============================================================
//
//  fronthlp.c - Win32 frontend management functions
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <ctype.h>

// MAME headers
#include "driver.h"
#include "hash.h"
#include "info.h"
#include "audit.h"
#include "unzip.h"
#include "jedparse.h"
#include "sound/samples.h"
#include "options.h"


#define KNOWN_START 0
#define KNOWN_ALL   1
#define KNOWN_NONE  2
#define KNOWN_SOME  3


static FILE *verify_file;


static int knownstatus,identfiles,identmatches,identnonroms;



static void namecopy(char *name_ref,const char *desc)
{
	char name[200];

	strcpy(name,desc);

	/* remove details in parenthesis */
	if (strstr(name," (")) *strstr(name," (") = 0;

	/* Move leading "The" to the end */
	if (strncmp(name,"The ",4) == 0)
	{
		sprintf(name_ref,"%s, The",name+4);
	}
	else
		sprintf(name_ref,"%s",name);
}


/*-------------------------------------------------
    match_roms - scan for a matching ROM by hash
-------------------------------------------------*/

static void match_roms(const game_driver *driver, const char *hash, int length, int *found, FILE *output)
{
	const rom_entry *region, *rom;

	/* iterate over regions and files within the region */
	for (region = rom_first_region(driver); region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			if (hash_data_is_equal(hash, ROM_GETHASHDATA(rom), 0))
			{
				int baddump = hash_data_has_info(ROM_GETHASHDATA(rom), HASH_INFO_BAD_DUMP);

				if (output != NULL)
				{
					if (*found != 0)
						fprintf(output, "             ");
					fprintf(output, "= %s%-12s  %s\n", baddump ? "(BAD) " : "", ROM_GETNAME(rom), driver->description);
				}
				(*found)++;
			}
}


/*-------------------------------------------------
    identify_data - identify a buffer full of
    data; if it comes from a .JED file, parse the
    fusemap into raw data first
-------------------------------------------------*/

void identify_data(const char *name, const UINT8 *data, int length, FILE *output)
{
	int namelen = strlen(name);
	char hash[HASH_BUF_SIZE];
	UINT8 *tempjed = NULL;
	int found = 0;
	jed_data jed;
	int i;

	/* if this is a '.jed' file, process it into raw bits first */
	if (namelen > 4 && name[namelen - 4] == '.' &&
		tolower(name[namelen - 3]) == 'j' &&
		tolower(name[namelen - 2]) == 'e' &&
		tolower(name[namelen - 1]) == 'd' &&
		jed_parse(data, length, &jed) == JEDERR_NONE)
	{
		/* now determine the new data length and allocate temporary memory for it */
		length = jedbin_output(&jed, NULL, 0);
		tempjed = malloc(length);
		if (!tempjed)
			return;

		/* create a binary output of the JED data and use that instead */
		jedbin_output(&jed, tempjed, length);
		data = tempjed;
	}

	/* compute the hash of the data */
	hash_data_clear(hash);
	hash_compute(hash, data, length, HASH_SHA1 | HASH_CRC);

	/* remove directory portion of the name */
	for (i = namelen - 1; i > 0; i--)
		if (name[i] == '/' || name[i] == '\\')
		{
			i++;
			break;
		}

	/* output the name */
	identfiles++;
	if (output != NULL)
		fprintf(output, "%s ", &name[i]);

	/* see if we can find a match in the ROMs */
	for (i = 0; drivers[i]; i++)
		match_roms(drivers[i], hash, length, &found, output);

	/* if we didn't find it, try to guess what it might be */
	if (found == 0)
	{
		/* if not a power of 2, assume it is a non-ROM file */
		if ((length & (length - 1)) != 0)
		{
			if (output != NULL)
				fprintf(output, "NOT A ROM\n");
			identnonroms++;
		}

		/* otherwise, it's just not a match */
		else
		{
			if (output != NULL)
				fprintf(output, "NO MATCH\n");
			if (knownstatus == KNOWN_START)
				knownstatus = KNOWN_NONE;
			else if (knownstatus == KNOWN_ALL)
				knownstatus = KNOWN_SOME;
		}
	}

	/* if we did find it, count it as a match */
	else
	{
		identmatches++;
		if (knownstatus == KNOWN_START)
			knownstatus = KNOWN_ALL;
		else if (knownstatus == KNOWN_NONE)
			knownstatus = KNOWN_SOME;
	}

	/* free any temporary JED data */
	if (tempjed)
		free(tempjed);
}


/*-------------------------------------------------
    identify_file - identify a file; if it is a
    ZIP file, scan it and identify all enclosed
    files
-------------------------------------------------*/

void identify_file(const char *name, FILE *output)
{
	int namelen = strlen(name);
	int length;
	FILE *f;

	/* if the file has a 3-character extension, check it */
	if (namelen > 4 && name[namelen - 4] == '.' &&
		tolower(name[namelen - 3]) == 'z' &&
		tolower(name[namelen - 2]) == 'i' &&
		tolower(name[namelen - 1]) == 'p')
	{
		/* first attempt to examine it as a valid ZIP file */
		zip_file *zip = openzip(FILETYPE_RAW, 0, name);
		if (zip != NULL)
		{
			zip_entry *entry;

			/* loop over entries in the ZIP, skipping empty files and directories */
			for (entry = readzip(zip); entry; entry = readzip(zip))
				if (entry->uncompressed_size != 0)
				{
					UINT8 *data = (UINT8 *)malloc(entry->uncompressed_size);
					if (data != NULL)
					{
						readuncompresszip(zip, entry, data);
						identify_data(entry->name, data, entry->uncompressed_size, output);
						free(data);
					}
				}

			/* close up and exit early */
			closezip(zip);
			return;
		}
	}

	/* open the file directly */
	f = fopen(name, "rb");
	if (f)
	{
		/* determine the length of the file */
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);

		/* skip empty files */
		if (length != 0)
		{
			UINT8 *data = (UINT8 *)malloc(length);
			if (data != NULL)
			{
				fread(data, 1, length, f);
				identify_data(name, data, length, output);
				free(data);
			}
		}
		fclose(f);
	}
}


/*-------------------------------------------------
    identify_dir - scan a directory and identify
    all the files in it
-------------------------------------------------*/

void identify_dir(const char *dirname, FILE *output)
{
	WIN32_FIND_DATA entry;
	char *dirfilter;
	HANDLE dir;
	int more;

	/* create the search name */
	dirfilter = malloc(strlen(dirname) + 5);
	if (dirfilter == NULL)
		return;
	sprintf(dirfilter, "%s\\*", dirname);

	/* find the first file */
	memset(&entry, 0, sizeof(WIN32_FIND_DATA));
    dir = FindFirstFile(dirfilter, &entry);
    free(dirfilter);

	/* bail on failure */
    if (dir == INVALID_HANDLE_VALUE)
        return;

	/* loop until we run out of entries */
    do
    {
    	/* skip directories */
		if (!(entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			/* create a full path to the file and identify it */
			char *buf = (char *)malloc(strlen(dirname) + 1 + strlen(entry.cFileName) + 1);
			sprintf(buf, "%s\\%s", dirname, entry.cFileName);
			identify_file(buf, output);
			free(buf);
		}

		/* find the next entry */
		more = FindNextFileA(dir, &entry);
	} while (more);

	/* stop the search */
	FindClose(dir);
}


/*-------------------------------------------------
    romident - identify files
-------------------------------------------------*/

void romident(const char *name, FILE *output)
{
	TCHAR error[256];
	DWORD attr;

	/* reset the globals */
	knownstatus = KNOWN_START;
	identfiles = identmatches = identnonroms = 0;

	/* see what kind of file we're looking at */
	attr = GetFileAttributes(name);

	/* invalid -- nothing to see here */
	if (attr == INVALID_FILE_ATTRIBUTES)
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, error, ARRAY_LENGTH(error), NULL);
		_tprintf("%s: %s\n",name, error);
		return;
	}

	/* directory -- scan it */
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		identify_dir(name, output);
	else
		identify_file(name, output);
}


INLINE int isclone(int drvindex)
{
	const game_driver *clone_of = driver_get_clone(drivers[drvindex]);
	return (clone_of != NULL && (clone_of->flags & NOT_A_DRIVER) == 0);
}


int frontend_listxml(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	print_mame_xml(output, drivers, gamename);
	return 0;
}


int frontend_listfull(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	int drvindex, count = 0;

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	/* print the header */
	fprintf(output, "Name:     Description:\n");

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if ((drivers[drvindex]->flags & NOT_A_DRIVER) == 0 && mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			fprintf(output, "%-10s\"%s\"\n", drivers[drvindex]->name, drivers[drvindex]->description);
			count++;
		}

	/* return an error if none found */
	return (count > 0) ? 0 : 1;
}


int frontend_listsource(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	int drvindex, count = 0;

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0)
		{
			fprintf(output, "%-8s %s\n", drivers[drvindex]->name, drivers[drvindex]->source_file);
			count++;
		}

	/* return an error if none found */
	return (count > 0) ? 0 : 1;
}


int frontend_listclones(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	int drvindex, count = 0;

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	/* print the header */
	fprintf(output, "Name:    Clone of:\n");

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
	{
		const game_driver *clone_of = driver_get_clone(drivers[drvindex]);

		/* if we are a clone, and either our name matches the gamename, or the clone's name matches, display us */
		if (clone_of != NULL && (clone_of->flags & NOT_A_DRIVER) == 0)
			if (mame_strwildcmp(gamename, drivers[drvindex]->name) == 0 || mame_strwildcmp(gamename, clone_of->name) == 0)
			{
				fprintf(output, "%-8s %-8s\n", drivers[drvindex]->name, clone_of->name);
				count++;
			}
	}

	/* return an error if none found */
	return (count > 0) ? 0 : 1;
}


int frontend_listcrc(FILE *output)
{
	int drvindex;

	/* iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
	{
		const rom_entry *region, *rom;

		/* iterate over regions, and then ROMs within the region */
		for (region = rom_first_region(drivers[drvindex]); region; region = rom_next_region(region))
			for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
			{
				char hashbuf[512];

				/* if we have a CRC, display it */
				if (hash_data_extract_printable_checksum(ROM_GETHASHDATA(rom), HASH_CRC, hashbuf))
					fprintf(output, "%s %-12s %s\n", hashbuf, ROM_GETNAME(rom), drivers[drvindex]->description);
			}
	}

	return 0;
}


int frontend_listroms(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	const rom_entry *region, *rom, *chunk;
	const game_driver **gamedrv;

	/* find the gamename */
	for (gamedrv = (const game_driver **)&drivers[0]; *gamedrv != NULL; gamedrv++)
		if (mame_stricmp(gamename, (*gamedrv)->name) == 0)
			break;

	/* error if not found */
	if (*gamedrv == NULL)
	{
		fprintf(stderr, "Game \"%s\" not supported!\n", gamename);
		return 1;
	}

	/* print the header */
	fprintf(output, "This is the list of the ROMs required for driver \"%s\".\n"
			"Name            Size Checksum\n", gamename);

	/* iterate over regions and then ROMs within the region */
	for (region = (*gamedrv)->rom; region; region = rom_next_region(region))
		for (rom = rom_first_file(region); rom; rom = rom_next_file(rom))
		{
			const char *name = ROM_GETNAME(rom);
			const char* hash = ROM_GETHASHDATA(rom);
			char hashbuf[512];
			int length = -1; /* default is for disks! */

			/* accumulate the total length of all chunks */
			if (ROMREGION_ISROMDATA(region))
			{
				length = 0;
				for (chunk = rom_first_chunk(rom); chunk; chunk = rom_next_chunk(chunk))
					length += ROM_GETLENGTH(chunk);
			}

			/* start with the name */
			fprintf(output, "%-12s ", name);

			/* output the length next */
			if (length >= 0)
				fprintf(output, "%7d", length);
			else
				fprintf(output, "       ");

			/* output the hash data */
			if (!hash_data_has_info(hash, HASH_INFO_NO_DUMP))
			{
				if (hash_data_has_info(hash, HASH_INFO_BAD_DUMP))
					printf(" BAD");

				hash_data_print(hash, 0, hashbuf);
				fprintf(output, " %s", hashbuf);
			}
			else
				fprintf(output, " NO GOOD DUMP KNOWN");

			/* end with a CR */
			fprintf(output, "\n");
		}

	return 0;
}


int frontend_listsamples(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	const game_driver **gamedrv;

#if (HAS_SAMPLES)
	machine_config drv;
	int sndnum;
#endif

	/* find the gamename */
	for (gamedrv = (const game_driver **)&drivers[0]; *gamedrv != NULL; gamedrv++)
		if (mame_stricmp(gamename, (*gamedrv)->name) == 0)
			break;

	/* error if not found */
	if (*gamedrv == NULL)
	{
		fprintf(stderr, "Game \"%s\" not supported!\n", gamename);
		return 1;
	}

#if (HAS_SAMPLES)
	expand_machine_driver((*gamedrv)->drv, &drv);
	for (sndnum = 0; drv.sound[sndnum].sound_type && sndnum < MAX_SOUND; sndnum++)
	{
		const char **samplenames = NULL;

		/* if we hit a sample generator, grab the sample list */
		if (drv.sound[sndnum].sound_type == SOUND_SAMPLES)
			samplenames = ((struct Samplesinterface *)drv.sound[sndnum].config)->samplenames;

		/* if the list is legit, walk it and print the sample info */
		if (samplenames != NULL && samplenames[0] != NULL)
		{
			int sampnum;

			for (sampnum = 0; samplenames[sampnum] != NULL; sampnum++)
				fprintf(output, "%s\n", samplenames[sampnum]);
		}
	}
#endif

	return 0;
}


void CLIB_DECL verify_printf(const char *fmt, ...)
{
	va_list arg;

	/* dump to the buffer */
	va_start(arg, fmt);
	vfprintf(verify_file, fmt, arg);
	va_end(arg);
}



int frontend_verifyroms(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	int correct = 0;
	int incorrect = 0;
	int checked = 0;
	int notfound = 0;
	int total;
	int drvindex;

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	/* first count up how many drivers match the string */
	total = 0;
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (!mame_strwildcmp(gamename, drivers[drvindex]->name))
			total++;

	/* gross: stash the output handle in a global */
	verify_file = output;

	/* now iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
	{
		int res;

		/* skip if we don't match */
		if (mame_strwildcmp(gamename, drivers[drvindex]->name))
			continue;

		/* audit the ROMs in this set */
		res = audit_verify_roms(drvindex, verify_printf);

		/* if not found, count that and leave it at that */
		if (res == CLONE_NOTFOUND || res == NOTFOUND)
			notfound++;

		/* else display information about what we discovered */
		else
		{
			const game_driver *clone_of;

			/* output the name of the driver and its clone */
			fprintf(output, "romset %s ", drivers[drvindex]->name);
			clone_of = driver_get_clone(drivers[drvindex]);
			if (clone_of != NULL)
				fprintf(output, "[%s] ", clone_of->name);

			/* switch off of the result */
			switch (res)
			{
				case INCORRECT:
					fprintf(output, "is bad\n");
					incorrect++;
					break;

				case CORRECT:
					fprintf(output, "is good\n");
					correct++;
					break;

				case BEST_AVAILABLE:
					fprintf(output, "is best available\n");
					correct++;
					break;

				case MISSING_OPTIONAL:
					fprintf(output, "is missing optional files\n");
					correct++;
					break;
			}
		}

		/* update progress information on stderr */
		checked++;
		fprintf(stderr, "%d%%\r", 100 * checked / total);
	}

	/* if we didn't get anything at all, display a generic end message */
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			fprintf(output, "romset \"%8s\" not found!\n", gamename);
		else
			fprintf(output, "romset \"%8s\" not supported!\n", gamename);
		return 1;
	}

	/* otherwise, print a summary */
	else
	{
		fprintf(output, "%d romsets found, %d were OK.\n", correct + incorrect, correct);
		return (incorrect > 0) ? 2 : 0;
	}
}



int frontend_verifysamples(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);
	int correct = 0;
	int incorrect = 0;
	int checked = 0;
	int notfound = 0;
	int total;
	int drvindex;

	/* a NULL gamename == '*' */
	if (gamename == NULL)
		gamename = "*";

	/* first count up how many drivers match the string */
	total = 0;
	for (drvindex = 0; drivers[drvindex]; drvindex++)
		if (!mame_strwildcmp(gamename, drivers[drvindex]->name))
			total++;

	/* gross: stash the output handle in a global */
	verify_file = output;

	/* now iterate over drivers */
	for (drvindex = 0; drivers[drvindex]; drvindex++)
	{
		const char **samplenames = NULL;
		machine_config drv;
		int res;

#if (HAS_SAMPLES)
		int sndnum;
#endif

		/* skip if we don't match */
		if (mame_strwildcmp(gamename, drivers[drvindex]->name))
			continue;

		/* expand the machine driver and look for samples */
		expand_machine_driver(drivers[drvindex]->drv, &drv);
#if (HAS_SAMPLES)
		for (sndnum = 0; drv.sound[sndnum].sound_type && sndnum < MAX_SOUND; sndnum++)
			if (drv.sound[sndnum].sound_type == SOUND_SAMPLES)
				samplenames = ((struct Samplesinterface *)drv.sound[sndnum].config)->samplenames;
#endif
		/* if we have samples, continue */
		if (samplenames != NULL && samplenames[0] != NULL)
		{
			/* audit the samples in this set */
			res = audit_verify_samples(drvindex, verify_printf);

			/* if not found, count that and leave it at that */
			if (res == NOTFOUND)
				notfound++;

			/* else display information about what we discovered */
			else
			{
				fprintf(output, "sampleset %s ", drivers[drvindex]->name);

				/* switch off of the result */
				switch (res)
				{
					case INCORRECT:
						fprintf(output, "is bad\n");
						incorrect++;
						break;

					case CORRECT:
						fprintf(output, "is good\n");
						correct++;
						break;

					case BEST_AVAILABLE:
						fprintf(output, "is best available\n");
						correct++;
						break;

					case MISSING_OPTIONAL:
						fprintf(output, "is missing optional files\n");
						correct++;
						break;
				}
			}
		}

		/* update progress information on stderr */
		checked++;
		fprintf(stderr, "%d%%\r", 100 * checked / total);
	}

	/* if we didn't get anything at all, display a generic end message */
	if (correct + incorrect == 0)
	{
		if (notfound > 0)
			printf("sampleset \"%8s\" not found!\n", gamename);
		else
			printf("sampleset \"%8s\" not supported!\n", gamename);
		return 1;
	}

	/* otherwise, print a summary */
	else
	{
		printf("%d samplesets found, %d were OK.\n", correct + incorrect, correct);
		return (incorrect > 0) ? 2 : 0;
	}
}


int frontend_romident(FILE *output)
{
	/* do the identification */
	romident(options_get_string("", FALSE), output);

	/* return the appropriate error code */
	if (identmatches == identfiles)
		return 0;
	else if (identmatches == identfiles - identnonroms)
		return 1;
	else if (identmatches > 0)
		return 2;
	else
		return 3;
}


int frontend_isknown(FILE *output)
{
	const char *gamename = options_get_string("", FALSE);

	/* do the identification */
	romident(gamename, NULL);

	/* switch off of the result to print a summary */
	switch (knownstatus)
	{
		case KNOWN_START: fprintf(output, "ERROR     %s\n", gamename); break;
		case KNOWN_ALL:   fprintf(output, "KNOWN     %s\n", gamename); break;
		case KNOWN_NONE:  fprintf(output, "UNKNOWN   %s\n", gamename); break;
		case KNOWN_SOME:  fprintf(output, "PARTKNOWN %s\n", gamename); break;
	}

	/* return the appropriate error code */
	if (identmatches == identfiles)
		return 0;
	else if (identmatches == identfiles - identnonroms)
		return 1;
	else if (identmatches > 0)
		return 2;
	else
		return 3;
}

