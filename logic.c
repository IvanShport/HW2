#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#define START_MAX_SIZE_FOR_VECTOR 4
#define START_MAX_SIZE_FOR_STACK 4
#define START_SIZE_FOR_STRING 32

typedef struct Data{    // Переменные
    char* name;
    int value;
}Data;

Data* create_data(size_t size){
    Data* data = (Data*)malloc(sizeof(Data));
    if(data == NULL) return NULL;
    data->value = 0;
    data->name = (char*)malloc(size * sizeof(char));
    if(data->name == NULL){
        free(data);
        return NULL;
    }
    memset(data->name, 0, sizeof(char) * size);

    return data;
}

void free_data(Data* data){
    free(data->name);
    free(data);
}

//---------------------------------------------------------------

typedef struct Vector{   // Вектор переменных
    Data** values;
    size_t real_size;
    size_t max_size;
}Vector;

Vector* create_vector(){
    Vector* vec = (Vector*)malloc(sizeof(Vector));
    if(vec == NULL) return NULL;
    vec->max_size = START_MAX_SIZE_FOR_VECTOR;
    vec->real_size = 0;
    vec->values = (Data**)malloc(vec->max_size * sizeof(Data*));
    if(vec->values == NULL){
        free(vec);
        return NULL;
    }

    return vec;
}

Data** realloc_for_vector(Vector* vec){
    vec->max_size *= 2;
    Data** temp_arr = (Data**)realloc(vec->values, vec->max_size * sizeof(Data*));
    if(temp_arr == NULL) return NULL;
    vec->values = temp_arr;

    return vec->values;
}

Data* get(Vector* vec, char* name){
    for(size_t i = 0; i < vec->real_size; i++){
        if(strcmp(name, vec->values[i]->name) == 0)
            return vec->values[i];
    }
    return NULL;
}

int push_back(Vector* vec, Data* val){
    if(vec->real_size == vec->max_size){
        if(realloc_for_vector(vec) == NULL) return 1;
    }

    vec->values[vec->real_size] = val;
    vec->real_size++;
    return 0;
}

void pop_back(Vector* vec){
    if(!vec->real_size)
        return;
    vec->real_size--;
}

void free_vector(Vector* vec){
    for(size_t i = 0; i < vec->real_size; i++)
        free_data(vec->values[i]);
    free(vec->values);
    free(vec);
}

//---------------------------------------------------------------------------------

typedef struct Stack{  //  Стек для операндов и операторов
    void* values;
    size_t real_size;
    size_t max_size;
}Stack;

Stack* create_stack(size_t item_size){
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if(stack == NULL) return NULL;
    stack->max_size = START_MAX_SIZE_FOR_STACK;
    stack->real_size = 0;
    stack->values = malloc(item_size * stack->max_size);
    if(stack->values == NULL){
        free(stack);
        return NULL;
    }
    return stack;
}

void free_stack(Stack* stack){
    free(stack->values);
    free(stack);
}

void* realloc_for_stack(Stack* stack, size_t item_size){
    stack->max_size *= 2;
    void* temp = realloc(stack->values, stack->max_size * item_size);
    if(temp == NULL) return NULL;
    stack->values = temp;
    return stack->values;
}

void pop(Stack* stack){
    if(!stack->real_size)
        return;

    stack->real_size--;
}

int push(Stack* stack, void* new_value, size_t item_size){
    if(stack->real_size == stack->max_size){
        if(realloc_for_stack(stack, item_size) == NULL) return 1;
    }
    memcpy(stack->values + stack->real_size, new_value, item_size);
    stack->real_size++;
    return 0;
}

void* top(Stack* stack){
    return stack->real_size != 0? stack->values + stack->real_size - 1: NULL;
}

//-------------------------------------------------------------------

char* input_row();  // Ввод сторки
int init(Vector *names_and_values, char* str);  // Инициализация переменных
int isvalid(char* name);  // Проверка на совпадение c or, and, xor, not
int isrepit(Vector* vec_of_name ,char* name);  // Проверка на повторение имен
int calc(Vector* names_and_values, char* str);  // Вычиление выражения
int search_and(char* str, size_t i);
int search_not(char* str, size_t i);
int search_xor(char* str, size_t i);
int search_or(char* str, size_t i);
int search_true(char* str, size_t i);
int search_false(char* str, size_t i);
int set_operator(Stack* operators, char op, Stack* operands); // Добавление операторов в стек
int get_priority(char symbol);  // Приоритеты операций
int calculate(Stack* operands, char op);  // Вычиление операндов в зависимости от оператора
int output_result(Stack* operands, Stack* operators); // Вывод результата

