char *unixify_path(char *path);
int mkdir_p(const char *path);

ssize_t strscpy(char *dest, const char *src, size_t count);
ssize_t strsncpy(char *dest, const char *src, size_t count);
char *strscat(char *dest, const char *src, size_t dest_size);

ssize_t strscpychr(char *dest, const char *src, char end, size_t count);

template<size_t count> int saprintf(char (&string)[count], const char *format, ...)
{
	int ret;
	va_list args;
	va_start(args, format);
	ret = vsnprintf(string, count, format, args);
	va_end(args);
	return ret;
}

template<size_t count> forceinline ssize_t stracpy(char (&dest)[count], const char *src)
{
	return strscpy(dest, src, count);
}

template<size_t count> forceinline char *stracat(char (&dest)[count], const char *src)
{
	return strscat(dest, src, count);
}

template<size_t count> forceinline ssize_t stracpychr(char (&dest)[count], const char *src, char end)
{
	return strscpychr(dest, src, end, count);
}
