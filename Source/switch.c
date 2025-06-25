#include "tinyOS.h"
#include "ARMCM3.H"

#define NVIC_INT_CTRL   0xE000ED04
#define NVIC_PENDSVSET  0x10000000
#define NVIC_SYSPRI2    0xE000ED22
#define NVIC_PENDSV_PRI 0x000000FF 


#define MEM32(addr) *(volatile unsigned long*)(addr)
#define MEM8(addr)  *(volatile unsigned char*)(addr)

//可成功执行，但后续采取汇编文件写法
#if 0
__attribute__((naked)) void PendSV_Handler()
{
    __asm volatile(
        "MRS R0, PSP\n"
        "CBZ R0, PendSVHandler_nosave\n"

        "STMDB R0!, {R4-R11}\n"

        "LDR R1, =currentTask\n"
        "LDR R1, [R1]\n"
        "STR R0, [R1]\n"
		
		"PendSVHandler_nosave:\n"

        "LDR R0, =currentTask\n"
        "LDR R1, =nextTask\n"
        "LDR R2, [R1]\n"
        "STR R2, [R0]\n"

        "LDR R0, [R2]\n"
        "LDMIA R0!, {R4-R11}\n"

        "MSR PSP, R0\n"
        "ORR LR, LR, #0x04\n"
        "BX LR\n"
    );
    
}
#endif

void tTaskRunFirst(void)
{
    __set_PSP(0);
    MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void tTaskSwitch(void)
{
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}
