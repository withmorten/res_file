#include <Windows.h>
#include <io.h>

#include "res_file.h"

struct res_file_type_info
{
	char extension[4];
	uint32 rType;
};

struct BuildRes_id_node
{
	uint32 id;
	int32 dataOffset;
	byte *resData;
	int32 resSize;
	BuildRes_id_node *next;
	char fName[MAX_PATH];
};

struct BuildRes_type_node
{
	uint32 type;
	int32 idOffset;
	int32 nIds;
	BuildRes_id_node *idList;
	BuildRes_type_node *next;
};

struct resfile_id_dir_node
{
	uint32 id;
	int32 itemOffset;
	int32 itemSize;
};

struct resfile_type_dir_node
{
	uint32 type;
	int32 dirOffset;
	int32 numID;
};

struct resfile_header
{
	char banner_msg[40];
	char version[4];
	char password[12];
	int32 num_res_types;
	int32 directory_size;
};

struct ResFileHdr
{
	byte *mapped_file;
	rge_handle handle;
	resfile_header *header;
	ResFileHdr *next;
	char res_name[MAX_PATH];
};

local bool32 resource_missing_flag = FALSE;

void RESFILE_Set_Missing_Flag(bool32 FlagVal)
{
	resource_missing_flag = FlagVal;
}

local res_file_type_info res_extension_table[] =
{
	{ "voc", RESFILE_TYPE_VOC },
	{ "wav", RESFILE_TYPE_WAV },
	{ "xmi", RESFILE_TYPE_XMI },
	{ "lbm", RESFILE_TYPE_LBM },
	{ "pcx", RESFILE_TYPE_PCX },
	{ "gif", RESFILE_TYPE_GIF },
	{ "shp", RESFILE_TYPE_SHP },
	{ "slp", RESFILE_TYPE_SLP },
	{ "cel", RESFILE_TYPE_CEL },
	{ "flc", RESFILE_TYPE_FLC },
	{ "fli", RESFILE_TYPE_FLI },
	{ "ccs", RESFILE_TYPE_CCS },
	{ "fnt", RESFILE_TYPE_FNT },
	{ "rgb", RESFILE_TYPE_RGB },
	{ "xxx", RESFILE_TYPE_XXX },
};

local char header_message[40] = "Copyright (c) 1997 Ensemble Studios.\x1A";

uint32 BUILDRES_get_files_resource_type(char *filename)
{
	// the code here originally is pretty weird and overcomplicated
	// this does the same, but is a lot more concise ...

	char *extension = strrchr(filename, '.');

	if (!extension) return RESFILE_TYPE_BIN;

	extension++;

	size_t extension_len = strlen(extension) + 1;

	if (extension_len != sizeof(res_extension_table->extension)) return RESFILE_TYPE_BIN;

	for (size_t i = 0; i < sizeof(res_extension_table) / sizeof(res_file_type_info); i++)
	{
		if (!stricmp(extension, res_extension_table[i].extension))
		{
			return res_extension_table[i].rType;
		}
	}

	return RESFILE_TYPE_BIN;
}

