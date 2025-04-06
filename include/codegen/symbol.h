#ifndef SYMBOL_H
#define SYMBOL_H

/* A simple symbol structure to hold the variable name, 
   a label (used when generating assembly code) and its value. */
typedef struct Symbol {
    char *name;
    char *label;   // e.g., "var0", "var1", etc.
    char *value;   // stored string value (for printing)
    struct Symbol *next;
} Symbol;

/* Initializes the symbol table (if needed). */
void init_symbol_table(void);

/* Frees all symbols in the table. */
void free_symbol_table(void);

/* Add a new symbol with the given name and value.
   If a symbol with the same name already exists, its value is updated. */
Symbol* add_symbol(const char *name, const char *value);

/* Look up a symbol by name. Returns NULL if not found. */
Symbol* lookup_symbol(const char *name);

/* Returns the head of the symbol table (for iterating through it). */
Symbol* get_symbol_table(void);

#endif