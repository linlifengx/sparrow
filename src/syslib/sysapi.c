#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gc.h"

struct CLASS_INFO {
	char *name;
	struct CLASS_INFO *superClass;
	char **fieldNameTable;
	char **methodNameTable;
	void **methodTable;
	unsigned int fieldCount;
	unsigned int methodCount;
};
typedef struct CLASS_INFO ClassInfo;

//static int alloca_time = 0;

static int getFieldIndex(ClassInfo *classInfo, char *fieldName);
void* getMethodPtr(ClassInfo *classInfo, char *methodName);

void gcInit() {
	GC_INIT();
}

void println() {
	printf("\n");
}

void printL(long long v) {
	printf("%lld", v);
}

void printD(double v) {
	printf("%lf", v);
}

void printB(int v) {
	if (v) {
		printf("true");
	} else {
		printf("false");
	}
}

// return field ptr
void* sysObjectField(int64_t *object, char *fieldName) {
	if (object == NULL ) {
		printf("Error in getting field from null point!\n");
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
		printf("Error in invoking method from null point!\n");
		exit(1);
	}
	ClassInfo *classInfo = *((ClassInfo**) object);
	return getMethodPtr(classInfo, methodName);
}

void* sysObjectAlloca(ClassInfo *classInfo) {
	//alloca_time++;
	unsigned int size = (classInfo->fieldCount + 2) * sizeof(int64_t);
	void *data = memset(GC_MALLOC(size), 0, size);
	ClassInfo **infoPtr = (ClassInfo**) data;
	*infoPtr = classInfo;
	/*	if (alloca_time % 1024 == 0) {
	 printf("Heap size = %d\n", GC_get_heap_size());
	 }*/
	return data;
}

int getFieldIndex(ClassInfo *classInfo, char *fieldName) {
	char **fieldNameTable = classInfo->fieldNameTable;
	unsigned int fieldCount = classInfo->fieldCount;
	for (unsigned int i = 0; i < fieldCount; i++) {
		if (strcmp(fieldName, fieldNameTable[i]) == 0) {
			return i;
		}
	}
	return -1;
}

void* getMethodPtr(ClassInfo *classInfo, char *methodName) {
	char **methodNameTable = classInfo->methodNameTable;
	void **methodTable = classInfo->methodTable;
	unsigned int methodCount = classInfo->methodCount;
	for (unsigned int i = 0; i < methodCount; i++) {
		if (strcmp(methodName, methodNameTable[i]) == 0) {
			return methodTable[i];
		}
	}
	if (classInfo->superClass != NULL ) {
		return getMethodPtr(classInfo->superClass, methodName);
	}
	return NULL ;
}

void* sysAlloca(int size) {
	//alloca_time++;
	void *data = memset(GC_MALLOC(size), 0, size);
	/*	if (alloca_time % 1024 == 0) {
	 printf("Heap size = %d\n", GC_get_heap_size());
	 }*/
	return data;
}
