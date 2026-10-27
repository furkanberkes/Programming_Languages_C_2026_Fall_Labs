
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef enum {
  TOKEN_NUM,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MULTIPLY,
  TOKEN_DIVIDE,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_ERROR,
  TOKEN_END,
  TOKEN_NEWLINE,
  TOKEN_POWER,
} tokenType;

typedef struct {
  tokenType type;
  long long value;  // only used for TOKEN_NUM
  long pos;         // position in the input string for error reporting
} Token;
// globals
const char* src;  // global pointer to the input string
long current_pos =
    1;  // global variable to track the current position in the input string
Token current_token;  // global variable to hold the current token
int is_line_start =
    1;  // global variable to track if we are at the start of a new line
// error handling vars
int found_error = 0;  // global variable to track if an error has been found
long error_pos = 0;   // global variable to track the position of the error
// parsing
long long
parse_expression();      // forward declaration of parse_expression function
long long parse_term();  // forward declaration of parse_term function
long long parse_primary();
long long parse_power();
int has_error() { return found_error; }

void set_error(long pos) {
  if (!found_error) {
    found_error = 1;
    error_pos = pos;
  }
}
// tokenizer
void next_token() {
  if (has_error()) {
    return;  // if an error has been found, do not proceed with tokenization
  }
  while (src[current_pos - 1] == ' ' || src[current_pos - 1] == '\t') {
    current_pos++;  // skip whitespace
  }
  char c = src[current_pos - 1];    // the current character
  current_token.pos = current_pos;  // set the position of the current token
  // endfile
  if (c == '\0') {
    current_token.type = TOKEN_END;
    return;
  }
  // comment
  if (c == '#') {
    if (is_line_start) {
      while (src[current_pos - 1] != '\n' && src[current_pos - 1] != '\0') {
        current_pos++;  // skip comment
      }
      next_token();  // get the next token after the comment
      return;
    } else {
      set_error(current_pos);
      current_token.type = TOKEN_ERROR;
      return;
    }
  }
  // newline
  if (c == '\n') {
    current_token.type = TOKEN_NEWLINE;
    current_pos++;  // move to the next character
    is_line_start =
        1;  // set the flag to indicate that we are at the start of a new line
    return;
  }
  is_line_start = 0;  // we are not at the start of a new line anymore
  if (isdigit(c)) {
    long long value = 0;
    while (isdigit(src[current_pos - 1])) {
      value = value * 10 + (src[current_pos - 1] - '0');
      current_pos++;  // move to the next character
    }
    current_token.type = TOKEN_NUM;
    current_token.value = value;  // set the value of the current token
    return;
  }
  // operators
  // operators
  switch (c) {
    case '+':
      current_token.type = TOKEN_PLUS;
      current_pos++;
      break;
    case '-':
      current_token.type = TOKEN_MINUS;
      current_pos++;
      break;
    case '/':
      current_token.type = TOKEN_DIVIDE;
      current_pos++;
      break;
    case '(':
      current_token.type = TOKEN_LPAREN;
      current_pos++;
      break;
    case ')':
      current_token.type = TOKEN_RPAREN;
      current_pos++;
      break;
    case '*':
      // Eger bir sonraki karakter de '*' ise bu bir us alma (**) islemidir
      if (src[current_pos] == '*') {
        current_token.type = TOKEN_POWER;
        current_pos +=
            2;  // Iki tane yildiz okudugumuz icin imleci 2 adim ilerlet
      } else {
        current_token.type = TOKEN_MULTIPLY;
        current_pos++;  // Tek yildizsa 1 adim ilerlet
      }
      break;
    default:
      set_error(current_pos);
      current_token.type = TOKEN_ERROR;
      break;
  }
}
// power
long long int_power(long long base, long long exp) {
  if (exp == 0) {
    return 1;  // base case: any number to the power of 0 is 1
  }
  long long result = 1;
  for (long long i = 0; i < exp; i++) {
    result *= base;  // multiply the base for exp times
  }
  return result;
}
long long parse_primary() {
  if (has_error()) {
    return 0;  // if an error has been found, do not proceed with parsing
  }
  if (current_token.type == TOKEN_NUM) {
    long long value = current_token.value;  // store the value of the number
    next_token();                           // move to the next token
    return value;                           // return the value of the number
  } else if (current_token.type == TOKEN_LPAREN) {
    long pos =
        current_token.pos;  // store the position of the '(' for error reporting
    next_token();           // move to the next token
    long long value =
        parse_expression();  // parse the expression inside the parentheses
    if (current_token.type == TOKEN_RPAREN) {
      next_token();  // move to the next token
      return value;  // return the value of the expression inside the
                     // parentheses
    } else {
      set_error(
          current_token
              .pos);  // set error at the position of '(' if ')' is not found
      return 0;       // return 0 as a placeholder value
    }
  }
  set_error(current_token.pos);  // set error at the position of the current
                                 // token if it's not a number or '('
  return 0;                      // return 0 as a placeholder value
}
long long parse_power() {
  if (has_error()) {
    return 0;  // if an error has been found, do not proceed with parsing
  }
  long long left = parse_primary();  // parse the left operand
  if (current_token.type ==
      TOKEN_POWER) {  // if the current token is a power operator
    next_token();     // move to the next token
    long right_pos = current_token.pos;  // store the position of the right
                                         // operand for error reporting
    long long right = parse_power();  // parse the right operand (recursive call
                                      // for right-associative)
    if (right < 0) {                  // check if the right operand is negative
      set_error(right_pos);  // set error at the position of the right operand
      return 0;              // return 0 as a placeholder value
    }
    return int_power(
        left, right);  // return the result of left raised to the power of
  }
  return left;  // return the left operand if there is no power operator
}
long long parse_term() {
  if (has_error()) {
    return 0;  // if an error has been found, do not proceed with parsing
  }
  long long left = parse_power();  // parse the left operand
  while (current_token.type == TOKEN_MULTIPLY ||
         current_token.type == TOKEN_DIVIDE) {
    tokenType op = current_token.type;   // store the operator
    next_token();                        // move to the next token
    long right_pos = current_token.pos;  // store the position of the right
                                         // operand for error reporting
    long long right = parse_power();
    if (has_error()) {
      return 0;  // if an error has been found, do not proceed with parsing
    }
    if (op == TOKEN_MULTIPLY) {
      left *= right;           // perform multiplication
    } else {                   // op == TOKEN_DIVIDE
      if (right == 0) {        // check for division by zero
        set_error(right_pos);  // set error at the position of the right operand
        return 0;              // return 0 as a placeholder value
      }
      left /= right;  // perform division
    }
  }
  return left;  // return the result of the term
}
long long parse_expression() {
  if (has_error()) {
    return 0;  // if an error has been found, do not proceed with parsing
  }
  long long left = parse_term();  // parse the left operand
  while (current_token.type == TOKEN_PLUS ||
         current_token.type == TOKEN_MINUS) {
    tokenType op = current_token.type;  // store the operator
    next_token();                       // move to the next token
    long long right = parse_term();     // parse the right operand
    if (has_error()) {
      break;  // if an error has been found, do not proceed with parsing
    }
    if (op == TOKEN_PLUS) {
      left += right;  // perform addition
    } else {          // op == TOKEN_MINUS
      left -= right;  // perform subtraction
    }
  }
  return left;  // return the result of the expression
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    return 1;  // if there are not enough arguments, return an error code
  }
  char* outdir =
      argv[2];  // get the output directory from the command line arguments
  char* filename = argv[3];  // get the filename from the command line arguments
  FILE* f = fopen(filename, "rb");  // open the input file for reading
  if (f == NULL) {
    fprintf(stderr, "Error opening file\n");  // print an error message if the
                                              // file cannot be opened
    return 1;                                 // return an error code
  }
  fseek(f, 0, SEEK_END);      // move the file pointer to the end of the file
  long file_size = ftell(f);  // get the size of the file
  rewind(f);  // rewind the file pointer to the beginning of the file
  char* t_buffer =
      (char*)malloc(file_size + 2);  // allocate memory for the file contents
  if (t_buffer == NULL) {
    fprintf(stderr, "Error allocating memory\n");  // print an error message if
                                                   // memory allocation fails
    fclose(f);                                     // close the input file
    return 1;                                      // return an error code
  }
  size_t bytes_read = fread(t_buffer, 1, file_size, f);
  (void)bytes_read;
  t_buffer[file_size] =
      '\n';  // add a newline character at the end of the buffer
  t_buffer[file_size + 1] =
      '\0';        // add a null terminator at the end of the buffer
  fclose(f);       // close the input file
  src = t_buffer;  // set the global source pointer to the buffer
  next_token();    // get the first token from the input
  long long* calc_result = (long long*)malloc(
      100000 *
      sizeof(long long));  // allocate memory for the calculation results
  int line_count = 0;      // initialize the line count
  while (current_token.type != TOKEN_END) {  // loop until the end of the input
    if (current_token.type ==
        TOKEN_NEWLINE) {  // if the current token is a newline
      next_token();       // move to the next token
      continue;           // skip to the next iteration of the loop
    }
    long long line_value =
        parse_expression();  // parse the expression and get the result
    if (has_error()) {       // check if an error was found during parsing
      break;                 // break out of the loop if an error was found
    }
    calc_result[line_count++] =
        line_value;  // store the result in the results array and
    next_token();    // move to the next token
  }
  mkdir(outdir, 0777);  // create the output directory with full permissions
  char out_path[512];   // buffer to hold the output file path
  const char* student_info =
      "_furkan_berke_seferoglu_260ADM048";  // student information string
  char* basename =
      strrchr(filename, '/');  // get the base name of the input file
  basename = basename ? basename + 1
                      : filename;  // if there is no '/', use the full filename
  char prefix[256] = {0};  // buffer to hold the prefix of the output file name
  strncpy(prefix, basename, strcspn(basename, "."));  // copy the
  sprintf(out_path, "%s/%s%s.txt", outdir, prefix,
          student_info);             // create the output file path
  FILE* out = fopen(out_path, "w");  // open the output file for writing
  if (found_error) {
    fprintf(out, "ERROR:%ld\n", error_pos);
  } else {
    for (int i = 0; i < line_count; i++) {
      fprintf(
          out, "%lld\n",
          calc_result[i]);  // print the calculation results to the output file
    }
  }
  fclose(out);        // close the output file
  free(t_buffer);     // free the memory allocated for the input buffer
  free(calc_result);  // free the memory allocated for the calculation results
  return 0;           // return success
}