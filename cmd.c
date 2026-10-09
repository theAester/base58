#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"

bool parse_uint32(const char *text, uint32_t *result) {
  char *end;
  unsigned long long value;

  if (text == NULL || *text == '\0' || *text == '-') {
    return false;
  }

  errno = 0;
  value = strtoull(text, &end, 10);

  if (errno != 0 || *end != '\0' || value > UINT32_MAX) {
    return false;
  }

  *result = (uint32_t)value;
  return true;
}

void print_usage(FILE *stream) {
  fprintf(
      stream,
      "Usage: base58 [OPTION]... [FILE]\n"
      "Base58 encode or decode FILE, or standard input, to standard output.\n"
      "\n"
      "With no FILE, or when FILE is -, read standard input.\n"
      "\n"
      "Mandatory arguments to long options are mandatory for short options "
      "too.\n"
      "  -d, --decode          decode data\n"
      "  -i, --ignore-garbage  when decoding, ignore non-alphabet characters\n"
      "  -w, --wrap=COLS       wrap encoded lines after COLS characters "
      "(default 76).\n"
      "                          Use 0 to disable line wrapping\n"
      "  -x, --check           use Base58Check instead of Base58\n"
      "  -V, --check-version[=BYTE]\n"
      "                        Base58Check version byte, 0-255 (default 0)\n"
      "\n"
      "      --help             display this help and exit\n"
      "      --version          output version information and exit\n");
}

/*
 * Returns 0 on success, 1 when the program should exit successfully
 * (e.g. --help), and -1 on an error.
 */
int parse_options(int argc, char **argv, struct config *config) {
  enum { OPT_HELP = 256, OPT_VERSION };

  static const struct option long_options[] = {
      {"decode", no_argument, NULL, 'd'},
      {"ignore-garbage", no_argument, NULL, 'i'},
      {"wrap", required_argument, NULL, 'w'},
      {"check", no_argument, NULL, 'x'},
      {"check-version", optional_argument, NULL, 'V'},
      {"help", no_argument, NULL, OPT_HELP},
      {"version", no_argument, NULL, OPT_VERSION},
      {NULL, 0, NULL, 0}};

  *config = (struct config){.wrap = 76,
                            .check_version = 0,
                            .filename = "-",
                            .check_version_present = false};

  int option;
  while ((option = getopt_long(argc, argv, "diw:xV::", long_options, NULL)) !=
         -1) {
    switch (option) {
    case 'd':
      config->decode = true;
      break;

    case 'i':
      config->ignore_garbage = true;
      break;

    case 'x':
      config->check = true;
      break;

    case 'w':
      if (!parse_uint32(optarg, &config->wrap)) {
        fprintf(stderr, "%s: invalid wrap width: %s\n", PROGRAM_NAME, optarg);
        return -1;
      }
      break;

    case 'V': {
      config->check_version_present = true;
      uint32_t version;

      if (optarg != NULL) {
        if (!parse_uint32(optarg, &version) || version > UINT8_MAX) {
          fprintf(stderr,
                  "%s: check version must be an integer from 0 to 255\n",
                  PROGRAM_NAME);
          return -1;
        }
        config->check_version = (uint8_t)version;
      }
      break;
    }

    case OPT_HELP:
      print_usage(stdout);
      return 1;

    case OPT_VERSION:
      printf("%s %s\n", PROGRAM_NAME, VERSION);
      return 1;

    default:
      print_usage(stderr);
      return -1;
    }
  }

  if (argc - optind > 1) {
    fprintf(stderr, "%s: too many input files\n", PROGRAM_NAME);
    print_usage(stderr);
    return -1;
  }

  if (optind < argc) {
    config->filename = argv[optind];
  }

  return 0;
}
