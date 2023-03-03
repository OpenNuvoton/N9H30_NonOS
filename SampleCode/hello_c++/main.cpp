/* Standard includes. */
#include <stdlib.h>

/* Nuvoton includes. */
#include "N9H30.h"
#include "sys.h"

class MyTestClass
{
public:
	MyTestClass():
		test_variable(2)
	{

	}
	int GetTestVariable()
	{
		return test_variable;
	}
protected:
	int test_variable;
};

static MyTestClass my_test_class_a;

int main( void )
{

	/* Setup the hardware */
    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);
    sysInitializeUART();
    sysSetLocalInterrupt(ENABLE_IRQ);

    sysprintf((char *)"\n\nHello World!\r\n");

    int val_a = my_test_class_a.GetTestVariable();
    sysprintf((char *)"static construction: %d\r\n", val_a);

    MyTestClass* my_test_class_b = new MyTestClass();
    int val_b = my_test_class_b->GetTestVariable();
    sysprintf((char *)"dynamic construction: %d\r\n", val_b);
    delete my_test_class_b;

    return 0;
}

