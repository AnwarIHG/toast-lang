#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



/* Functions */

size_t string_len(const char* s){
  assert(s);
  size_t n;
  for(n=0; *s; s++, n++);
  return n;
}

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

void intToStr(int N, char *str) {
    if(N == 0){
      str[0] = '0';
      return;
    }

    int i = 0;
    int sign = N;
    if (N < 0)
        N = -N;

    while (N > 0) {
        str[i++] = N % 10 + '0';
      	N /= 10;
    } 

    if (sign < 0) {
        str[i++] = '-';
    }

    str[i] = '\0';

    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

int strToInt(const char* str){
  int num = 0;

  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] >= 48 && str[i] <= 57) {
      num = num * 10 + (str[i] - 48);
    }
    else {
      break;
    }
  }
  return num;
}

/* Functions end */

/* Classes  */

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


// class Lexer

typedef enum {
  LexerTokenType_INVALID = 0,
  LexerTokenType_END,

  LexerTokenType_STRING,
  LexerTokenType_SYMBOL,
  LexerTokenType_UNIQUE_UTF_8,
  LexerTokenType_INTEGER_LITERAL,
} LexerTokenType;

typedef struct{
  char* value;
  LexerTokenType type;
  unsigned char len;
} LexerToken;

LexerToken LexerToken_create(LexerTokenType type,const char* value){
  unsigned char len = value != NULL ? (unsigned char)string_len(value): 0;
  LexerToken token = {
    .type = type,
    .len = len,
    .value = value != NULL ? calloc(len+1,sizeof *value) : NULL,
  };

  if(token.value != NULL) strncpy(token.value,value,len);

  return token;
}

void LexerToken_distroy(LexerToken* token){
  free(token->value);
  token->value = NULL;
}

// LexerToken end

// LexerTokenList

typedef struct{
  LexerToken* items;
  size_t capacity;
  size_t size;
} LexerTokenList;

#define TOKEN_LIST_CAPACITY 2
LexerTokenList LexerTokenList_create(void){
  return (LexerTokenList){
    .capacity = TOKEN_LIST_CAPACITY,
    .size = 0,
    .items = NULL,
  };
}

void LexerTokenList_push(LexerTokenList* token_list,LexerToken token){
  assert(token_list);

  if(token_list->size >= token_list->capacity || token_list->items == NULL){
    size_t new_capacity = token_list->capacity + (token_list->capacity/2);
    token_list->items = (LexerToken*)realloc(token_list->items,new_capacity * sizeof(LexerToken));
    token_list->capacity = new_capacity;
  } 

  token_list->items[token_list->size++] = token;
}

void LexerTokenList_distroy(LexerTokenList* token_list){
  for(size_t i=0; i<token_list->size; i++){
    LexerToken_distroy(&token_list->items[i]);
  }
  free(token_list->items);
  token_list->items = NULL;
}

// LexerTokenList end

#define LEXER_BUFFER_SIZE 32
LexerTokenList lexer(const StringBuilder *sb){
  assert(sb);

  LexerTokenList token_list = LexerTokenList_create();

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

      // bnic = byte number in chars

      if(bnic-1 >= 1){

        int k = 0;

        while(!isalnum(sb->value[i]) && !isspace(sb->value[i]) && bnic > k){
          buffer[len++] = sb->value[i++];
          k++;
        }
        k = 0; i--;
        
        LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_UNIQUE_UTF_8,buffer));
        clean_buffer(buffer);
      }else {
        buffer[len++] = c;
        LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_SYMBOL,buffer));
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
 
      LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_STRING,(char*)buffer));
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

      LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_INTEGER_LITERAL,(char*)buffer));
      clean_buffer(buffer);
      continue;
    }

    else {
      LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_INVALID,(char*)buffer));
      clean_buffer(buffer);
      continue;
    }
  }
  LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_END,NULL));
  clean_buffer(buffer);

  #undef clean_buffer
  return token_list;
}

// Lexer end

// class Token