int main(){
    Vector* names_and_values = create_vector();
    if(names_and_values == NULL){
        printf("[error]\n");
        return 0;
    }
    while(1){
        char* str = input_row();
        if(str == NULL){
            printf("[error]\n");
            free_vector(names_and_values);
            return 0;
        }

        if(strlen(str) != 0 && str[strlen(str) - 1] == ';'){  // Инициализация
            if(init(names_and_values, str) != 0){
                printf("[error]\n");
                free_vector(names_and_values);
                return 0;
            }
        }
        else{                                             // Вычисление
            if(calc(names_and_values ,str) != 0){
                printf("[error]\n");
                free_vector(names_and_values);
                return 0;
            }
            else
                break;
        }
    }

    free_vector(names_and_values);


    return 0;
}

char* input_row(){
    size_t size = START_SIZE_FOR_STRING;
    char* row_pointer = (char*)malloc(size * sizeof(char));
    if(row_pointer == NULL) return NULL;

    ptrdiff_t cur_size = 1;
    size_t empty_size = size;

    char* end_row = row_pointer;

    row_pointer[0] = '\0';
    while(fgets(end_row, empty_size, stdin)){
        if(end_row[strlen(end_row) - 1] == '\n'){
            end_row[strlen(end_row) - 1] = '\0';
            break;
        }

        empty_size = size;
        size *= 2;
        char* temp_row = (char*)realloc(row_pointer, size * sizeof(char));
        if(temp_row == NULL) return NULL;
        row_pointer = temp_row;
        end_row = row_pointer + (empty_size - cur_size);
        cur_size++;
    }

    char* temp_row = (char*)realloc(row_pointer, (strlen(row_pointer) + 1) * sizeof(char));
    if(temp_row == NULL) return NULL;
    row_pointer = temp_row;

    return row_pointer;
}


int init(Vector* names_and_values, char* str){
    size_t i = 0;
    for(; isspace(str[i]); i++){}   // Игнорирование пробелов

    size_t size_of_name = i;

    for(; str[size_of_name] != ' ' &&          // Поиск конца имени
          str[size_of_name] != '=' &&
          str[size_of_name] != ';';
          size_of_name++){
        if(!islower(str[size_of_name])){
            free(str);
            return 1;
        }
    }

    Data* data = create_data(size_of_name - i + 1);   // Создание переменной
    if(data == NULL){
        free(str);
        return 1;
    }
    memcpy(data->name, str + i, size_of_name - i);
    if(!isvalid(data->name)){
        free_data(data);
        free(str);
        return 1;
    }
    i = size_of_name;

    for(; str[i] != ';' &&                                  // Поиск символа '=' или ';'
           ((!islower(str[i - 1]) &&
            !isspace(str[i - 1])) ||
           str[i] != '=' ||
           (!isalpha(str[i + 1]) &&
            !isspace(str[i + 1])));
           i++){
        if(!isspace(str[i])){
            free_data(data);
            free(str);
            return 1;
        }
    }

    for(;str[i] != ';'; i++){         // Поиск значения True
        if(search_true(str, i)){
            data->value = 1;
            break;
        }
    }
    if(isrepit(names_and_values ,data->name)){
        Data* repit_data = get(names_and_values, data->name);
        repit_data->value = data->value;
        free_data(data);

    }
    else{
        if(push_back(names_and_values, data) != 0){   // Добавление переменной с вектор
            free_data(data);
            free(str);
            return 1;
        }
    }
    free(str);

    return 0;
}

int isvalid(char* name){
    if(strcmp(name, "and") == 0 ||
       strcmp(name, "or") == 0 ||
       strcmp(name, "xor") == 0 ||
       strcmp(name, "not") == 0){
        return 0;
    }
    return 1;
}

