#include <Windows.h>

#include "main.h"

#include "res_file.h"

#define USAGE \
"res_file <e/x>/<b/m> <rm/drs> [source path] [target path]\n" \
"e/x extract, b/m build\n" \
"extracting can use rm or drs (if no rm present), building works with rm only\n" \
"source and target path must exist (if given, default is current directory)\n" \
"only target path must be given when extracting a drs without rm file\n"

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
		char *build_list_file = argv[2];

		bool32 drs_mode = FALSE;

		char *p = strrchr(build_list_file, '.');

		if (p)
		{
			if (!stricmp(p, ".drs"))
			{
				drs_mode = TRUE;
			}
		}

		char *source_path = EMPTYSTR;
		char *target_path = EMPTYSTR;

		if (!drs_mode)
		{
			if (argc > 3)
			{
				source_path = (char *)rge_calloc(strlen(argv[3]) + 1 + 1, 1);
				strcpy(source_path, argv[3]);

				if (*source_path != '.' || (*source_path == '.' && source_path[1] == '..'))
				{
					size_t len = strlen(source_path);

					if (source_path[len - 1] != '/' && source_path[len - 1] != '\\')
					{
						source_path[len] = '/';
					}
				}
				else
				{
					*source_path = '\0';
				}
			}

			if (argc > 4)
			{
				target_path = (char *)rge_calloc(strlen(argv[4]) + 1 + 1, 1);
				strcpy(target_path, argv[4]);

				if (*target_path != '.' || (*target_path == '.' && target_path[1] == '..'))
				{
					size_t len = strlen(target_path);

					if (target_path[len - 1] != '/' && target_path[len - 1] != '\\')
					{
						target_path[len] = '/';
					}
				}
				else
				{
					*target_path = '\0';
				}
			}
		}
		else
		{
			if (argc > 3)
			{
				target_path = (char *)rge_calloc(strlen(argv[3]) + 1 + 1, 1);
				strcpy(target_path, argv[3]);

				if (*target_path != '.' || (*target_path == '.' && target_path[1] == '..'))
				{
					CreateDirectory(target_path, NULL);

					size_t len = strlen(target_path);

					if (target_path[len - 1] != '/' && target_path[len - 1] != '\\')
					{
						target_path[len] = '/';
					}
				}
				else
				{
					*target_path = '\0';
				}
			}
		}

		if (!drs_mode)
		{
			char drs_name[MAX_PATH];
			char password[40];
			char temp_filename[MAX_PATH];
			char temp_path[MAX_PATH];
			uint32 rId;

			FILE *buildFile = rge_fopen(build_list_file, "r");

			if (!buildFile)
			{
				printf("error: couldn't open build list file %s\n", build_list_file);

				return 1;
			}

			fscanf(buildFile, "%s %s", drs_name, password);

			snprintf(temp_path, sizeof(temp_path), "%s/%s", drs_name, target_path);
			CreateDirectory(temp_path, NULL);

			char *drs_file = (char *)rge_calloc(strlen(drs_name) + 4 + 1, 1);
			sprintf(drs_file, "%s%s", drs_name, ".drs");

			RESFILE_open_new_resource_file(drs_file, RESFILE_PASSWORD, source_path);

			while (fscanf(buildFile, "%s", temp_filename) != EOF && fscanf(buildFile, "%d", &rId) != EOF)
			{
				char temp_filepath[MAX_PATH];
				snprintf(temp_filepath, sizeof(temp_filepath), "%s/%s", temp_path, temp_filename);

				RESFILE_Make_File(BUILDRES_get_files_resource_type(temp_filename), rId, temp_filepath);
			}

			RESFILE_close_new_resource_file(drs_file);

			rge_free(drs_file);

			rge_fclose(buildFile);
		}
		else
		{
			char temp_path[MAX_PATH];
			char *temp = strdup(build_list_file);

			char *p = strrchr(temp, '.');

			if (!p)
			{
				printf("error: wrong extension for infile, must be .rm or .drs\n");

				return 1;
			}

			*p = '\0';
			snprintf(temp_path, sizeof(temp_path), "%s%s", target_path, temp);

			CreateDirectory(temp_path, NULL);

			*p++ = '/';
			*p = '\0';
			snprintf(temp_path, sizeof(temp_path), "%s%s", target_path, temp);

			RESFILE_open_new_resource_file(build_list_file, RESFILE_PASSWORD, EMPTYSTR);

			RESFILE_dump_all(build_list_file, temp_path);

			RESFILE_close_new_resource_file(build_list_file);

			rge_free(temp);
		}

		break;
	}
	case 'b':
	case 'm':
	{
		char *build_list_file = argv[2];
		
		char *source_path = EMPTYSTR;
		char *target_path = EMPTYSTR;

		if (argc > 3)
		{
			source_path = (char *)rge_calloc(strlen(argv[3]) + 1 + 1, 1);
			strcpy(source_path, argv[3]);

			if (*source_path != '.' || (*source_path == '.' && source_path[1] == '..'))
			{
				size_t len = strlen(source_path);

				if (source_path[len - 1] != '/' && source_path[len - 1] != '\\')
				{
					source_path[len] = '/';
				}
			}
			else
			{
				*source_path = '\0';
			}
		}

		if (argc > 4)
		{
			target_path = (char *)rge_calloc(strlen(argv[4]) + 1 + 1, 1);

			strcpy(target_path, argv[4]);

			if (*target_path != '.' || (*target_path == '.' && target_path[1] == '..'))
			{
				size_t len = strlen(target_path);

				if (target_path[len - 1] != '/' && target_path[len - 1] != '\\')
				{
					target_path[len] = '/';
				}
			}
			else
			{
				*target_path = '\0';
			}
		}

		RESFILE_build_res_file(build_list_file, source_path, target_path);

		break;
	}
	}

	return 0;
}
