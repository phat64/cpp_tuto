// tested on onlinegdb : https://onlinegdb.com/zPwGKtS_1

/******************************************************************************

cpp_tuto : 10. How to change a local variable outside the scope ? (c) phat64

*******************************************************************************/


#include <stdio.h>

void set(int index, int value)
{
    int tmp;
    *(index + 8 + (int*)(&tmp)) = value;
}

int get(int index)
{
    int tmp;
    return *(index + 8 + (int*)(&tmp));
}

int get2(int index)
{
    int tmp;
    return *(index + 12 + (int*)(&tmp));
}

void printVars(void)
{
    printf("printVars\n");
    printf("x = %d\n", get2(0));
    printf("y =  %d\n", get2(1));
    printf("z =  %d\n", get2(2));
}

void setVars(void)
{
    set(0 + 4, 108);
    set(1 + 4, 109);
    set(2 + 4, 110);
}

void setVars1(void)
{
    set(0 + 8, 208);
    set(1 + 8, 209);
    set(2 + 8, 210);
}

void setVars2(void)
{
    setVars1();
}

int main()
{
    int x = 5;
    int y = 99;
    int z = 300;
    
    printf("main (before changes) :\n");
    printf("x = %d\n", x);
    printf("y = %d\n", y);
    printf("z = %d\n", z);

    set(0, 1234);
    set(1, 5678);
    set(2, 9999);

    printf("main (after changes) :\n");
    printf("x = %d\n", x);
    printf("y = %d\n", y);
    printf("z = %d\n", z);


    printf("main (get from variable index) :\n");
    printf("x = %d\n", get(0));
    printf("y = %d\n", get(1));
    printf("z = %d\n", get(2));

    printf("main (before setVars) :\n");
    printVars();
    setVars();
    printf("main (after setVars) :\n");
    printVars();

    printf("main (before setVars2) :\n");
    printVars();
    setVars2();
    printf("main (after setVars2) :\n");
    printVars();

    return 0;
}

/*
output :
main (before changes) :
x = 5
y = 99
z = 300
main (after changes) :
x = 1234
y = 5678
z = 9999
main (get from variable index) :
x = 1234
y = 5678
z = 9999
main (before setVars) :
printVars
x = 1234
y =  5678
z =  9999
main (after setVars) :
printVars
x = 108
y =  109
z =  110
main (before setVars2) :
printVars
x = 108
y =  109
z =  110
main (after setVars2) :
printVars
x = 208
y =  209
z =  210
*/




