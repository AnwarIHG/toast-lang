#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

size_t string_len(const char* s){
  assert(s);
  size_t n;
  for(n=0; *s; s++, n++);
  return n;
}

/* Structs */

typedef struct{
} Node;

/* Structs end */

/* Classes  */

// class Token


// int keyword
// Identifier “main”
// Open brace
// return keyword
// Constant “2”
// Semicolon
// Close brace

//  ;
// Int keyword int
// Return keyword return
// Identifier [a-zA-Z]\w*
// Integer literal [0-9]+

typedef enum { 
  TokenType_INVALID = 0,
  TokenType_END,

  TokenType_KEYWORD,
  TokenType_IDENTIFIER,
 
  TokenType_OPEN_PARENTHESES,
  TokenType_CLOSE_PARENTHESES,

  TokenType_OPEN_BRACE,
  TokenType_CLOSE_BRACE,

  TokenType_SEMICOLON,
  TokenType_INTEGER_LITERAL,

  // custome type
  TokenType_UNIQUE,
  TokenType_UNIQUE_UTF_8,

  TokenType_STRING,
} TokenType;

typedef struct{
  char* value;
  TokenType type;
  unsigned char len;
} Token;

Token Token_create(TokenType type,const char* value){
  unsigned char len = value != NULL ? (unsigned char)string_len(value): 0;
  Token token = {
    .type = type,
    .len = len,
    .value = value != NULL ? calloc(len+1,sizeof *value) : NULL,
  };

  if(token.value != NULL) strncpy(token.value,value,len);

  return token;
}

void Token_distroy(Token* token){
  free(token->value);
  token->value = NULL;
}

// Token end

// class TokenList

typedef struct{
  Token* items;
  size_t capacity;
  size_t size;
} TokenList;

#define TOKEN_LIST_CAPACITY 2
TokenList TokenList_create(void){
  return (TokenList){
    .capacity = TOKEN_LIST_CAPACITY,
    .size = 0,
    .items = NULL,
  };
}

void TokenList_push(TokenList* token_list,Token token){
  assert(token_list);

  if(token_list->size >= token_list->capacity || token_list->items == NULL){
    size_t new_capacity = token_list->capacity + (token_list->capacity/2);
    token_list->items = (Token*)realloc(token_list->items,new_capacity * sizeof(Token));
    token_list->capacity = new_capacity;
  } 

  token_list->items[token_list->size++] = token;
}

void TokenList_distroy(TokenList* token_list){
  for(size_t i=0; i<token_list->size; i++){
    Token_distroy(&token_list->items[i]);
  }
  free(token_list->items);
  token_list->items = NULL;
}

// TokenList end

// class StringBuilder 

typedef struct{
  char* value;
  size_t size;
} StringBuilder;

StringBuilder read_entire_file(const char* file_name){

  assert(file_name);

  size_t file_size;

  FILE* file = fopen(file_name, "rb");
  if(file == NULL) {perror("file dosn't exiset"); exit(1);};

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* output = (char*)calloc(file_size,sizeof(char));

  if((fread(output, file_size, 1, file)) < 0) {perror("can't fread file"); exit(1);}

  fclose(file);

  const StringBuilder sb = {
    .value = output,
    .size = file_size
  };

  return sb;
}

void StringBuilder_distory(StringBuilder* sb) {
  free(sb->value);
}

// class StringBuilder end 

/* Classes end */

/* Functions */

/* The first byte of a UTF-8 character
 * indicates how many bytes are in
 * the character, so only check that
 */
int number_of_bytes_in_char(unsigned char val) {
    if (val < 128) {
        return 1;
    } else if (val < 224) {
        return 2;
    } else if (val < 240) {
        return 3;
    } else {
        return 4;
    }
}

#define LEXER_BUFFER_SIZE 32
TokenList lexer(const StringBuilder *sb){
  assert(sb);

  TokenList token_list = TokenList_create();

  char buffer[LEXER_BUFFER_SIZE] = {0};
  short len = 0;

  #define clean_buffer(BUFFER)\
    memset(buffer, 0, sizeof buffer);\
    len = 0;

  char c;


  for (size_t i = 0; i<sb->size; i++) {
    c = sb->value[i];

    if(isspace(c)){
      continue;
    }

    else if(!isalnum(c) && !isspace(c)){
      int bnic = number_of_bytes_in_char((unsigned char)c);

      // bnic means byte number in chars

      if(bnic-1 >= 1){

        int k = 0;

        while(!isalnum(sb->value[i]) && !isspace(sb->value[i]) && bnic > k){
          buffer[len++] = sb->value[i++];
          k++;
        }
        k = 0; i--;
        
        TokenList_push(&token_list, Token_create(TokenType_UNIQUE_UTF_8,buffer));
        clean_buffer(buffer);
      }else {
        buffer[len++] = c;
        TokenList_push(&token_list, Token_create(TokenType_UNIQUE,buffer));
        clean_buffer(buffer);
      } 

      continue;
    }

    else if (isalpha(c)) {

      buffer[len++] = c;
      i++;

      while(isalpha(sb->value[i])){
        buffer[len++] = sb->value[i++];
      }

      i--;

      TokenList_push(&token_list, Token_create(TokenType_STRING,(char*)buffer));
      clean_buffer(buffer);
      continue;
    }

    else if (isdigit(c)){

      buffer[len++] = c;
      i++;

      while(isdigit(sb->value[i])){
        buffer[len++] = sb->value[i++];
      }

      i--;

      TokenList_push(&token_list, Token_create(TokenType_INTEGER_LITERAL,(char*)buffer));
      clean_buffer(buffer);
      continue;
    }

    else {
      TokenList_push(&token_list, Token_create(TokenType_INVALID,NULL));
      clean_buffer(buffer);
      continue;
    }
  }
  TokenList_push(&token_list, Token_create(TokenType_END,NULL));
  clean_buffer(buffer);

  #undef clean_buffer
  return token_list;
}

/* Functions end */

int main(void){
  const char* file = "./pg100.txt";

  StringBuilder sb = read_entire_file(file);
  TokenList list = lexer(&sb);
  StringBuilder_distory(&sb);

  TokenList_distroy(&list);
  return 0;
}