int isrepit(Vector* vec_of_name ,char* name){
    if(get(vec_of_name, name) == NULL){
        return 0;
    }
    return 1;
}

int calc(Vector* names_and_values, char* str){
    Stack* operators = create_stack(sizeof(char));
    if(operators == NULL){
        free(str);
        return 1;
    }
    Stack* operands = create_stack(sizeof(int));
    if(operands == NULL){
        free_stack(operators);
        free(str);
        return 1;
    }
    size_t i = 0;
    for(; i < strlen(str); i++){
        if(search_and(str, i)){
            i += 2;
            if(set_operator(operators, '&', operands) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(search_not(str, i)){
            i += 2;
            if(set_operator(operators, '~', operands) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(search_xor(str, i)){
            i += 2;
            if(set_operator(operators, '^', operands) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(search_or(str, i)){
            i++;
            if(set_operator(operators, '|', operands) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(str[i] == '('){           // Поиск оператора (
            if(set_operator(operators, '(', operands) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(str[i] == ')'){          // Поиск оператора )
            while(*(char*)(top(operators)) != '('){          // Вычисляем содержимое скобок
                if(calculate(operands, *(char*)(top(operators))) != 0){
                    free(str);
                    free_stack(operators);
                    free_stack(operands);
                    return 1;
                }
                pop(operators);
            }
            pop(operators);
            if(operators->real_size != 0 && *(char*)(top(operators)) == '~'){  // Проверка на оператор not
                int new_value = !(*(int*)top(operands) % 256);
                pop(operands);
                if(push(operands, &new_value, sizeof(int)) != 0){
                    free(str);
                    free_stack(operators);
                    free_stack(operands);
                    return 1;
                }

                pop(operators);
            }
        }
        else if(search_true(str, i)){
            i += 3;
            int true_value = 1;
            if(operators->real_size != 0 && *(char*)(top(operators)) == '~'){
                true_value = !true_value;
                pop(operators);
            }
            if(push(operands, &true_value, sizeof(int)) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(search_false(str, i))
        {
            i += 4;
            int false_value = 0;
            if(operators->real_size != 0 && *(char*)(top(operators)) == '~'){
                false_value = !false_value;
                pop(operators);
            }
            if(push(operands, &false_value, sizeof(int)) != 0){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
        }
        else if(islower(str[i]) && (i == 0 || isspace(str[i - 1]) || ispunct(str[i - 1])))  // Поиск переменной
        {
            size_t j = i;
            for(; str[j] && !isspace(str[j + 1]) && !ispunct(str[j + 1]); j++){}  // Поиск конца имени
            char* temp = (char*)calloc((j - i + 2), sizeof(char));
            if(temp == NULL){
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
            temp = memcpy(temp, str + i, j - i + 1);
            i = j;
            if(get(names_and_values, temp) != NULL){
                int new_value = get(names_and_values, temp)->value;
                if(operators->real_size != 0 && *(char*)(top(operators)) == '~'){
                    new_value = !new_value;
                    pop(operators);
                }
                if(push(operands, &new_value, sizeof(int)) != 0){
                    free(temp);
                    free(str);
                    free_stack(operators);
                    free_stack(operands);
                    return 1;
                }
            }
            else{
                free(temp);
                free(str);
                free_stack(operators);
                free_stack(operands);
                return 1;
            }
            free(temp);
        }

    }

    free(str);
    while(operators->real_size){         //  Вычисление выражения
        if(calculate(operands, *(char*)(top(operators))) != 0){
            free_stack(operators);
            free_stack(operands);
            return 1;
        }
        pop(operators);
    }

    if(output_result(operands, operators) != 0){    // Вывод результата
        free_stack(operators);
        free_stack(operands);
        return 1;
    }

    free_stack(operators);
    free_stack(operands);

    return 0;
}

int search_and(char* str, size_t i){
    return (i + 2 <= strlen(str) &&   // Поиск оператора and
       str[i] == 'a' &&
       (i == 0 ||
        isspace(str[i - 1]) ||
        str[i - 1] == ')' ||
        str[i - 1] == '(') &&
       str[i + 1] == 'n' &&
       str[i + 2] == 'd' &&
       (!str[i + 3] ||
        isspace(str[i + 3]) ||
        str[i + 3] == '(' ||
        str[i + 3] == ')'));
}

int search_not(char* str, size_t i){
    return (i + 2 <= strlen(str) &&   // Поиск оператора not
            str[i] == 'n' &&
            (i == 0 ||
             isspace(str[i - 1]) ||
             str[i - 1] == ')' ||
             str[i - 1] == '(') &&
            str[i + 1] == 'o' &&
            str[i + 2] == 't' &&
            (!str[i + 3] ||
             isspace(str[i + 3]) ||
             str[i + 3] == '(' ||
             str[i + 3] == ')'));
}

int search_xor(char* str, size_t i){
    return (i + 2 <= strlen(str) &&   // Поиск оператора xor
            str[i] == 'x' &&
            (i == 0 ||
             isspace(str[i - 1]) ||
             str[i - 1] == ')' ||
             str[i - 1] == '(') &&
            str[i + 1] == 'o' &&
            str[i + 2] == 'r' &&
            (!str[i + 3] ||
             isspace(str[i + 3]) ||
             str[i + 3] == '(' ||
             str[i + 3] == ')'));
}

int search_or(char* str, size_t i){
    return (i + 1 <= strlen(str) &&    // Поиск оператора or
            str[i] == 'o' &&
            (i == 0 ||
             isspace(str[i - 1]) ||
             str[i - 1] == ')' ||
             str[i - 1] == '(') &&
            str[i + 1] == 'r' &&
            (!str[i + 2] ||
             isspace(str[i + 2]) ||
             str[i + 2] == '(' ||
             str[i + 2] == ')'));
}

int search_true(char* str, size_t i){
    return (i + 3 <= strlen(str) &&   // Поиск значения True
            str[i] == 'T' &&
            (i == 0 ||
             isspace(str[i - 1]) ||
             ispunct(str[i - 1])) &&
            str[i + 1] == 'r' &&
            str[i + 2] == 'u' &&
            str[i + 3] == 'e' &&
            (!str[i + 4] ||
             isspace(str[i + 4]) ||
             ispunct(str[i + 4])));
}

int search_false(char* str, size_t i){
    return (i + 4 <= strlen(str) &&   // Поиск значения False
            str[i] == 'F' &&
            (i == 0 ||
             isspace(str[i - 1]) ||
             ispunct(str[i - 1])) &&
            str[i + 1] == 'a' &&
            str[i + 2] == 'l' &&
            str[i + 3] == 's' &&
            str[i + 4] == 'e' &&
            (!str[i + 5] ||
             isspace(str[i + 5]) ||
             ispunct(str[i + 5])));
}

int set_operator(Stack* operators, char op, Stack* operands){
    if(op == '(' ||
       op == '~' ||
       operators->real_size == 0 ||
       get_priority(op) >= get_priority(*(char*)(top(operators))))
    {
        if(push(operators, &op, sizeof(op)) != 0) return 1;
    }
    else{
        while(operators->real_size != 0 &&
              get_priority(*(char*)(top(operators))) > get_priority(op))
        {
            if(calculate(operands, *(char*)(top(operators))) != 0) return 1;
            pop(operators);
        }
        if(push(operators, &op, sizeof(op)) != 0) return 1;
    }
    return 0;
}

int get_priority(char symbol){
    switch (symbol) {
    case '(':
        return 1;
    case '|':
        return 2;
    case '^':
        return 3;
    case '&':
        return 4;
    default:
        return 0;
    }
}

int calculate(Stack* operands, char op){
    int value_right = *(int*)top(operands) % 256;
    pop(operands);
    int value_left = *(int*)top(operands) % 256;
    pop(operands);
    int result = 1;
    switch (op) {
    case '|':
        result = value_left | value_right;
        break;
    case '^':
        result = value_left ^ value_right;
        break;
    case '&':
        result = value_left & value_right;
        break;
    }
    if(push(operands, &result, sizeof(int)) != 0) return 1;
    return 0;
}

int output_result(Stack* operands, Stack* operators){
    if(operands->real_size == 1 && operators->real_size == 0){
        if(*(int*)top(operands) % 256 == 0)
            printf("False");
        else
            printf("True");
        return 0;
    }
    else{
        return 1;
    }
}