bool32 RESFILE_build_res_file(char *build_list_file, char *source_path, char *target_path)
{
	// this isn't exactly done as originally - originally all the files were in the same folder "resource",
	// and the resource filename in the .rm file included the extension
	// for easier handling of making them that's implemented differently here

	char rPassword[40] = ZEROSTR;
	char data_filename[MAX_PATH];
	char resource_basename[MAX_PATH];
	char temp_filename[MAX_PATH];
	char resource_filename[MAX_PATH];
	char build_filename[MAX_PATH];
	uint32 rId, rType;
	BuildRes_type_node *ID_List;
	int32 numResTypes;

	snprintf(build_filename, sizeof(build_filename), "%s%s", source_path, build_list_file);

	FILE *buildFile = rge_fopen(build_filename, "r");

	if (!buildFile)
	{
		printf("Error: could not find resource build file: %s\n", build_list_file);

		return FALSE;
	}

	fscanf(buildFile, "%s %s", resource_basename, rPassword);

	snprintf(resource_filename, sizeof(resource_filename), "%s%s.drs", target_path, resource_basename);

	ID_List = NULL;
	numResTypes = 0;

	while (fscanf(buildFile, "%s", temp_filename) != EOF && fscanf(buildFile, "%d", &rId) != EOF)
	{
		snprintf(data_filename, sizeof(data_filename), "%s%s\\%s", source_path, resource_basename, temp_filename);

		rType = BUILDRES_get_files_resource_type(temp_filename);

		if (rType == RESFILE_TYPE_XXX)
		{
			printf("Error: could not determine file type of: %s\n", temp_filename);

			return FALSE;
		}

		BuildRes_type_node *p, *n, *Type_Node;

		Type_Node = NULL;
		p = NULL;
		n = ID_List;

		while (n)
		{
			if (n->type == rType)
			{
				Type_Node = n;

				break;
			}

			if (n->type < rType)
			{
				p = n;
			}

			n = n->next;
		}

		if (!Type_Node)
		{
			Type_Node = rge_new(BuildRes_type_node());

			Type_Node->type = rType;
			Type_Node->idOffset = 0;
			Type_Node->nIds = 0;
			Type_Node->idList = NULL;
			Type_Node->next = NULL;

			if (p)
			{
				Type_Node->next = p->next;
				p->next = Type_Node;
			}
			else
			{
				Type_Node->next = ID_List;
				ID_List = Type_Node;
			}

			numResTypes++;
		}

		BuildRes_id_node *in, *iq, *ID_Node;

		in = Type_Node->idList;
		iq = NULL;

		while (in)
		{
			if (in->id == rId)
			{
				printf("Error: duplicate resources: %s & %s\n", in->fName, data_filename);

				return FALSE;
			}

			if (in->id < rId)
			{
				iq = in;
			}

			in = in->next;
		}

		ID_Node = rge_new(BuildRes_id_node());

		ID_Node->id = rId;
		ID_Node->dataOffset = 0;
		ID_Node->resData = NULL;
		ID_Node->resSize = 0;
		ID_Node->next = NULL;

		strncpy_s(ID_Node->fName, data_filename, sizeof(data_filename));

		rge_handle dataHandle = _open(data_filename, DEFAULT_READ_FLAGS);

		if (dataHandle == INVALID_FILE_HANDLE)
		{
			printf("Error: unable to open file: %s\n", data_filename);

			rge_delete(ID_Node);

			continue;
		}

		if (iq)
		{
			ID_Node->next = iq->next;
			iq->next = ID_Node;
		}
		else
		{
			ID_Node->next = Type_Node->idList;
			Type_Node->idList = ID_Node;
		}

		Type_Node->nIds++;

		ID_Node->resSize = _lseek(dataHandle, 0, SEEK_END);
		_lseek(dataHandle, 0, SEEK_SET);

		if (ID_Node->resSize <= 0)
		{
			printf("Error: file is empty/NULL: %s\n", data_filename);

			return FALSE;
		}

		ID_Node->resData = (byte *)malloc(ID_Node->resSize);

		if (_read(dataHandle, ID_Node->resData, ID_Node->resSize) != ID_Node->resSize)
		{
			printf("Error: error reading file: %s\n", data_filename);

			return FALSE;
		}

		_close(dataHandle);
	}

	rge_fclose(buildFile);

	rge_handle resHandle = _open(resource_filename, DEFAULT_WRITE_FLAGS, DEFAULT_WRITE_PMODE);

	if (resHandle == INVALID_FILE_HANDLE)
	{
		printf("Error: unable to create resource file file: %s\n", resource_filename);

		return FALSE;
	}

	resfile_header theHeader = ZEROMEM;

	strncpy_s(theHeader.banner_msg, header_message, sizeof(header_message));
	memcpy(theHeader.version, RESFILE_VERSION, sizeof(theHeader.version));
	strncpy_s(theHeader.password, rPassword, sizeof(rPassword));

	int32 offset = sizeof(resfile_header) + numResTypes * sizeof(resfile_type_dir_node);

	for (BuildRes_type_node *n = ID_List; n; n = n->next)
	{
		n->idOffset = offset;

		offset += n->nIds * sizeof(resfile_id_dir_node);
	}

	theHeader.num_res_types = numResTypes;
	theHeader.directory_size = offset;

	for (BuildRes_type_node *n = ID_List; n; n = n->next)
	{
		for (BuildRes_id_node *i = n->idList; i; i = i->next)
		{
			i->dataOffset = offset;

			offset += i->resSize;
		}
	}

	if (_write(resHandle, &theHeader, sizeof(theHeader)) != sizeof(theHeader))
	{
		printf("Error writing resource file header\n");

		return FALSE;
	}

	resfile_type_dir_node typeDirNode;
	resfile_id_dir_node idDirNode;

	offset = sizeof(resfile_header);

	for (BuildRes_type_node *n = ID_List; n; n = n->next)
	{
		typeDirNode.type = n->type;
		typeDirNode.dirOffset = n->idOffset;
		typeDirNode.numID = n->nIds;

		if (_write(resHandle, &typeDirNode, sizeof(typeDirNode)) != sizeof(typeDirNode))
		{
			printf("Error writing resource file header: type node\n");

			return FALSE;
		}

		offset += sizeof(typeDirNode);
	}

	for (BuildRes_type_node *n = ID_List; n; n = n->next)
	{
		if (offset != n->idOffset)
		{
			printf("Error writing resource file: pos out of sync 1\n");

			return FALSE;
		}

		for (BuildRes_id_node *i = n->idList; i; i = i->next)
		{
			idDirNode.id = i->id;
			idDirNode.itemOffset = i->dataOffset;
			idDirNode.itemSize = i->resSize;

			if (_write(resHandle, &idDirNode, sizeof(idDirNode)) != sizeof(idDirNode))
			{
				printf("Error writing resource file header: id node\n");

				return FALSE;
			}

			offset += sizeof(idDirNode);
		}
	}

	for (BuildRes_type_node *n = ID_List; n; n = n->next)
	{
		for (BuildRes_id_node *i = n->idList; i; i = i->next)
		{
			if (offset != i->dataOffset)
			{
				printf("Error writing resource file: pos out of sync 2\n");

				return FALSE;
			}

			if (_write(resHandle, i->resData, i->resSize) != i->resSize)
			{
				printf("Error writing resource file data\n");

				return FALSE;
			}

			offset += i->resSize;
		}
	}

	_close(resHandle);

	BuildRes_type_node *n = ID_List;

	while (n)
	{
		BuildRes_id_node *i = n->idList;

		while (i)
		{
			BuildRes_id_node *tmp = i;
			i = i->next;

			rge_free(tmp->resData);
			rge_delete(tmp);
		}

		BuildRes_type_node *tmp = n;
		n = n->next;

		rge_delete(tmp);
	}

	return TRUE;
}

