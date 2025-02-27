//============================================================
//
//  vconv.c - VC++ parameter conversion code
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>



//============================================================
//  CONSTANTS
//============================================================

#define PRINT_COMMAND_LINE	0

#define	VS6		0x00060000
#define VS7		0x00070000
#define VS71	0x0007000a
#define VS2005	0x00080000



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef struct _translation_info translation_info;
struct _translation_info
{
	DWORD vc_version;
	const char *gcc_option;
	const char *vc_option;
};



//============================================================
//  TRANSLATION TABLES
//============================================================

static const translation_info gcc_translate[] =
{
	{ 0,		"-D*",					"/D*" },
	{ 0,		"-U*",					"/U*" },
	{ 0,		"-I*",					"/I*" },
	{ 0,		"-o*",					"~*" },
	{ 0,		"-include*",			"/FI*" },
	{ 0,		"-c",					"/c~/Fo" },
	{ 0,		"-E",					"/E" },
	{ 0,		"-S",					"/FA~/Fa" },
	{ VS7,		"-O0",					"/Od /GS" },
	{ 0,		"-O0",					"/Od" },
	{ 0,		"-O3",					"/O2" },
	{ 0,		"-g",					"/Zi" },
	{ VS2005,	"-fno-strict-aliasing",	"" },		// deprecated in VS2005
	{ 0,		"-fno-strict-aliasing",	"/Oa" },
	{ 0,		"-Werror",				"/WX" },
	{ VS2005,	"-Wall",				"/Wall /W3 /wd4018 /wd4146 /wd4242 /wd4244 /wd4305 /wd4619 /wd4702 /wd4706 /wd4710 /wd4711 /wd4738 /wd4826" },
	{ VS7,		"-Wall",				"/Wall /W3 /wd4018 /wd4146 /wd4242 /wd4244 /wd4305 /wd4550 /wd4619 /wd4702 /wd4706 /wd4710 /wd4711 /wd4826" },
	{ 0,		"-Wall",				"/W0" },
	{ VS7,		"-Wno-unused",			"/wd4100 /wd4101 /wd4102" },
	{ 0,		"-W*",					"" },
	{ VS2005,	"-march=*",				"" },		// deprecated in VS2005
	{ 0,		"-march=pentium",		"/G5" },
	{ 0,		"-march=athlon",		"/G7" },
	{ 0,		"-march=pentiumpro",	"/G6" },
	{ 0,		"-march=pentium4",		"/G7" },
	{ 0,		"-march=athlon64",		"/G7" },
	{ 0,		"-march=pentium3",		"/G6" },
	{ VS71,		"-msse2",				"/arch:SSE2" },
	{ 0,		"-msse2",				"" },
	{ 0,		"-mwindows",			"" },
	{ 0,		"-mno-cygwin",			"" },
	{ 0,		"-std=gnu89",			"" },
	{ 0 }
};

static const translation_info ld_translate[] =
{
	{ 0,		"-l*",				"*.lib" },
	{ 0,		"-o*",				"/out:*" },
	{ 0,		"-Wl,-Map,*",		"/map:*" },
 	{ 0,		"-Wl,--allow-multiple-definition", "/force:multiple" },
	{ 0,		"-mno-cygwin",		"" },
	{ 0,		"-s",				"" },
	{ 0,		"-WO",				"" },
	{ 0,		"-mconsole",		"" },
	{ 0,		"-mwindows",		"" },
	{ 0,		"-shared",			"/dll" },
	{ 0 }
};

static const translation_info ar_translate[] =
{
	{ 0,		"-cr",				"" },
	{ 0 }
};


static const translation_info windres_translate[] =
{
	{ 0,		"-D*",				"/D*" },
	{ 0,		"-U*",				"/U*" },
	{ 0,		"--include-dir*",	"/I*" },
	{ 0,		"-o*",				"/fo*" },
	{ 0,		"-O*",				"" },
	{ 0,		"-i*",				"*" },
	{ 0 }
};



//============================================================
//  GLOBALS
//============================================================

static char command_line[32768];



//============================================================
//  get_exe_version
//============================================================

static DWORD get_exe_version(const char *executable)
{
	char path[MAX_PATH];
	void *version_info;
	DWORD version_info_size, dummy;
	void *sub_buffer;
	UINT sub_buffer_size;
	VS_FIXEDFILEINFO *info;
	DWORD product_version;
	char sub_block[2] = { '\\', '\0' };

	// try to locate the executable
	if (!SearchPath(NULL, executable, NULL, sizeof(path) / sizeof(path[0]), path, NULL))
	{
		fprintf(stderr, "Cannot find %s\n", executable);
		exit(-100);
	}

	// determine the size of the version info
	version_info_size = GetFileVersionInfoSize(path, &dummy);
	if (version_info_size == 0)
	{
		switch(GetLastError())
		{
			case ERROR_RESOURCE_TYPE_NOT_FOUND:
				fprintf(stderr, "\"%s\" does not contain version info; this is probably not a MSVC executable\n", path);
				break;

			default:
				fprintf(stderr, "GetFileVersionInfoSize() failed\n");
				break;
		}
		exit(-100);
	}

	// allocate the memory; using GlobalAlloc() so that we do not
	// unintentionally uses malloc() overrides
	version_info = GlobalAlloc(GMEM_FIXED, version_info_size);
	if (!version_info)
	{
		fprintf(stderr, "Out of memory\n");
		exit(-100);
	}

	// retrieve the version info
	if (!GetFileVersionInfo(path, 0, version_info_size, version_info))
	{
		fprintf(stderr, "GetFileVersionInfo() failed\n");
		exit(-100);
	}

	// extract the VS_FIXEDFILEINFO from the version info
	if (!VerQueryValue(version_info, sub_block, &sub_buffer, &sub_buffer_size))
	{
		fprintf(stderr, "VerQueryValue() failed\n");
		exit(-100);
	}

	info = (VS_FIXEDFILEINFO *) sub_buffer;
	product_version = info->dwProductVersionMS;

	GlobalFree(version_info);
	return product_version;
}



