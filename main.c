#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base58.h"
#include "vector.h"

#define PROGRAM_NAME "base58"
#define VERSION "0.1"
#define MAX_INPUT_LEN (256 * 1024 * 1024) // 256MiB

struct config {
  bool decode;
  bool ignore_garbage;
  bool check;
  bool check_version_present;
  uint32_t wrap;
  uint8_t check_version;
  const char *filename;
};

static void print_usage(FILE *stream) {
  fprintf(
      stream,
      "Usage: bas58 [OPTION]... [FILE]\n"
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

static bool parse_uint32(const char *text, uint32_t *result) {
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

/*
 * Returns 0 on success, 1 when the program should exit successfully
 * (e.g. --help), and -1 on an error.
 */
static int parse_options(int argc, char **argv, struct config *config) {
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

int read_input(FILE *input, struct vector *vec, size_t max_len) {
#define B58RI_BUF_LEN 1024
  size_t len = 0;
  size_t count;
  uint8_t buf[B58RI_BUF_LEN];
  printf("in function\n");
  while ((count = fread(buf, 1, sizeof(buf), input)) > 0) {
    printf("TEST\n");
    len += count;
    if (len > max_len) {
      fprintf(stderr, "Input is too long\n");
      return 1;
    }
    if (vector_push(vec, buf, count))
      return 1;
  }
  if (ferror(input)) {
    fprintf(stderr, "Error while reading input: [%d]%s\n", errno,
            strerror(errno));
    return 1;
  }
  return 0;
#undef B58RI_BUF_LEN
}

void print_data(uint8_t *data, int64_t len) {
  for (int i = 0; i < len; i++) {
    printf("%c", (char)data[i]);
  }
  printf("\n");
}

char *hex_encode(uint8_t *buf) {
  char *str = (char *)malloc(11);
  str[0] = '0';
  str[1] = 'x';
  str[10] = '\0';
  const char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  for (int i = 0; i < 4; i++) {
    uint8_t d = buf[i];
    uint8_t u = d >> 4;
    uint8_t l = d & 0x0f;
    str[2 * i] = HEX[u];
    str[2 * i + 1] = HEX[l];
  }
  return str;
}

int main(int argc, char **argv) {
  struct config config;
  int result;
  result = parse_options(argc, argv, &config);

  if (result != 0) {
    return result > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  FILE *input = stdin;
  if (strcmp(config.filename, "-") != 0) {
    input = fopen(config.filename, "rb");
    if (input == NULL) {
      perror(config.filename);
      return EXIT_FAILURE;
    }
  }

  struct vector input_vec;
  init_vector(&input_vec);

  // if base58check and encoding: add version to the beginning
  if (!config.decode && config.check) {
    vector_push(&input_vec, &(config.check_version), 1);
  }

  result = read_input(input, &input_vec, MAX_INPUT_LEN);
  if (result != 0) {
    fprintf(stderr, "Aborting due to input error.\n");
    return EXIT_FAILURE;
  }

  // if base58check and encoding: append checksum to the end
  if (!config.decode && config.check) {
    /*
    uint8_t checksum[4];
    base58_checksum(&checksum, input_vec.buf, input_vec.len);
    vector_push(&input_vec, checksum, 4);
    */
  }

  uint8_t *output_buffer = NULL;
  int64_t output_size;
  if (config.decode) {
    output_size = base58_decode(&output_buffer, input_vec.buf, input_vec.len,
                                config.ignore_garbage);
  } else {
    output_size = base58_encode(&output_buffer, input_vec.buf, input_vec.len);
  }
  destroy_vector(&input_vec);

  if (config.decode && config.check) {
    /*
    if (output_size < 5) {
      fprintf(stderr, "Input cannot possibly be in base58check format: it is "
                      "less than 5 bytes long. Run again without -x.\n");
      return 1;
    }
    uint8_t in_version = output_buffer[0];
    if (config.check_version_present) {
      fprintf(stderr, "Version: %d [Expected %d]\n", in_version,
              config.check_version);
    } else {
      fprintf(stderr, "Version: %d\n", in_version);
    }
    uint8_t *data = output_buffer + 1;
    int64_t data_len = output_size - 5;
    if (data_len < 0) {
      fprintf(stderr, "PANIC: Unexpected error number 1\n");
      return 1;
    }
    uint8_t *checksum = output_buffer + (output_size - 4);
    char *checksum_string = hex_encode(checksum);
    fprintf(stderr, "Checksum: %s ", checksum_string);
    free(checksum_string);
    if (base58_checksum_validate(checksum, output_buffer, data_len + 1)) {
      fprintf(stderr, "[VALID]\n");
    } else {
      base58_checksum(checksum, output_buffer, data_len + 1);
      char *expected_string = hex_encode(checksum);
      fprintf(stderr, "[INVALID. Expected %s]\n", expected_string);
      free(expected_string);
    }
    fprintf(stderr, "\n");
    print_data(data, data_len);
    */
  } else {
    print_data(output_buffer, output_size);
  }

  if (output_buffer != NULL) {
    free(output_buffer);
  }
  if (input != stdin) {
    fclose(input);
  }

  return EXIT_SUCCESS;
}
