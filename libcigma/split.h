#ifndef __UTIL_SPLIT_H__
#define __UTIL_SPLIT_H__
#ifdef __cplusplus
extern "C" {
#endif

void split(const char *str, int len,
           char ***split_list, int *split_count,
           char sep);

void split_free(char **split_list, int split_count);

#ifdef __cplusplus
}
#endif
#endif
