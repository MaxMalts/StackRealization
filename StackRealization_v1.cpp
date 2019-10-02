// Версии:
// v1 - стэк фиксированного размера

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef _DEBUG
#define PrintStk_OK(stk) StackDump(&stk, __FILE__, __LINE__, __FUNCTION__, "just looking");
#define PrintStk_NOK(stk) StackDump(&stk, __FILE__, __LINE__, __FUNCTION__, "stack has an error");
#else
#define PrintStk_OK(stk) ;
#define PrintStk_NOK(stk) ;
#endif

const int stackMaxSize = 10;    ///<Максимальный размер стэка
typedef int elem_t;             ///<Тим элементов стэка


/**
*	Стэк
*/

#pragma pack(1)
struct stack_t {
#ifdef _DEBUG
	int secureVarBeg = 0;
#endif

	elem_t data[stackMaxSize] = {};    ///<Элементы стэка
	int size = 0;                      ///<Настоящий размер стэка
	elem_t emptyelem = -2147483647;    ///<Элемент, соответствующий пустому
#ifdef _DEBUG
	char name[30] = "";                ///<Имя стэка
	int err = 0;                       ///<Код ошибки, содержащейся в стэке:\n
	                                   ///0 - нет ошибок\n
	                                   ///1 - выход за вержнюю границу стэка\n
	                                   ///2 - выход за нижнюю границу стэка\n
	int hash = 0;
	int secureVarEnd = 0;
#endif
};
#pragma pack()

int CalcHash(stack_t*);

/**
*	Преобразует elem_t в строку
*
*	@param[in] elem Элемент
*
*	@return Указатель на строку
*/

char* elem_tToStr(elem_t elem) {

	const int elem_tMaxStrSize = 20;

	char* str = (char*)calloc(elem_tMaxStrSize, sizeof(char));
	itoa(elem, str, 10);
	return str;
}


/**
*	Выводит информацию о стэке
*
*	@param[in] stk Стэк
*	@param[in] file Название файла, окуда вызвали функцию
*	@param[in] line Номер строки, из которой вызвали функцию
*	@param[in] function Имя функции, из которой вызвали функцию
*	@param[in] reason Причина, по которой вызвали функцию
*/

#ifdef _DEBUG
void StackDump(stack_t* stk, const char* file, const int line, const char* function, const char* reason) {
	char status[10] = "ok";
	if (stk->err != 0) {
		strcpy(status, "ERR");
	}

	printf("\nInfo about a stack from file: %s, function: %s, line: %d, reason: %s:\n", file, function, line, reason);
	printf("stack_t \"%s\" (%p):    (%s)\n", stk->name, stk, status);
	printf("\tsecureVarBeg = %d\n", stk->secureVarBeg);
	printf("\tsize = %d\n", stk->size);

	printf("\tdata[%d] (%p):\n", stackMaxSize, &stk->data);
	for (int i = 0; i < stackMaxSize; i++) {
		char* elemStr = elem_tToStr(stk->data[i]);
		if (stk->data[i] == stk->emptyelem) {
			printf("\t\t[%d] = %s (poison),\n", i, elemStr);
		}
		else {
			printf("\t\t[%d] = %s,\n", i, elemStr);
		}
		free(elemStr);
	}

	printf("\tsecureVarEnd = %d\n", stk->secureVarEnd);
	printf("err = %d\n\n\n", stk->err);
}
#endif

/**
*	Проверяет стэк
*
*	@param[in] stk Стэк
*
*	@return fasle - стэк некорректный; true - стэк корректный
*/

#ifdef _DEBUG
bool StkOk(stack_t* stk) {
	if (stk->size > stackMaxSize) {
		stk->err = 1;
		return false;
	}
	if (stk->size < 0) {
		stk->err = 2;
		return false;
	}
	if (stk->secureVarBeg != 0) {
		stk->err = 3;
		return false;
	}
	if (stk->secureVarEnd != 0) {
		stk->err = 4;
		return false;
	}
	if (stk->hash != CalcHash(stk)) {
		stk->err = 5;
		return false;
	}
	stk->err = 0;
	return true;
}
#endif


/**
*	Вычисляет хэш-сумму стэка
*
*	@param[in] stk Стэк
*
*	@return Значение хэш-суммы
*/

