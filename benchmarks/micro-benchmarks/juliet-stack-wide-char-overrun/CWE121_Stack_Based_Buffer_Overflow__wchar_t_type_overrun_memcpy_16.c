/* TEMPLATE GENERATED TESTCASE FILE
Filename: CWE121_Stack_Based_Buffer_Overflow__wchar_t_type_overrun_memcpy_15.c
Label Definition File: CWE121_Stack_Based_Buffer_Overflow.label.xml
Template File: point-flaw-15.tmpl.c
*/
/*
 * @description
 * CWE: 121 Stack Based Buffer Overflow
 * Sinks: type_overrun_memcpy
 *    GoodSink: Perform the memcpy() and prevent overwriting part of the structure
 *    BadSink : Overwrite part of the structure by incorrectly using the sizeof(struct) in memcpy()
 * Flow Variant: 15 Control flow: switch(6)
 *
 * */

#include "std_testcase.h"

#ifndef _WIN32
#include <wchar.h>
#endif

/* SRC_STR is 32 wchar_t long, including the null terminator, for 64-bit architectures */
//#define SRC_STR L"0123456789abcdef0123456789abcde"

typedef struct _charVoid
{
    wchar_t charFirst[16];
    void * voidSecond;
    void * voidThird;
} charVoid;

#ifndef OMITBAD

void CWE121_Stack_Based_Buffer_Overflow__wchar_t_type_overrun_memcpy_15_bad(wchar_t * arg, int idx)
{
    switch(idx)
    {
    case 6:
    {
        charVoid structCharVoid;
        structCharVoid.voidSecond = (void *)arg;
        /* Print the initial block pointed to by structCharVoid.voidSecond */
        printWLine((wchar_t *)structCharVoid.voidSecond);
        /* FLAW: Use the sizeof(structCharVoid) which will overwrite the pointer voidSecond */
        memcpy(structCharVoid.charFirst, arg, wcslen(arg));
        structCharVoid.charFirst[(sizeof(structCharVoid.charFirst)/sizeof(wchar_t))-1] = L'\0'; /* null terminate the string */
        printWLine((wchar_t *)structCharVoid.charFirst);
        printWLine((wchar_t *)structCharVoid.voidSecond);
    }
    break;
    default:
        printLine("Benign, fixed string");
        break;
    }
}

#endif /* OMITBAD */

#ifndef OMITGOOD

/* good1() changes the switch to switch(5) */
static void good1()
{
    switch(5)
    {
    case 6:
        /* INCIDENTAL: CWE 561 Dead Code, the code below will never run */
        printLine("Benign, fixed string");
        break;
    default:
    {
        charVoid structCharVoid;
        structCharVoid.voidSecond = (void *)SRC_STR;
        /* Print the initial block pointed to by structCharVoid.voidSecond */
        printWLine((wchar_t *)structCharVoid.voidSecond);
        /* FIX: Use sizeof(structCharVoid.charFirst) to avoid overwriting the pointer voidSecond */
        memcpy(structCharVoid.charFirst, SRC_STR, sizeof(structCharVoid.charFirst));
        structCharVoid.charFirst[(sizeof(structCharVoid.charFirst)/sizeof(wchar_t))-1] = L'\0'; /* null terminate the string */
        printWLine((wchar_t *)structCharVoid.charFirst);
        printWLine((wchar_t *)structCharVoid.voidSecond);
    }
    break;
    }
}

/* good2() reverses the blocks in the switch */
static void good2()
{
    switch(6)
    {
    case 6:
    {
        charVoid structCharVoid;
        structCharVoid.voidSecond = (void *)SRC_STR;
        /* Print the initial block pointed to by structCharVoid.voidSecond */
        printWLine((wchar_t *)structCharVoid.voidSecond);
        /* FIX: Use sizeof(structCharVoid.charFirst) to avoid overwriting the pointer voidSecond */
        memcpy(structCharVoid.charFirst, SRC_STR, sizeof(structCharVoid.charFirst));
        structCharVoid.charFirst[(sizeof(structCharVoid.charFirst)/sizeof(wchar_t))-1] = L'\0'; /* null terminate the string */
        printWLine((wchar_t *)structCharVoid.charFirst);
        printWLine((wchar_t *)structCharVoid.voidSecond);
    }
    break;
    default:
        /* INCIDENTAL: CWE 561 Dead Code, the code below will never run */
        printLine("Benign, fixed string");
        break;
    }
}

void CWE121_Stack_Based_Buffer_Overflow__wchar_t_type_overrun_memcpy_15_good()
{
    good1();
    good2();
}

#endif /* OMITGOOD */

/* Below is the main(). It is only used when building this testcase on
   its own for testing or for building a binary to use in testing binary
   analysis tools. It is not used when compiling all the testcases as one
   application, which is how source code analysis tools are tested. */

#ifdef INCLUDEMAIN

int main(int argc, char * argv[])
{
    /* seed randomness */
    if (argc < 3) {
        exit(0);
    }
    char * strArg = argv[1];
    size_t length = strlen(strArg) / 2 + 2;
    wchar_t wideArgArr[length];
    memcpy((void *) wideArgArr, (void *) strArg, strlen(strArg));
    wideArgArr[length - 1] = '\0';
    wideArgArr[length - 2] = '\0';

    printf("sizeof wideArgArr %ld\n", wcslen(wideArgArr));

    srand( (unsigned)time(NULL) );
#ifndef OMITGOOD
    printLine("Calling good()...");
    CWE121_Stack_Based_Buffer_Overflow__wchar_t_type_overrun_memcpy_15_good();
    printLine("Finished good()");
#endif /* OMITGOOD */
#ifndef OMITBAD
    printLine("Calling bad()...");
    CWE121_Stack_Based_Buffer_Overflow__wchar_t_type_overrun_memcpy_15_bad(wideArgArr, atoi(argv[2]));
    printLine("Finished bad()");
#endif /* OMITBAD */
    return 0;
}

#endif