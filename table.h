#ifndef TABLE_H
#define TABLE_H
#include <stdio.h>

#define SIZE 80 // длина строки ключа

// информация - a record of two float numbers and a string with variable length (null-terminated)
typedef struct InfoType {
    float n1, n2;
    char *str;
} InfoType;

// структура элемента таблицы
typedef struct Item {
    long pos; // позиция информации в файле
    long len; // длина строковой части информации
    int key1;
    char key2[SIZE];
} Item;

// структура элемента первого пространства ключей
typedef struct KeySpace1 {
    int key;
    Item *info;
} KeySpace1;

// структура элемента второго пространства ключей
typedef struct KeySpace2 {
    char key[SIZE];
    unsigned release;
    Item *info;
    struct KeySpace2 *next;
} KeySpace2;
typedef KeySpace2* KeySpace2Ptr;

// структура таблицы
typedef struct Table {
    char filename[80]; // дополняем именем файла
    KeySpace1 *ks1;
    KeySpace2Ptr *ks2;
    unsigned msize1;
    unsigned msize2;
    unsigned csize1;
} Table;

// создание таблицы
Table* init(unsigned msize1, unsigned msize2, char *filename);
KeySpace1* initKeySpace1(unsigned msize);
KeySpace2Ptr* initKeySpace2(unsigned msize);
// удаление таблицы
void removeTable(Table *table);
void removeKeySpace1(KeySpace1 *ks1);
void removeKeySpace2(KeySpace2Ptr *ks2, unsigned msize);

// включение нового элемента в таблицу
int addElement(Table *table, Item *item);
void addElementFirst(KeySpace1 *ks1, unsigned *csize, Item *item);
void addElementSecond(KeySpace2Ptr *ks2, unsigned msize, Item *item);
// поиск в таблице элемента, заданного составным ключом
Item* findElement(Table *table, int key1, char key2[]);
// удаление из таблицы элемента, заданного составным ключом
int removeElement(Table *table, int key1, char key2[]);
// поиск элементов по любому заданному ключу
Item* findFirst(Table *table, int key1, unsigned *ind);
KeySpace2Ptr findSecond(Table *table, char key2[], KeySpace2Ptr prev);
// удаление из таблицы элементов, заданного ключом в одном из ключевых пространств
int removeFirst(Table *table, int key1);
int removeSecond(Table *table, char key2[]);
// вывод содержимого таблицы на экран (или в текстовый файл)
void printTable(Table *table, FILE *file);
void printFirst(KeySpace1 *ks1, unsigned csize, FILE *file, char *infoFile);
void printSecond(KeySpace2Ptr *ks2, unsigned msize, FILE *file, char *infoFile);
// особые операции
KeySpace1* findRange(KeySpace1 *ks1, unsigned csize, int min, int max, unsigned *size);
KeySpace2Ptr* findSecond2(KeySpace2Ptr *ks2, unsigned msize, char key2[], unsigned release);
void clearTable_(KeySpace2Ptr *ks2, unsigned msize, int removeItems);
void clearTable(Table *table);

char* getStr(void);
int getInt(void);
float getFloat(void);
unsigned long strHash(char str[]);

Item* getItem(void);
void removeInfo(InfoType *info);
void removeItem(Item *item);

// функции для работы с файлом

// отделяем ввод информации
InfoType* getInfo(void);

// чтение и запись таблицы
void readFromFile(Table *table);
void writeToFile(Table *table);

// чтение и запись информации
InfoType* infoFromFile(char* filename, long pos, long len);
void infoToFile(char* filename, InfoType *info, Item *item);

#endif