#ifdef _DEBUG
int CalcHash(stack_t* stk) {
	assert(stk != NULL);

	int hash = 0;
	for (int i = 0; i < stackMaxSize; i++) {
		for (int j = 0; j < sizeof(elem_t); j++) {
			char curByte = (char)(*((char*)& stk->data[i] + j));
			hash = hash ^ (curByte*2+hash/2);
		}
	}
	for (int j = 0; j < sizeof(int); j++) {
		char curByte = (char)(*((char*)& stk->size + j));
		hash = hash ^ (curByte * 2 + hash / 2);
	}
	for (int j = 0; j < sizeof(elem_t); j++) {
		char curByte = (char)(*((char*)& stk->emptyelem + j));
		hash = hash ^ (curByte * 2 + hash / 2);
	}
#ifdef _DEBUG
	for (int i = 0; i < 30; i++) {
		char curByte = stk->name[i];
		hash = hash ^ (curByte * 2 + hash / 2);;
	}
	for (int j = 0; j < sizeof(int); j++) {
		char curByte = (char)(*((char*)& stk->err + j));
		hash = hash ^ (curByte * 2 + hash / 2);
	}
#endif

	return hash;
}
#endif


/**
*	Пересчитывает хэш-сумму стэка
*
*	@param[in] stk Стэк
*
*	@return 1 - ошибка в стэке после пересчитывания хэша; 0 - все прошло нормально
*/

#ifdef _DEBUG
int RecalcHash(stack_t* stk) {
	assert(stk != NULL);

	stk->hash = CalcHash(stk);

#ifdef _DEBUG
	if (!StkOk(stk)) {
		PrintStk_NOK(*stk);
		return 2;
	}
#endif

	return 0;
}
#endif

/**
*	Создает новый стэк, заполняя его "пустыми" элементами
*
*	@param[in] name Имя стэка
*
*	@return Указатель на созданный стэк
*/

stack_t StackConstructor(const char* name) {
	stack_t stk = {};
#ifdef _DEBUG
	strcpy(stk.name, name);
	stk.err = 0;
#endif

	elem_t emptyelem = stk.emptyelem;
	for (int i = 0; i < stackMaxSize; i++) {
		stk.data[i] = emptyelem;
	}
	stk.size = 0;

#ifdef _DEBUG
	RecalcHash(&stk);

	if (StkOk(&stk)) {
		PrintStk_OK(stk);
	}
	else {
		PrintStk_NOK(stk);
	}
#endif
	return stk;
}


/**
*	Берет элемент их стэка
*
*	@param[in] stk Стэк
*	@param[in] value Значение нового элемента
*
*	@return 0 - все прошло нормально; 2 - превышение максимального количества элементов стэка; 1 - проблемы со стэком
*/

int StackPush(stack_t* stk, elem_t value) {
	assert(stk != NULL);
#ifdef _DEBUG
	if (!StkOk(stk)) {
		PrintStk_NOK(*stk);
		return 1;
	}
#endif

	if (stk->size == stackMaxSize) return 2;
	stk->data[stk->size++] = value;

#ifdef _DEBUG
	RecalcHash(stk);

	if (StkOk(stk)) {
		PrintStk_OK(*stk);
	}
	else {
		PrintStk_NOK(*stk);
		return 1;
	}
#endif
	return 0;
}


/**
*	Берет элемент их стэка
*
*	@param[in] stk Стэк
*	@param[in] variable Переменная для возврата элемента. Если стэк пустой, остается нетронутой
*
*	@return 0 - все прошло нормально; 2 - в стэке нет элементов; 1 - проблемы со стэком
*/

int StackPop(stack_t* stk, elem_t* variable) {
	assert(stk != NULL);
#ifdef _DEBUG
	if (StkOk(stk)) {
		PrintStk_OK(*stk);
	}
	else {
		PrintStk_NOK(*stk);
		return 1;
	}
#endif

	int err = 0;
	if (stk->size > 0) {
		*variable = stk->data[--stk->size];
		stk->data[stk->size] = stk->emptyelem;
	}
	else {
		err = 2;
	}

#ifdef _DEBUG
	RecalcHash(stk);

	if (StkOk(stk)) {
		PrintStk_OK(*stk);
	}
	else {
		PrintStk_NOK(*stk);
		return 1;
	}
#endif

	return err;
}


/**
*	Удаляет стэк
*
*	@param[in] stk Стэк
*
*	@return 0 - все прошло нормально; 1 - проблемы со стэком
*/

int StackDestructor(stack_t* stk) {
	assert(stk != NULL);
#ifdef _DEBUG
	if (!StkOk(stk)) {
		PrintStk_NOK(*stk);
		return 1;
	}
#endif
	return 0;
}

int main() {
	stack_t stk1 = StackConstructor("stk1");
	StackPush(&stk1, 12);
	StackPush(&stk1, 65);

	int x = 0;
	int err = StackPop(&stk1, &x);

	StackDestructor(&stk1);
	getchar();
}