typedef enum { 
  TokenType_KEYWORD,
  TokenType_IDENTIFIER,
 
  TokenType_OPERATOR,

  TokenType_OPEN_PARENTHESES,
  TokenType_CLOSE_PARENTHESES,

  TokenType_OPEN_BRACE,
  TokenType_CLOSE_BRACE,

  TokenType_SEMICOLON,
  TokenType_INTEGER_LITERAL,

  // custome type
  TokenType_UNIQUE_UTF_8,
  TokenType_STRING,
} TokenMainType;


typedef union{
  LexerTokenType lexer;
  TokenMainType token;
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

// TokenList
typedef struct{
  Token* items;
  size_t size;
} TokenList;

TokenList TokenList_create(size_t size){
  assert(size);
  return (TokenList){
    .size = size,
    .items = (Token*)calloc(size,sizeof(Token)),
  };
}

void TokenList_copy_LexerToken_2_Token(Token* token,const LexerToken* lexer_token){
  assert(token && lexer_token);
  printf("[INFO]:lexer_token->len: %d\n",lexer_token->len);
  token->len = lexer_token->len;
  printf("[INFO]:token->len: %d\n",token->len);
  token->type.lexer = lexer_token->type;
  token->value = lexer_token->value != NULL ? calloc(lexer_token->len+1,sizeof *lexer_token->value) : NULL;
  if(token->value != NULL){
    strncpy(token->value, lexer_token->value, lexer_token->len);
  }
}

void TokenList_copy_LexerTokenList_2_TokenList(TokenList* token_list,const LexerTokenList* lexer_token_list){
  for(size_t i =0; i < lexer_token_list->size; i++){
    TokenList_copy_LexerToken_2_Token(&token_list->items[i],&lexer_token_list->items[i]);
  }
}

void TokenList_distroy(TokenList* token_list){
  for(size_t i=0; i<token_list->size; i++){
    Token_distroy(&token_list->items[i]);
  }
  free(token_list->items);
  token_list->items = NULL;
}

TokenList TokenList_convert_LexerTokenList_2_TokenList(LexerTokenList* lexer_list){

  TokenList token_list = TokenList_create(lexer_list->size);
  TokenList_copy_LexerTokenList_2_TokenList(&token_list,lexer_list);

  LexerToken lt;
  for(size_t i = 0; i< lexer_list->size; i++){
    lt = lexer_list->items[i];

    switch (lt.type) {
      case LexerTokenType_SYMBOL:
        if(strcmp(lt.value, ";") == 0){
          token_list.items[i].type.token = TokenType_SEMICOLON;
        }else if(strcmp(lt.value, "(") == 0){
          token_list.items[i].type.token = TokenType_OPEN_PARENTHESES;
        }else if(strcmp(lt.value, ")") == 0){
          token_list.items[i].type.token = TokenType_CLOSE_PARENTHESES;
        }else if(strcmp(lt.value, "{") == 0){
          token_list.items[i].type.token = TokenType_OPEN_BRACE;
        }else if(strcmp(lt.value, "}") == 0){
          token_list.items[i].type.token = TokenType_CLOSE_BRACE;
        }else if(strcmp(lt.value, "=") == 0){
          token_list.items[i].type.token = TokenType_OPERATOR;
        }else {
          continue;
        }
      case LexerTokenType_STRING:
        if(strcmp(lt.value, "if") == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strcmp(lt.value, "else") == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strcmp(lt.value, "struct") == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strcmp(lt.value, "return") == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strcmp(lt.value, "exit") == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else {
          continue;
        }
        break;
      default:
        break;
    }
  }
  return token_list;
}

// TokenList end

// Token end

/* Classes end */

int main(void){
  const char* file = "./file.txt";

  StringBuilder sb = read_entire_file(file);
  LexerTokenList list = lexer(&sb);
  StringBuilder_distory(&sb);

  TokenList token_list = TokenList_convert_LexerTokenList_2_TokenList(&list);
  LexerTokenList_distroy(&list);

  for (size_t i = 0; i < list.size; i++) {
    printf("%s\n",token_list.items[i].value);
  }
  
  TokenList_distroy(&token_list);
  return 0;
}
