#ifndef __TTYHELPER_H_
#define __TTYHELPER_H_

int input_is_tty(FILE *input);
int output_is_tty(FILE *output);
int stdin_enable_raw_mode();
int stdin_disable_raw_mode();

#endif