local ResFileHdr *Res_Files = NULL;

void RESFILE_open_new_resource_file(char *resFileName, char *password, char *path, int32 open_mode)
{
	resfile_header rHeader;

	byte *mapped_file_data = NULL;
	rge_handle fHandle = INVALID_FILE_HANDLE;

	char resFile[MAX_PATH];

	snprintf(resFile, sizeof(resFile), "%s%s", path, resFileName);

	if (open_mode == RESOURCE_MEMORY_MAPPED)
	{
		HANDLE mHandle = CreateFile(resFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (mHandle == INVALID_HANDLE_VALUE)
		{
			printf("Error: Open_new_ResFile, mapped file %s not Found.\n", resFileName);

			return;
		}

		HANDLE fMapped = CreateFileMapping(mHandle, NULL, PAGE_READONLY, 0, 0, NULL);
		CloseHandle(mHandle);

		if (!fMapped)
		{
			printf("Error: Open_Mapped_ResFile, Could not map file(1).\n");

			return;
		}

		mapped_file_data = (byte *)MapViewOfFile(fMapped, FILE_MAP_READ, 0, 0, 0);

		if (!mapped_file_data)
		{
			printf("Error: Open_Mapped_ResFile, Could not map file to addr.\n");

			return;
		}

		CloseHandle(fMapped);
	}
	else
	{
		fHandle = _open(resFile, DEFAULT_READ_FLAGS);

		if (fHandle == INVALID_FILE_HANDLE)
		{
			printf("Error: Open_new_ResFile, file %s not Found.\n", resFileName);

			return;
		}
	}

	ResFileHdr *New_Res_File = (ResFileHdr *)malloc(sizeof(ResFileHdr));

	strncpy_s(New_Res_File->res_name, resFileName, sizeof(New_Res_File->res_name));

	New_Res_File->next = NULL;

	if (open_mode == RESOURCE_MEMORY_MAPPED)
	{
		New_Res_File->handle = INVALID_FILE_HANDLE;
		New_Res_File->mapped_file = mapped_file_data;
		New_Res_File->header = (resfile_header *)mapped_file_data;
	}
	else
	{
		New_Res_File->handle = fHandle;
		New_Res_File->mapped_file = NULL;

		_lseek(fHandle, 0, SEEK_CUR);

		if (_read(fHandle, &rHeader, sizeof(rHeader)) != sizeof(rHeader))
		{
			printf("Error: Reading resfile header.\n");

			return;
		}

		New_Res_File->header = (resfile_header *)malloc(rHeader.directory_size);

		_lseek(fHandle, 0, SEEK_SET);

		if (_read(fHandle, New_Res_File->header, rHeader.directory_size) != rHeader.directory_size)
		{

			printf("Error: Reading resfile header data.\n");

			return;
		}
	}

	if (!Res_Files)
	{
		Res_Files = New_Res_File;
	}
	else
	{
		ResFileHdr *Res_File = Res_Files;

		while (Res_File->next) Res_File = Res_File->next;

		Res_File->next = New_Res_File;
	}

	if (strcmp(New_Res_File->header->password, password))
	{
		printf("Error: Open_ResFile, Corruption detected in resfile.\n");
	}
	else
	{
		if (strncmp(New_Res_File->header->version, RESFILE_VERSION, sizeof(New_Res_File->header->version)))
		{
			printf("Error: Open_ResFile, Resfile not correct.\n");
		}
	}
}

void RESFILE_close_new_resource_file(char *res_file)
{
	ResFileHdr *Res_File = Res_Files;
	ResFileHdr *Save_Res_File = NULL;

	if (Res_Files)
	{
		while (strcmp(res_file, Res_File->res_name))
		{
			Save_Res_File = Res_File;
			Res_File = Res_File->next;

			if (!Res_File) return;
		}

		if (!Res_File->mapped_file) rge_free(Res_File->header);

		if (Save_Res_File)
		{
			Save_Res_File->next = Res_File->next;
		}
		else
		{
			Res_Files = Res_File->next;
		}

		if (Res_File->handle != INVALID_FILE_HANDLE) _close(Res_File->handle);

		if (Res_File->mapped_file) UnmapViewOfFile(Res_File->mapped_file);

		rge_free(Res_File);
	}
}

byte *RESFILE_load(uint32 rType, uint32 rId, int32 &rLoadType, int32 &rDataSize)
{
	rge_handle infile;
	int32 offset, datalength;
	byte *mapped_file;

	rLoadType = RESOURCE_MEMORY_INVALID;
	rDataSize = 0;

	bool32 located = RESFILE_locate_resource(rType, rId, infile, offset, mapped_file, datalength);

	if (!located)
	{
		if (resource_missing_flag)
		{
			printf("ERROR: res_read_bin, resource type %08X , id %d, not found\n", rType, rId);
		}

		return NULL;
	}

	rDataSize = datalength;

	if (mapped_file)
	{
		rLoadType = RESOURCE_MEMORY_MAPPED;

		return mapped_file + offset;
	}

	rLoadType = RESOURCE_MEMORY_ALLOCATED;

	byte *file = (byte *)malloc(datalength);

	_lseek(infile, offset, SEEK_SET);

	int32 read = _read(infile, file, datalength);

	if (read != datalength)
	{
		rge_free(file);

		printf("Error: unable to read resource\n");
	}

	return file;
}

bool32 RESFILE_Decommit_Mapped_Memory(byte *ResData, int32 resSize)
{
	if (ResData && resSize > 0) return VirtualFree(ResData, resSize, MEM_DECOMMIT);

	return FALSE;
}

bool32 RESFILE_locate_resource(uint32 rType, uint32 rId, rge_handle &file, int32 &offset, byte *&mapped_file, int32 &size)
{
	file = INVALID_FILE_HANDLE;
	offset = 0;
	mapped_file = NULL;
	size = 0;

	if (!Res_Files) return FALSE;

	ResFileHdr *p = Res_Files;

	while (p)
	{
		resfile_header *header = p->header;
		resfile_type_dir_node *type_nodes = (resfile_type_dir_node *)&header[1];
byte *resfile = (byte *)header;

for (int32 x = 0; x < header->num_res_types; x++)
{
	resfile_type_dir_node *type_node = &type_nodes[x];

	if (type_node->type == rType)
	{
		resfile_id_dir_node *id_nodes = (resfile_id_dir_node *)&resfile[type_node->dirOffset];

		for (int32 y = 0; y < type_node->numID; y++)
		{
			resfile_id_dir_node *id_node = &id_nodes[y];

			if (id_node->id == rId)
			{
				file = p->handle;
				offset = id_node->itemOffset;
				mapped_file = p->mapped_file;
				size = id_node->itemSize;

				return TRUE;
			}
		}
	}
}

p = p->next;
	}

	return FALSE;
}

bool32 RESFILE_Extract_to_File(uint32 rType, uint32 rId, char *file_name, FILE *&file)
{
	int32 rLoadType, rDataSize;

	file = NULL;

	byte *data = RESFILE_load(rType, rId, rLoadType, rDataSize);

	if (data)
	{
		char temp_path[MAX_PATH] = ZEROSTR;

		if (!GetTempPath(sizeof(temp_path), temp_path))
		{
			*temp_path = '.';
		}

		if (GetTempFileName(temp_path, "temp", 0, file_name))
		{
			file = rge_fopen(file_name, "wb+");

			if (file)
			{
				fwrite(data, rDataSize, 1, file);
				fseek(file, 0, SEEK_SET);
			}
		}

		if (rLoadType == RESOURCE_MEMORY_ALLOCATED)
		{
			rge_free(data);
		}
	}

	return file != NULL;
}

bool32 RESFILE_Make_File(uint32 rType, uint32 rId, char *fileName)
{
	int32 rLoadType, rDataSize;

	byte *data = RESFILE_load(rType, rId, rLoadType, rDataSize);

	if (data)
	{
		FILE *file = rge_fopen(fileName, "wb+");

		if (file)
		{
			fwrite(data, rDataSize, 1, file);
			fseek(file, 0, SEEK_SET);

			rge_fclose(file);
		}

		if (rLoadType == RESOURCE_MEMORY_ALLOCATED)
		{
			free(data);
		}
	}

	return data != NULL;
}

char *RESFILE_get_extension(uint32 rType)
{
	for (size_t i = 0; i < sizeof(res_extension_table) / sizeof(res_file_type_info); i++)
	{
		if (res_extension_table[i].rType == rType)
		{
			return res_extension_table[i].extension;
		}
	}

	return "bin";
}

void RESFILE_dump_all(char *res_file, char *target_path)
{
	ResFileHdr *Res_File = Res_Files;

	if (Res_Files)
	{
		while (strcmp(res_file, Res_File->res_name))
		{
			Res_File = Res_File->next;

			if (!Res_File) return;
		}

		resfile_header *header = Res_File->header;
		resfile_type_dir_node *type_nodes = (resfile_type_dir_node *)&header[1];
		byte *resfile = (byte *)header;

		for (int32 x = 0; x < header->num_res_types; x++)
		{
			resfile_type_dir_node *type_node = &type_nodes[x];

			resfile_id_dir_node *id_nodes = (resfile_id_dir_node *)&resfile[type_node->dirOffset];

			for (int32 y = 0; y < type_node->numID; y++)
			{
				resfile_id_dir_node *id_node = &id_nodes[y];
				char filename[MAX_PATH];

				snprintf(filename, sizeof(filename), "%s%05d.%s", target_path, id_node->id, RESFILE_get_extension(type_node->type));

				RESFILE_Make_File(type_node->type, id_node->id, filename);
			}
		}
	}
}
