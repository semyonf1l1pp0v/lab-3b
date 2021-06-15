#include "table.h"
#include <stdlib.h>
#include <string.h>

// подменю особых операций
void subMenu(Table *table);

int main() {
    printf("Введите имя файла: ");
    char *str = getStr();
    Table *table = init(100, 100, str);
    free(str);
    Item *item = NULL;
    KeySpace2Ptr it = NULL;
    int key1;
    char key2[SIZE];
    FILE *file;
    InfoType *info;
    int menu;
    do {
        printf("1 - Включение элемента в таблицу\n");
        printf("2 - Поиск элемента в таблице\n");
        printf("3 - Удаление элемента из таблицы\n");
        printf("4 - Поиск элемента по первому ключу\n");
        printf("5 - Поиск элемента по второму ключу\n");
        printf("6 - Удаление элемента по первому ключу\n");
        printf("7 - Удаление элемента по второму ключу\n");
        printf("8 - Вывод таблицы на экран\n");
        printf("9 - Вывод таблицы в текстовый файл\n");
        printf("10 - Особые операции\n");
        printf("0 - Выход\n");
        menu = getInt();
        switch (menu) {
            case 1:
                item = getItem();
                info = getInfo();
                if (!addElement(table, item)) {
                    removeItem(item);
                    printf("Ошибка добавления элемента\n");
                }
                else {
                    infoToFile(table->filename, info, item);
                    printf("Элемент успешно добавлен\n");
                }
                removeInfo(info);
                break;
            case 2:
                printf("Введите первый ключ: ");
                key1 = getInt();
                printf("Введите второй ключ: ");
                str = getStr();
                strncpy(key2, str, SIZE);
                free(str);
                item = findElement(table, key1, key2);
                if (item) {
                    printf("Найден элемент с информацией:\n");
                    info = infoFromFile(table->filename, item->pos, item->len);
                    printf("%lf, %lf, %s\n", info->n1, info->n2, info->str);
                    removeInfo(info);
                }
                else {
                    printf("Элемент не найден\n");
                }
                break;
            case 3:
                printf("Введите первый ключ: ");
                key1 = getInt();
                printf("Введите второй ключ: ");
                str = getStr();
                strncpy(key2, str, SIZE);
                free(str);
                if (removeElement(table, key1, key2)) {
                    printf("Элемент успешно удален\n");
                }
                else {
                    printf("Элемент не найден\n");
                }
                break;
            case 4:
                printf("Введите ключ: ");
                key1 = getInt();
                item = findFirst(table, key1, NULL);
                if (item) {
                    printf("Найден элемент с информацией:\n");
                    info = infoFromFile(table->filename, item->pos, item->len);
                    printf("%lf, %lf, %s\n", info->n1, info->n2, info->str);
                    removeInfo(info);
                }
                else {
                    printf("Элемент не найден\n");
                }
                break;
            case 5:
                printf("Введите ключ: ");
                str = getStr();
                strncpy(key2, str, SIZE);
                free(str);
                it = findSecond(table, key2, NULL);
                if (it) {
                    printf("Найден(ы) элемент(ы) с информацией:\n");
                    do {
                        info = infoFromFile(table->filename, it->info->pos, it->info->len);
                        printf("%lf, %lf, %s\n", info->n1, info->n2, info->str);
                        removeInfo(info);
                        it = findSecond(table, key2, it);
                    } while (it);
                }
                else {
                    printf("Элемент не найден\n");
                }
                break;
            case 6:
                printf("Введите ключ: ");
                key1 = getInt();
                if (removeFirst(table, key1)) {
                    printf("Элемент успешно удален\n");
                }
                else {
                    printf("Элемент не найден\n");
                }
                break;
            case 7:
                printf("Введите ключ: ");
                str = getStr();
                strncpy(key2, str, SIZE);
                free(str);
                if (removeSecond(table, key2)) {
                    printf("Элемент(ы) удален(ы)\n");
                }
                else {
                    printf("Элемент не найден\n");
                }
                break;
            case 8:
                printTable(table, stdout);
                break;
            case 9:
                printf("Введите имя файла: ");
                str = getStr();
                file = fopen(str, "wt");
                if (file) {
                    printTable(table, file);
                    fclose(file);
                    printf("Таблица выведена в файл\n");
                }
                else {
                    printf("Не удалось создать файл\n");
                }
                break;
            case 10:
                subMenu(table);
                break;
            case 0:
                break;
            default:
                printf("Неверно выбран пункт меню\n");
                break;
        }
    } while (menu != 0);
    writeToFile(table);
    removeTable(table);
    return 0;
}

void subMenu(Table *table) {
    KeySpace1 *ks1 = NULL;
    KeySpace2Ptr *ks2 = NULL;
    int a, b;
    unsigned size;
    char *str;
    char key2[SIZE];
    unsigned release;
    printf("1 - Поиск элементов по диапазону (первое пространство)\n");
    printf("2 - Поиск всех версий элемента по ключу (второе пространство)\n");
    printf("3 - Поиск версии элемента по ключу (второе пространство)\n");
    printf("4 - Чистка таблицы (второе пространство)\n");
    int menu = getInt();
    switch (menu) {
        case 1:
            printf("Введите диапазон: ");
            a = getInt();
            b = getInt();
            ks1 = findRange(table->ks1, table->csize1, a, b, &size);
            printFirst(ks1, size, stdout, table->filename);
            removeKeySpace1(ks1);
            break;
        case 2:
            printf("Введите ключ: ");
            str = getStr();
            strncpy(key2, str, SIZE);
            free(str);
            ks2 = findSecond2(table->ks2, table->msize2, key2, 0);
            printSecond(ks2, table->msize2, stdout, table->filename);
            removeKeySpace2(ks2, table->msize2);
            break;
        case 3:
            printf("Введите ключ: ");
            str = getStr();
            strncpy(key2, str, SIZE);
            free(str);
            printf("Введите номер версии: ");
            release = getInt();
            ks2 = findSecond2(table->ks2, table->msize2, key2, release);
            printSecond(ks2, table->msize2, stdout, table->filename);
            removeKeySpace2(ks2, table->msize2);
            break;
        case 4:
            clearTable(table);
            printf("Чистка таблицы проведена\n");
            break;
        default:
            printf("Неверно выбран пункт меню\n");
            break;
    }
}

