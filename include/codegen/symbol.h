#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct Symbol {
    char* name;
    char* type;
    char* label;
    char* value;
    struct Symbol* next;
} Symbol;

void init_symbol_table(void);
void free_symbol_table(void);
Symbol* add_symbol(const char* name, const char* value, const char* type);
Symbol* lookup_symbol(const char *name);
Symbol* get_symbol_table(void);

#endif