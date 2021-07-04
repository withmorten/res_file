#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#endif

#include "main.h"

#include "res_file.h"

#define USAGE \
"res_file <e/x> <drs> [target path]\n" \
"res_file <e/x> <rm> [target path] [source path]\n" \
"res_file <b/m> <rm> [source path] [target path] \n" \
"e/x extract, b/m build\n" \
"extracting can use rm or drs (if no rm present), building works with rm only\n"

int32 main(int32 argc, char **argv)
{
	RESFILE_Set_Missing_Flag(TRUE);

	if (argc < 3)
	{
		printf(USAGE);

		return 1;
	}

	switch (*argv[1])
	{
	case 'e':
	case 'x':
	{
		unixify_path(argv[2]);

		char *p = strrchr(argv[2], '.');

		if (!p)
		{
			printf("error: wrong extension for infile, must be .rm or .drs (or any extension)\n");

			return 1;
		}

		bool32 manifest = strequal(p, ".rm");

		if (!manifest)
		{
			char *drs_name = argv[2];
			char *target_path;

			if (!argv[3])
			{
				target_path = rge_calloc<char>(strlen(drs_name) + 1 + 1);
				strcpy(target_path, drs_name);

				p = strrchr(target_path, '.');
				*p++ = '/';
				*p = '\0';
			}
			else
			{
				unixify_path(argv[3]);
				size_t target_path_len = strlen(argv[3]);
				target_path = rge_calloc<char>(target_path_len + 1 + 1);
				strcpy(target_path, argv[3]);

				if (target_path[target_path_len - 1] != '/') target_path[target_path_len] = '/';
			}

			FILE *f = rge_fopen(drs_name, "rb");

			if (!f)
			{
				printf("error: couldn't open drs file: %s\n", drs_name);

				return 1;
			}

			rge_fclose(f);

			if (mkdir_p(target_path) == -1)
			{
				printf("error: couldn't create target path: %s\n", target_path);

				return 1;
			}

			RESFILE_open_new_resource_file(drs_name, RESFILE_PASSWORD);

			RESFILE_dump_all(drs_name, target_path);

			RESFILE_close_new_resource_file(drs_name);

			printf("RESFILE_dump_all done\n");

			rge_free(target_path);
		}
		else
		{
			char *build_list_file = argv[2];
			char *target_path;
			char *source_path;

			char drs_name[MAX_PATH];
			char res_name[MAX_PATH];
			char password[40];

			FILE *buildFile = rge_fopen(build_list_file, "r");

			if (!buildFile)
			{
				printf("error: couldn't open build list file: %s\n", build_list_file);

				return 1;
			}

			if (fscanf(buildFile, "%s %s", res_name, password) != 2)
			{
				printf("error: couldn't read from build list file: %s\n", build_list_file);

				return 1;
			}

#ifdef DRS_NAME_FROM_RM
			saprintf(drs_name, "%s.drs", res_name);
#else
			p = strchr(build_list_file, '/');

			if (p) p++;
			else p = build_list_file;

			stracpychr(drs_name, p, '.');
			stracat(drs_name, ".drs");

			stracpychr(res_name, p, '.');
#endif

			if (!argv[3])
			{
				size_t target_path_len = strlen(res_name);
				target_path = rge_calloc<char>(target_path_len + 1 + 1);
				strcpy(target_path, res_name);

				target_path[target_path_len] = '/';

				source_path = strdup("");
			}
			else
			{
				unixify_path(argv[3]);
				size_t target_path_len = strlen(argv[3]);
				target_path = rge_calloc<char>(target_path_len + 1 + 1);
				strcpy(target_path, argv[3]);

				if (target_path[target_path_len - 1] != '/') target_path[target_path_len] = '/';

				if (!argv[4])
				{
					source_path = strdup("");
				}
				else
				{
					unixify_path(argv[4]);
					size_t source_path_len = strlen(argv[4]);
					source_path = rge_calloc<char>(source_path_len + 1 + 1);
					strcpy(source_path, argv[4]);

					if (source_path[source_path_len - 1] != '/') source_path[source_path_len] = '/';
				}
			}

			char drs_path[MAX_PATH];
			saprintf(drs_path, "%s%s", source_path, drs_name);

			FILE *f = rge_fopen(drs_path, "rb");

			if (!f)
			{
				printf("error: couldn't open drs file: %s\n", drs_path);

				return 1;
			}

			rge_fclose(f);

			if (mkdir_p(target_path) == -1)
			{
				printf("error: couldn't create target path: %s\n", target_path);

				return 1;
			}

			char temp_filename[MAX_PATH];
			uint32 rId;

			RESFILE_open_new_resource_file(drs_name, RESFILE_PASSWORD, source_path, RESOURCE_MEMORY_ALLOCATED);

			while (fscanf(buildFile, "%s", temp_filename) == 1 && fscanf(buildFile, "%d", &rId) == 1)
			{
				char temp_filepath[MAX_PATH];
				saprintf(temp_filepath, "%s%s", target_path, temp_filename);

				RESFILE_Make_File(BUILDRES_get_files_resource_type(temp_filename), rId, temp_filepath);
			}

			RESFILE_close_new_resource_file(drs_name);

			printf("RESFILE_Make_File done\n");

			rge_fclose(buildFile);

			rge_free(target_path);
			rge_free(source_path);
		}

		break;
	}
	case 'b':
	case 'm':
	{
		unixify_path(argv[2]);
		char *build_list_file = argv[2];

		char *p = strrchr(build_list_file, '.');

		if (!p || !strequal(p, ".rm"))
		{
			printf("error: wrong extension for infile, must be .rm\n");

			return 1;
		}
		
		char *source_path;
		char *target_path;

		if (!argv[3])
		{
			source_path = strdup("");
			target_path = strdup("");
		}
		else
		{
			unixify_path(argv[3]);
			size_t source_path_len = strlen(argv[3]);
			source_path = rge_calloc<char>(source_path_len + 1 + 1);
			strcpy(source_path, argv[3]);

			if (source_path[source_path_len - 1] != '/') source_path[source_path_len] = '/';

			if (!argv[4])
			{
				target_path = strdup("");
			}
			else
			{
				unixify_path(argv[4]);
				size_t target_path_len = strlen(argv[4]);
				target_path = rge_calloc<char>(target_path_len + 1 + 1);
				strcpy(target_path, argv[4]);

				if (target_path[target_path_len - 1] != '/') target_path[target_path_len] = '/';
			}
		}

		if (*target_path && mkdir_p(target_path) == -1)
		{
			printf("error: couldn't create target path: %s\n", target_path);

			return 1;
		}

		if (RESFILE_build_res_file(build_list_file, source_path, target_path))
		{
			printf("RESFILE_build_res_file done\n");
		}
		else
		{
			printf("error: RESFILE_build_res_file failed\n");

			return 1;
		}

		rge_free(target_path);
		rge_free(source_path);
	}
	}

	return 0;
}
