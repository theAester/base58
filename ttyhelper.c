#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static struct termios old;

int input_is_tty(FILE *input) { return isatty(fileno(input)); }

int output_is_tty(FILE *output) { return isatty(fileno(output)); }

int stdin_enable_raw_mode(void) {
  if (!input_is_tty(stdin))
    return -1;
  struct termios raw;
  if (tcgetattr(STDIN_FILENO, &old) < 0)
    return -1;
  raw = old;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  return tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

int stdin_disable_raw_mode(void) {
  if (!input_is_tty(stdin))
    return -1;
  return tcsetattr(STDIN_FILENO, TCSANOW, &old);
}
