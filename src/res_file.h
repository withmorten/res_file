#pragma once

#include "main.h"

#define RESFILE_PASSWORD "tribe"
#define RESFILE_VERSION "1.00"

#define RESOURCE_MEMORY_INVALID -1
#define RESOURCE_MEMORY_MAPPED 0
#define RESOURCE_MEMORY_ALLOCATED 1

#define RESFILE_TYPE_XXX 0
#define RESFILE_TYPE_VOC 'voc '
#define RESFILE_TYPE_WAV 'wav '
#define RESFILE_TYPE_XMI 'midi'
#define RESFILE_TYPE_LBM 'ilbm'
#define RESFILE_TYPE_PCX 'pcx '
#define RESFILE_TYPE_GIF 'gif '
#define RESFILE_TYPE_SHP 'shp '
#define RESFILE_TYPE_SLP 'slp '
#define RESFILE_TYPE_CEL 'cell'
#define RESFILE_TYPE_FLC 'cell'
#define RESFILE_TYPE_FLI 'cell'
#define RESFILE_TYPE_CCS 'ccsr'
#define RESFILE_TYPE_FNT 'font'
#define RESFILE_TYPE_RGB 'rgbp'
#define RESFILE_TYPE_BIN 'bina'

void RESFILE_Set_Missing_Flag(bool32 FlagVal);

uint32 BUILDRES_get_files_resource_type(char *filename);
bool32 RESFILE_build_res_file(char *build_list_file, char *source_path, char *target_path);

void RESFILE_open_new_resource_file(char *resFileName, char *password, char *path = "", int32 open_mode = RESOURCE_MEMORY_MAPPED);
void RESFILE_close_new_resource_file(char *res_file);

byte *RESFILE_load(uint32 rType, uint32 rId, int32 &rLoadType, int32 &rDataSize);
bool32 RESFILE_Decommit_Mapped_Memory(byte *ResData, int32 resSize);
bool32 RESFILE_locate_resource(uint32 rType, uint32 rId, rge_handle &file, int32 &offset, byte *&mapped_file, int32 &size);
bool32 RESFILE_Extract_to_File(uint32 rType, uint32 rId, char *file_name, FILE *&file);
bool32 RESFILE_Make_File(uint32 rType, uint32 rId, char *fileName);

void RESFILE_dump_all(char *res_file, char *target_path);
