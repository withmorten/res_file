#include <Windows.h>
#include <direct.h>

#include "main.h"

char *unixify_path(char *path)
{
	for (char *p = path; *p; p++)
	{
		if (*p == '\\')
		{
			*p = '/';
		}
	}

	return path;
}

int mkdir_p(const char *path)
{
	// Adapted from https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
	// Adapted from https://stackoverflow.com/a/2336245/119527

	char _path[MAX_PATH];
	char *p;

	errno = 0;

	if (strlen(path) + 1 > MAX_PATH)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	// Copy string so its mutable
	strcpy(_path, path);

	// Iterate the string
	for (p = _path + 1; *p; p++)
	{
		if (*p == '/')
		{
			// Temporarily truncate
			*p = '\0';

			if (mkdir(_path) != 0)
			{
				// Don't throw error if it's a drive
				if (errno != EEXIST && (errno == EACCES && !strchr(_path, ':')))
				{
					return -1;
				}
			}

			*p = '/';
		}
	}

	if (mkdir(_path) != 0)
	{
		if (errno != EEXIST)
		{
			return -1;
		}
	}

	return 0;
}

ssize_t strscpy(char *dest, const char *src, size_t count)
{
	ssize_t res = 0;

	while (count)
	{
		char c = src[res];
		dest[res] = c;

		if (!c) return res;

		res++;
		count--;
	}

	if (res) dest[res - 1] = '\0';

	return -1;
}

ssize_t strsncpy(char *dest, const char *src, size_t count)
{
	ssize_t written = strscpy(dest, src, count);

	if (written < 0 || written == count - 1) return written;

	memzero(dest + written + 1, count - written - 1);

	return written;
}

char *strscat(char *dest, const char *src, size_t dest_size)
{
	if (!dest_size) return dest;

	size_t dest_content_len = strlen(dest);

	if ((dest_size - dest_content_len) <= 0) return dest;

	return strscpy(dest + dest_content_len, src, dest_size - dest_content_len) < 0 ? NULL : dest + dest_content_len;
}

ssize_t strscpychr(char *dest, const char *src, char end, size_t count)
{
	ssize_t res = 0;

	while (count)
	{
		char c = src[res];

		if (c == end)
		{
			dest[res] = '\0';
			return res;
		}

		dest[res] = c;

		if (!c) return res;

		res++;
		count--;
	}

	if (res) dest[res - 1] = '\0';

	return -1;
}
