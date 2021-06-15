#include "table.h"
#include <stdlib.h>
#include <string.h>

// создание таблицы
Table* init(unsigned msize1, unsigned msize2, char *filename) {
    Table *table = (Table*)malloc(sizeof(Table));
    strcpy(table->filename, filename);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        // если файла нет, создаем его
        file = fopen(filename, "wb");
        fclose(file);
        table->msize1 = msize1;
        table->msize2 = msize2;
        table->csize1 = 0;
        table->ks1 = initKeySpace1(table->msize1);
        table->ks2 = initKeySpace2(table->msize2);
    }
    else {
        // иначе читаем таблицу из файла
        fclose(file);
        readFromFile(table);
    }
    return table;
}

KeySpace1* initKeySpace1(unsigned msize) {
    KeySpace1 *ks1 = (KeySpace1*)malloc(sizeof(KeySpace1)*msize);
    return ks1;
}

KeySpace2Ptr* initKeySpace2(unsigned msize) {
    KeySpace2Ptr *ks2 = (KeySpace2Ptr*)malloc(sizeof(KeySpace2Ptr)*msize);
    for (int i = 0; i < msize; i++) {
        ks2[i] = NULL;
    }
    return ks2;
}

// удаление таблицы
void removeTable(Table *table) {
    for (int i = 0; i < table->csize1; i++) {
        removeItem(table->ks1[i].info);
    }
    removeKeySpace2(table->ks2, table->msize2);
    removeKeySpace1(table->ks1);
}

void removeKeySpace1(KeySpace1 *ks1) {
    free(ks1);
}

void removeKeySpace2(KeySpace2Ptr *ks2, unsigned msize) {
    for (int i = 0; i < msize; i++) {
        KeySpace2Ptr ptr = ks2[i];
        while (ptr) {
            ks2[i] = ks2[i]->next;
            free(ptr);
            ptr = ks2[i];
        }
    }
    free(ks2);
}

// включение нового элемента в таблицу
int addElement(Table *table, Item *item) {
    if (findFirst(table, item->key1, NULL) || table->csize1 == table->msize1) {
        return 0;
    }
    addElementFirst(table->ks1, &(table->csize1), item);
    addElementSecond(table->ks2, table->msize2, item);
    return 1;
}

void addElementFirst(KeySpace1 *ks1, unsigned *csize, Item *item) {
    // ищем позицию в упорядоченном массиве для нового элемента
    int key = item->key1;
    int ind = (*csize)-1;
    while (ind >= 0 && ks1[ind].key > key)
    {
        ks1[ind+1].key = ks1[ind].key;
        ks1[ind+1].info = ks1[ind].info;
        ind--;
    }
    ks1[ind+1].key = key;
    ks1[ind+1].info = item;
    (*csize)++;
}

void addElementSecond(KeySpace2Ptr *ks2, unsigned size, Item *item) {
    unsigned ind = strHash(item->key2) % size; // получаем индекс в массиве для элемента
    // находим последний элемент в цепочке, параллельно определяя номер версии нового элемента,
    // если элемент(ы) с таким ключом уже есть
    KeySpace2Ptr ptr = ks2[ind];
    KeySpace2Ptr last = NULL;
    unsigned release = 1;
    while (ptr) {
        if (strcmp(ptr->key, item->key2) == 0) {
            release = ptr->release+1;
        }
        last = ptr;
        ptr = ptr->next;
    }
    // создаем новый элемент цепочки
    KeySpace2Ptr tmp = (KeySpace2Ptr)malloc(sizeof(KeySpace2));
    tmp->info = item;
    tmp->release = release;
    tmp->next = NULL;
    strcpy(tmp->key, item->key2);
    // добавляем элемент в список
    if (last) {
        last->next = tmp;
    }
    else {
        ks2[ind] = tmp;
    }
}

// поиск в таблице элемента, заданного составным ключом
Item* findElement(Table *table, int key1, char key2[]) {
    Item *item = findFirst(table, key1, NULL);
    if (item && strcmp(item->key2, key2) == 0) {
        return item;
    }
    else {
        return NULL;
    }
}

