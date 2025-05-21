#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



/* Utils */

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

/* Utils end */

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
  LexerTokenType_END = 0,
  LexerTokenType_INVALID,

  LexerTokenType_STRING,
  LexerTokenType_SYMBOL,
  LexerTokenType_UNIQUE_UTF_8,
  LexerTokenType_NUMBER,
  LexerTokenType_ENDLINE,
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

    if (c == '\n'){
      LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_ENDLINE,NULL));
      continue;
    }
    else if(isspace(c)){
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

      LexerTokenList_push(&token_list, LexerToken_create(LexerTokenType_NUMBER,(char*)buffer));
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

  TokenType_KEYWORD = 0xFF,
  TokenType_IDENTIFIER,
 
  TokenType_COMMENT_START,
  TokenType_COMMENT,
  TokenType_COMMENT_END,
  TokenType_ENDLINE,


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

void TokenList_copy_LexerToken_to_Token(Token* token,const LexerToken* lexer_token){
  assert(token && lexer_token);
  token->len = lexer_token->len;
  token->type.lexer = lexer_token->type;
  token->value = lexer_token->value != NULL ? calloc(lexer_token->len+1,sizeof *lexer_token->value) : NULL;
  if(token->value != NULL){
    strncpy(token->value, lexer_token->value, lexer_token->len);
  }
}

void TokenList_copy_LexerTokenList_to_TokenList(TokenList* token_list,const LexerTokenList* lexer_token_list){
  for(size_t i =0; i < lexer_token_list->size; i++){
    TokenList_copy_LexerToken_to_Token(&token_list->items[i],&lexer_token_list->items[i]);
  }
}

TokenList TokenList_convert_LexerTokenList_to_TokenList(const LexerTokenList* lexer_list){

  TokenList token_list = TokenList_create(lexer_list->size);

  TokenList_copy_LexerTokenList_to_TokenList(&token_list,lexer_list);

  Token t;
  for(size_t i = 0; i < token_list.size; i++){
    t = token_list.items[i];

    switch (t.type.lexer) {
      case LexerTokenType_SYMBOL:
        if(strncmp(t.value, ";",t.len) == 0){
          token_list.items[i].type.token = TokenType_SEMICOLON;
        }else if(strncmp(t.value, "(",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPEN_PARENTHESES;
        }else if(strncmp(t.value, ")",t.len) == 0){
          token_list.items[i].type.token = TokenType_CLOSE_PARENTHESES;
        }else if(strncmp(t.value, "{",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPEN_BRACE;
        }else if(strncmp(t.value, "}",t.len) == 0){
          token_list.items[i].type.token = TokenType_CLOSE_BRACE;
        }else if(strncmp(t.value, "=",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPERATOR;
        }else if(strncmp(t.value, "<",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPERATOR;
        }else if(strncmp(t.value, ">",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPERATOR;
        }else if(strncmp(t.value, "+",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPERATOR;
        }else if(strncmp(t.value, "-",t.len) == 0){
          token_list.items[i].type.token = TokenType_OPERATOR;
        } //else if(strncmp(t.value, "/",t.len) == 0){
        //   if(strncmp(t.value, "/",t.len) == 0){
        //     if(strncmp(token_list.items[i].value, "/",token_list.items[i].len) == 0 &&
        //       token_list.size > i+1
        //     ){
        //       i++;
        //       if(strncmp(token_list.items[i].value,"/",token_list.items[i].len) == 0){
        //         while(token_list.size > i && token_list.items[i].type.lexer == LexerTokenType_ENDLINE){
        //           token_list.items[i++].type.token = TokenType_COMMENT;
        //         }
        //         if(token_list.items[i].type.lexer == LexerTokenType_ENDLINE)
        //           token_list.items[i++].type.token = TokenType_COMMENT;
        //       }
        //     }
        //   } else{
        //     token_list.items[i].type.token = TokenType_OPERATOR;
        //   }
        // }
        else { 
          continue;
        }
        break;
      case LexerTokenType_STRING:
        if(strncmp(t.value, "if",t.len) == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strncmp(t.value, "else",t.len) == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strncmp(t.value, "struct",t.len) == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strncmp(t.value, "return",t.len) == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else if(strncmp(t.value, "exit",t.len) == 0){
          token_list.items[i].type.token = TokenType_KEYWORD;
        }else {
          token_list.items[i].type.token = TokenType_IDENTIFIER;
        }
        break;
      case LexerTokenType_NUMBER:
        token_list.items[i].type.token = TokenType_INTEGER_LITERAL;
        break;
      case LexerTokenType_ENDLINE:
        token_list.items[i].type.token = TokenType_ENDLINE;
        break;
      default:
        break;
    }
  }
  return token_list;
}

const char* TokenList_to_asm(const TokenList* token_list){
  return NULL;
}

void TokenList_distroy(TokenList* token_list){
  for(size_t i=0; i<token_list->size; i++){
    Token_distroy(&token_list->items[i]);
  }
  free(token_list->items);
  token_list->items = NULL;
}

// TokenList end

// Token end

/* Classes end */

/* Function */
// ...
/* Function end*/

int main(void){
  const char* file = "./file.txt";
  const char* asm_output = "./out.s";

  StringBuilder sb = read_entire_file(file);
  LexerTokenList list = lexer(&sb);
  StringBuilder_distory(&sb);

  TokenList token_list = TokenList_convert_LexerTokenList_to_TokenList(&list);
  LexerTokenList_distroy(&list);

   for (size_t i = 0; i < token_list.size; i++) {
    switch (token_list.items[i].type.token){
      case TokenType_COMMENT_START:
        printf("comment start\n");
        while (token_list.items[i].type.token == TokenType_COMMENT) i++;
        break;
      case TokenType_COMMENT_END:
        printf("comment end\n");
        break;
      case TokenType_KEYWORD:
        if (strncmp(token_list.items[i].value,"exit",token_list.items[i].len) == 0){ 
          i++;
          if(token_list.items[i].type.token == TokenType_OPEN_PARENTHESES) i++;
          if(token_list.items[i].type.token == TokenType_INTEGER_LITERAL){
            printf("exit_value: %s\n",token_list.items[i].value);
            i++;
          }
          if(token_list.items[i].type.token == TokenType_CLOSE_PARENTHESES)
          {
            printf("find you\n");
          }
        }
        break;
      default:
        break;
    }
  }

  TokenList_distroy(&token_list);
  return 0;
}
