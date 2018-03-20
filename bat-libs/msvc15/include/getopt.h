#ifndef GETOPT_H
#define GETOPT_H

#ifdef __cplusplus
extern "C" {
#endif

int getopt(int nargc, char * const nargv[], const char *ostr);
extern char *optarg;

#ifdef __cplusplus
}
#endif

#endif
