#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include "gc.h"

//entry of program
extern int64_t start_program(void *args);

struct CLASS_INFO {
	char *name;
	struct CLASS_INFO *superClass;
	char **fieldNameTable;
	char **methodNameTable;
	void **methodTable;
	uint64_t fieldCount;
	uint64_t methodCount;
};
typedef struct CLASS_INFO ClassInfo;

static uint64_t getFieldIndex(ClassInfo *classInfo, char *fieldName);
static void* getMethodPtr(ClassInfo *classInfo, char *methodName);

void println() {
	wprintf(L"\n");
}

void printL(int64_t v) {
	wprintf(L"%lld", v);
}

void printD(double v) {
	wprintf(L"%lf", v);
}

void printB(int8_t v) {
	if (v) {
		wprintf(L"true");
	} else {
		wprintf(L"false");
	}
}

//wchar_t == int32_t
void printC(wchar_t wc) {
	wprintf(L"%lc", wc);
}

void printS(void *charArray) {
	if (charArray == NULL ) {
		return;
	}
	int64_t length = *((int64_t*) charArray);
	if (length == 0) {
		return;
	}
	void *wstr = charArray + sizeof(int64_t) + sizeof(int8_t);
	wprintf(L"%ls", (wchar_t*) wstr);
}

// return field ptr
void* sysObjectField(int64_t *object, char *fieldName) {
	if (object == NULL ) {
		wprintf(L"Error in getting field from null point!\n");
		exit(1);
	}
	ClassInfo *classInfo = *((ClassInfo**) object);
	int index = getFieldIndex(classInfo, fieldName);
	if (index == -1) {
		if (classInfo->superClass != NULL ) {
			return sysObjectField(*((int64_t**) (object + 1)), fieldName);
		}
	} else {
		return object + 2 + index;
	}
	return NULL ;
}

void* sysObjectMethod(int64_t *object, char *methodName) {
	if (object == NULL ) {
		wprintf(L"Error in invoking method from null point!\n");
		exit(1);
	}
	ClassInfo *classInfo = *((ClassInfo**) object);
	return getMethodPtr(classInfo, methodName);
}

void* sysObjectAlloca(ClassInfo *classInfo) {
	uint32_t size = (classInfo->fieldCount + 2) * sizeof(int64_t);
	void *data = memset(GC_MALLOC(size), 0, size);
	ClassInfo **infoPtr = (ClassInfo**) data;
	*infoPtr = classInfo;
	return data;
}

static uint64_t getFieldIndex(ClassInfo *classInfo, char *fieldName) {
	char **fieldNameTable = classInfo->fieldNameTable;
	uint64_t fieldCount = classInfo->fieldCount;
	for (uint64_t i = 0; i < fieldCount; i++) {
		if (strcmp(fieldName, fieldNameTable[i]) == 0) {
			return i;
		}
	}
	return -1;
}

static void* getMethodPtr(ClassInfo *classInfo, char *methodName) {
	char **methodNameTable = classInfo->methodNameTable;
	void **methodTable = classInfo->methodTable;
	uint64_t methodCount = classInfo->methodCount;
	for (uint64_t i = 0; i < methodCount; i++) {
		if (strcmp(methodName, methodNameTable[i]) == 0) {
			return methodTable[i];
		}
	}
	if (classInfo->superClass != NULL ) {
		return getMethodPtr(classInfo->superClass, methodName);
	}
	return NULL ;
}

void* sysArrayElement(void *arrayObject, int64_t index) {
	if (arrayObject == NULL ) {
		wprintf(L"Error in getting element from null point!\n");
		exit(1);
	}

	int64_t length = *((int64_t*) arrayObject);
	int8_t width = *((int8_t*) (arrayObject + sizeof(int64_t)));

	if (length == 0) {
		wprintf(L"Error in getting element from zero array!\n");
		exit(1);
	}

	index = index % length;
	index = index < 0 ? index + length : index;
	void *head = arrayObject + sizeof(int64_t) + sizeof(int8_t);
	return head + index * width;
}

void* sysArrayAlloca(int64_t length, int8_t width, void *inital) {
	if (length < 0) {
		wprintf(L"Error in allocating array with length < 0 !\n");
		exit(1);
	}
	int64_t dataSize = (length + 1) * width;
	int64_t allocaSize = sizeof(int64_t) + sizeof(int8_t) + dataSize;
	void *object = GC_MALLOC(allocaSize);
	*((int64_t*) object) = length;
	*((int8_t*) (object + sizeof(int64_t))) = width;

	if (inital != NULL ) {
		memset(object + allocaSize - width, 0, width);
		memcpy(object + sizeof(int64_t) + sizeof(int8_t), inital,
				length * width);
	} else if (length > 0) {
		memset(object + sizeof(int64_t) + sizeof(int8_t), 0, dataSize);
	}
	return object;
}

int64_t* sysArrayLength(void *arrayObject) {
	if (arrayObject == NULL ) {
		wprintf(L"Error in getting length from null point!\n");
		exit(1);
	}
	return (int64_t*) arrayObject;
}

int64_t sysGetHeapSize() {
	return GC_get_heap_size();
}

void sysGC() {
	GC_gcollect();
}

int8_t sysInstanceOf(int64_t *object, ClassInfo *destClass) {
	if (object == NULL ) {
		return 1;
	} else {
		ClassInfo *srcClass = *((ClassInfo**) object);
		while (srcClass != NULL ) {
			if (srcClass == destClass) {
				return 1;
			} else {
				srcClass = srcClass->superClass;
			}
		}
		return 0;
	}
}

void* sysDynamicCast(int64_t *object, ClassInfo *destClass) {
	if (sysInstanceOf(object, destClass)) {
		return object;
	} else {
		return NULL ;
	}
}

int main(int argc, char **argv) {
	setlocale(LC_CTYPE, "en_US.UTF-8");
	GC_INIT();
	void *array = sysArrayAlloca(argc, sizeof(void*), NULL );
	for (size_t i = 0; i < argc; i++) {
		size_t length = strlen(argv[i]);
		void *argstr = sysArrayAlloca(length, sizeof(wchar_t), NULL );
		for (size_t j = 0; j < length; j++) {
			wchar_t *c = (wchar_t*) sysArrayElement(argstr, j);
			*c = argv[i][j];
		}

		void **element = (void**) sysArrayElement(array, i);
		*element = argstr;
	}

	return start_program(array);
}