// удаление из таблицы элемента, заданного составным ключом
int removeElement(Table *table, int key1, char key2[]) {
    // т.к. первый ключ является уникальным, удаляем по нему
    // предварительно проверив, что второй ключ совпадает
    if (!findElement(table, key1, key2)) {
        return 0;
    }
    return removeFirst(table, key1);
}

// поиск элементов по любому заданному ключу

// для первого ключа дополнительно возвращаем индекс элемента в таблице
Item* findFirst(Table *table, int key1, unsigned *ind) {
    // используем бинарный поиск для поиска по упорядоченной таблице
    int low = 0;
    int up = table->csize1 - 1;
    while (low <= up) {
        int m = (low + up) / 2;
        if (table->ks1[m].key == key1) {
            if (ind) {
                *ind = m;
            }
            return table->ks1[m].info;
        }
        else if (table->ks1[m].key < key1)
            low = m + 1;
        else
            up = m - 1;
    }
    return NULL;
}

KeySpace2Ptr findSecond(Table *table, char key2[], KeySpace2Ptr prev) {
    KeySpace2Ptr ptr;
    if (!prev) { // первичный поиск
        ptr = table->ks2[strHash(key2) % table->msize2];
    }
    else { // поиск следующего элемента за prev
        ptr = prev->next;
    }
    while (ptr) {
        if (strcmp(ptr->key, key2) == 0) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

// удаление из таблицы элементов, заданного ключом в одном из ключевых пространств
int removeFirst(Table *table, int key1) {
    // находим элемент
    unsigned ind;
    Item *item = findFirst(table, key1, &ind);
    if (!item) {
        return 0;
    }
    // удаляем из самого первого пространства
    table->csize1--;
    for (int i = ind; i < table->csize1; i++) {
        table->ks1[i].info = table->ks1[i+1].info;
        table->ks1[i].key = table->ks1[i+1].key;
    }
    // удаляем из второго пространства
    unsigned ind2 = strHash(item->key2) % table->msize2;
    KeySpace2Ptr ptr = table->ks2[ind2];
    KeySpace2Ptr prev = NULL;
    while (ptr) {
        if (ptr->info == item) {
            if (!prev) {
                table->ks2[ind2] = ptr->next;
            }
            else {
                prev->next = ptr->next;
            }
            removeItem(item);
            free(ptr);
            break;
        }
        prev = ptr;
        ptr = ptr->next;
    }
    return 1;
}

int removeSecond(Table *table, char key2[]) {
    // находим элементы с key2 и удаляем их по первому пространству
    // (возможно из-за уникальности key1)
    int found = 0;
    KeySpace2Ptr ptr = findSecond(table, key2, NULL);
    while (ptr) {
        found = 1;
        removeFirst(table, ptr->info->key1);
        ptr = findSecond(table, key2, NULL);
    }
    return found;
}

// вывод содержимого таблицы на экран (или в текстовый файл)
void printTable(Table *table, FILE *file) {
    fprintf(file, "In first keyspace:\n");
    printFirst(table->ks1, table->csize1, file, table->filename);
    fprintf(file, "In second keyspace:\n");
    printSecond(table->ks2, table->msize2, file, table->filename);
}

void printFirst(KeySpace1 *ks1, unsigned csize, FILE *file, char *infoFile) {
    if (csize == 0) {
        fprintf(file, "Таблица пуста\n");
        return;
    }
    fprintf(file, "%10s | %40s |\n", "Key", "Info");
    fprintf(file, "-------------------------------------------------------\n");
    for (int i = 0; i < csize; i++) {
        InfoType *info = infoFromFile(infoFile, ks1[i].info->pos, ks1[i].info->len);
        fprintf(file, "%10d | %8.2f%8.2f%24s |\n", ks1[i].info->key1, info->n1, info->n2, info->str);
        removeInfo(info);
        fprintf(file, "-------------------------------------------------------\n");
    }
    
}

void printSecond(KeySpace2Ptr *ks2, unsigned msize, FILE *file, char *infoFile) {
    int isEmpty = 1;
    for (int i = 0; i < msize; i++) {
        KeySpace2Ptr ptr = ks2[i];
        while (ptr) {
            if (isEmpty) {
                isEmpty = 0;
                fprintf(file, "%20s | %10s | %40s |\n", "Key", "Release", "Info");
                fprintf(file, "------------------------------------------------------------------------------\n");
            }
            InfoType *info = infoFromFile(infoFile, ptr->info->pos, ptr->info->len);
            fprintf(file, "%20s | %10u | %8.2f%8.2f%24s |\n", ptr->key, ptr->release, info->n1, info->n2, info->str);
            removeInfo(info);
            fprintf(file, "------------------------------------------------------------------------------\n");
            ptr = ptr->next;
        }
    }
    if (isEmpty) {
        fprintf(file, "Таблица пуста\n");
    }
}

// особые операции
KeySpace1* findRange(KeySpace1 *ks1, unsigned csize, int min, int max, unsigned *size) {
    KeySpace1 *table = initKeySpace1(csize);
    *size = 0;
    // с помощью бинарного поиска находим индекс первого элемента больше min
    int low = 0;
    int up = csize;
    while (low < up) {
        int m = (low + up) / 2;
        if (min <= ks1[m].key) {
            up = m;
        }
        else {
            low = m + 1;
        }
    }
    // добавляем элементы диапазона в новую таблицу
    while (low < csize && ks1[low].key <= max) {
        addElementFirst(table, size, ks1[low].info);
        low++;
    }
    return table;
}

KeySpace2Ptr* findSecond2(KeySpace2Ptr *ks2, unsigned msize, char key2[], unsigned release) {
    KeySpace2Ptr *table = initKeySpace2(msize);
    KeySpace2Ptr ptr = ks2[strHash(key2) % msize];
    while (ptr) {
        if (strcmp(ptr->key, key2) == 0 && (!release || release == ptr->release)) {
            addElementSecond(table, msize, ptr->info);
        }
        ptr = ptr->next;
    }
    return table;
}

void clearTable_(KeySpace2Ptr *ks2, unsigned msize, int removeItems) {
    for (int i = 0; i < msize; i++) {
        KeySpace2Ptr prev = NULL;
        KeySpace2Ptr current = ks2[i];
        while (current) {
            KeySpace2 *current2 = current->next;
            while (current2 && strcmp(current2->key, current->key) != 0) {
                current2 = current2->next;
            }
            if (current2) {
                KeySpace2Ptr toDel = current;
                if (prev) {
                    prev->next = current->next;
                }
                else {
                    ks2[i] = current->next;
                }
                current = current->next;
                if (removeItems) {
                    removeItem(toDel->info);
                }
                else {
                    toDel->info->pos = -1;
                }
                free(toDel);
            }
            else {
                prev = current;
                current = current->next;
            }
        }
    }
}

void clearTable(Table *table) {
    clearTable_(table->ks2, table->msize2, 0);
    unsigned size = 0;
    KeySpace1 *ks1 = initKeySpace1(table->msize1);
    for (int i = 0; i < table->csize1; i++) {
        if (table->ks1[i].info->pos == -1) {
            free(table->ks1[i].info);
        }
        else {
            ks1[size++] = table->ks1[i];
        }
    }
    free(table->ks1);
    table->ks1 = ks1;
    table->csize1 = size;
}

char* getStr(void) {
    char *str = NULL;
    char buf[81];
    size_t size = 0;
    int res;
    do {
        res = scanf("%80[^\n]", buf);
        if (res == 0) {
            scanf("%*c");
            if (!str) {
                str = malloc(sizeof(char));
                str[0] = '\0';
            }
        } else if (res > 0) {
            size_t len = strlen(buf);
            str = realloc(str, (size+len+1)*sizeof(char));
            memcpy(str+size, buf, len);
            size += len;
            str[size] = '\0';
        }
    } while (res > 0);
    return str;
}

int getInt(void) {
    int n, res;
    do {
        res = scanf("%d", &n);
        scanf("%*[^\n]s");
        scanf("%*c");
        if (res != 1) {
            printf("Ошибка, введите целое число\n");
        }
    } while (res != 1);
    return n;
}

float getFloat(void) {
    float n;
    int res;
    do {
        res = scanf("%f", &n);
        scanf("%*[^\n]s");
        scanf("%*c");
        if (res != 1) {
            printf("Ошибка, введите число\n");
        }
    } while (res != 1);
    return n;
}

unsigned long strHash(char str[]) {
    unsigned long hash = 0;
    for (int i = 0; i < strlen(str); i++) {
        hash += (unsigned long)str[i];
    }
    return hash;
}

Item* getItem() {
    Item *item = (Item*)malloc(sizeof(Item));
    printf("Введите первый ключ (число): ");
    item->key1 = getInt();
    printf("Введите второй ключ (строка): ");
    char *str = getStr();
    strncpy(item->key2, str, SIZE);
    free(str);
    return item;
}

InfoType* getInfo() {
    InfoType* info = (InfoType*)malloc(sizeof(InfoType));
    printf("Введите информацию\n");
    printf("Первое число: ");
    info->n1 = getFloat();
    printf("Второе число: ");
    info->n2 = getFloat();
    printf("Строка: ");
    info->str = getStr();
    return info;
}

void removeInfo(InfoType *info) {
    free(info->str);
    free(info);
}

void removeItem(Item *item) {
    free(item);
}

// функции для работы с файлом

// чтение и запись таблицы
void readFromFile(Table *table) {
    FILE *file = fopen(table->filename, "rb");
    fread((char*)table, sizeof(Table), 1, file); // описатель таблицы
    table->ks1 = initKeySpace1(table->msize1);
    table->ks2 = initKeySpace2(table->msize2);
    // описатели элементов
    int size = table->csize1;
    table->csize1 = 0;
    for (int i = 0; i < size; i++) {
        Item *item = (Item*)malloc(sizeof(Item));
        fread((char*)item, sizeof(Item), 1, file);
        addElement(table, item);
    }
    fclose(file);
}

void writeToFile(Table *table) {
    
    // открываем файл для записи
    FILE *output = fopen("tmp", "wb");
    // определяем позицию для записи первого элемента (информации) в новом файле
    long pos = sizeof(Table) + table->csize1 * sizeof(Item);
    fwrite((char*)table, sizeof(Table), 1, output); // описатель таблицы
    // описатели элементов и информация
    for (int i = 0; i < table->csize1; i++) {
        long oldPos = table->ks1[i].info->pos;
        table->ks1[i].info->pos = pos;
        fwrite((char*)(table->ks1[i].info), sizeof(Item), 1, output);
        long currentPos = ftell(output);
        fseek(output, pos, SEEK_SET);
        InfoType *info = infoFromFile(table->filename, oldPos, table->ks1[i].info->len);
        fwrite((char*)&(info->n1), sizeof(float), 1, output);
        fwrite((char*)&(info->n2), sizeof(float), 1, output);
        fwrite(info->str, sizeof(char), table->ks1[i].info->len, output);
        removeInfo(info);
        pos = ftell(output);
        fseek(output, currentPos, SEEK_SET);
    }
    fclose(output);
    remove(table->filename);
    rename("tmp", table->filename);
}

// чтение и запись информации
InfoType* infoFromFile(char* filename, long pos, long len) {
    FILE *file = fopen(filename, "rb");
    fseek(file, pos, SEEK_SET);
    InfoType *info = (InfoType*)malloc(sizeof(InfoType));
    info->str = (char*)malloc(len);
    fread((char*)&(info->n1), sizeof(float), 1, file);
    fread((char*)&(info->n2), sizeof(float), 1, file);
    fread(info->str, sizeof(char), len, file);
    fclose(file);
    return info;
}

void infoToFile(char* filename, InfoType *info, Item *item) {
    FILE *file = fopen(filename, "ab");
    item->pos = ftell(file);
    item->len = strlen(info->str)+1;
    fwrite((char*)&(info->n1), sizeof(float), 1, file);
    fwrite((char*)&(info->n2), sizeof(float), 1, file);
    fwrite(info->str, sizeof(char), item->len, file);
    fclose(file);
}


