#include "codegen/symbol.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Symbol* symbol_table = NULL;
static int var_counter = 0;
static int func_var_counter = 0;

void init_symbol_table(void) {
    symbol_table = NULL;
    var_counter = 0;
}

void free_symbol_table(void) {
    Symbol *curr = symbol_table;
    while(curr) {
       Symbol *next = curr->next;
       free(curr->name);
       free(curr->label);
       if(curr->value) free(curr->value);
       free(curr);
       curr = next;
    }
    symbol_table = NULL;
    var_counter = 0;
}

Symbol* add_symbol(const char* name, const char* value, const char* type) {
    Symbol *sym = lookup_symbol(name);
    if(sym) {
        if(value) {
            free(sym->value);
            sym->value = strdup(value);
        }
        return sym;
    }
    
    sym = malloc(sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Memory allocation failed in add_symbol\n");
        exit(EXIT_FAILURE);
    }
    sym->name = strdup(name);
    if (!sym->name) {
        fprintf(stderr, "Memory allocation failed in add_symbol (name)\n");
        exit(EXIT_FAILURE);
    }
    sym->value = value ? strdup(value) : NULL;
    
    // Keep using the original name instead of using a generic label
    sym->label = strdup(name);
    
    sym->next = symbol_table;
    symbol_table = sym;
    sym->type = strdup(type);
    return sym;
}

int update_symbol_value(const char* name, const char* new_value) {
    Symbol* sym = lookup_symbol(name);
    if(sym) {
        if(new_value) {
            free(sym->value);
            sym->value = strdup(new_value);
        }
        return 1;
    }
    return 0;
}

Symbol* lookup_symbol(const char* name) {
    Symbol* curr = symbol_table;
    while(curr) {
       if(strcmp(curr->name, name) == 0)
           return curr;
       curr = curr->next;
    }
    return NULL;
}

Symbol* get_symbol_table(void) {
    return symbol_table;
}

void print_symbol_table(void) {
    Symbol* curr = symbol_table;
    while (curr) {
        printf("Symbol: %s, Label: %s, Value: %s\n",
               curr->name,
               curr->label,
               curr->value ? curr->value : "NULL");
        curr = curr->next;
    }
}