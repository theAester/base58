#ifndef __CMD_H_
#define __CMD_H_

#define PROGRAM_NAME "base58"
#define VERSION "0.1"

struct config {
  bool decode;
  bool ignore_garbage;
  bool check;
  bool check_version_present;
  uint32_t wrap;
  uint8_t check_version;
  const char *filename;
};

int parse_options(int argc, char **argv, struct config *config);

#endif