//============================================================
//  build_command_line
//============================================================

static void build_command_line(int argc, char *argv[])
{
	const translation_info *transtable;
	const char *executable;
	const char *outstring = "";
	char *dst = command_line;
	int output_is_first = 0;
	int param;
	DWORD exe_version;

	// if no parameters, show usage
	if (argc < 2)
	{
		fprintf(stderr, "Usage:\n  vconv {gcc|ar|ld} [param [...]]\n");
		exit(0);
	}

	// first parameter determines the type
	if (!strcmp(argv[1], "gcc"))
	{
		transtable = gcc_translate;
		executable = "cl.exe";
		dst += sprintf(dst, "cl /nologo ");
	}
	else if (!strcmp(argv[1], "windres"))
	{
		transtable = windres_translate;
		executable = "rc.exe";
		dst += sprintf(dst, "rc ");
	}
	else if (!strcmp(argv[1], "ld"))
	{
		transtable = ld_translate;
		executable = "link.exe";
		dst += sprintf(dst, "link /nologo /debug ");
	}
	else if (!strcmp(argv[1], "ar"))
	{
		transtable = ar_translate;
		executable = "lib.exe";
		dst += sprintf(dst, "link /lib /nologo ");
		outstring = "/out:";
		output_is_first = 1;
	}
	else
	{
		fprintf(stderr, "Error: unknown translation type '%s'\n", argv[1]);
		exit(-100);
	}

	// identify the version number of the EXE
	exe_version = get_exe_version(executable);

	// special case
	if (!strcmp(executable, "cl.exe") && (exe_version >= 0x00070000))
		dst += sprintf(dst, "/wd4025 ");

	// iterate over parameters
	for (param = 2; param < argc; param++)
	{
		const char *src = argv[param];
		int srclen = strlen(src);
		int i;

		// find a match
		if (src[0] == '-')
		{
			for (i = 0; transtable[i].gcc_option; i++)
			{
				const char *compare = transtable[i].gcc_option;
				const char *replace;
				int j;

				// check version number
				if (exe_version < transtable[i].vc_version)
					continue;

				// find a match
				for (j = 0; j < srclen; j++)
					if (src[j] != compare[j])
						break;

				// if we hit an asterisk, we're ok
				if (compare[j] == '*')
				{
					// if this is the end of the parameter, use the next one
					if (src[j] == 0)
						src = argv[++param];
					else
						src += j;

					// copy the replacement up to the asterisk
					replace = transtable[i].vc_option;
					while (*replace && *replace != '*')
					{
						if (*replace == '~')
						{
							dst += sprintf(dst, "%s", outstring);
							replace++;
						}
						else
							*dst++ = *replace++;
					}

					// if we have an asterisk in the replacement, copy the rest of the source
					if (*replace == '*')
					{
						int addquote = (strchr(src, ' ') != NULL);

						if (addquote)
							*dst++ = '"';
						while (*src)
						{
							*dst++ = (*src == '/') ? '\\' : *src;
							src++;
						}
						if (addquote)
							*dst++ = '"';

						// if there's stuff after the asterisk, copy that
						replace++;
						while (*replace)
							*dst++ = *replace++;
					}

					// append a final space
					*dst++ = ' ';
					break;
				}

				// if we hit the end, we're also ok
				else if (compare[j] == 0 && j == srclen)
				{
					// copy the replacement up to the tilde
					replace = transtable[i].vc_option;
					while (*replace && *replace != '~')
						*dst++ = *replace++;

					// if we hit a tilde, set the new output
					if (*replace == '~')
						outstring = replace + 1;

					// append a final space
					*dst++ = ' ';
					break;
				}

				// else keep looking
			}

			// warn if we didn't get a match
			if (!transtable[i].gcc_option)
				fprintf(stderr, "Unable to match parameter '%s'\n", src);
		}

		// otherwise, assume it's a filename and copy translating slashes
		// it can also be a Windows-specific option which is passed through unscathed
		else
		{
			int dotrans = (*src != '/');

			// if the output filename is implicitly first, append the out parameter
			if (output_is_first)
			{
				dst += sprintf(dst, "%s", outstring);
				output_is_first = 0;
			}

			// now copy the rest of the string
			while (*src)
			{
				*dst++ = (dotrans && *src == '/') ? '\\' : *src;
				src++;
			}
			*dst++ = ' ';
		}
	}

	// trim remaining spaces and NULL terminate
	while (dst > command_line && dst[-1] == ' ')
		dst--;
	*dst = 0;
}



//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	DWORD exitcode;

	// build the new command line
	build_command_line(argc, argv);

	if (PRINT_COMMAND_LINE)
		printf("%s\n", command_line);

	// create the process information structures
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));

	// create and execute the process
	if (!CreateProcess(NULL, command_line, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) ||
		pi.hProcess == INVALID_HANDLE_VALUE)
		return -101;

	// block until done and fetch the error code
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &exitcode);

	// clean up the handles
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// return the child's error code
	return exitcode;
}
