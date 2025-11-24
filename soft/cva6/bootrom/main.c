#include <stdint.h>

#include "uart.h"

static void handle_trap(void);

int main()
{
    uintptr_t trap_vector = (uintptr_t)handle_trap;

    __asm__ volatile("csrw mtvec, %0" ::"r"(trap_vector));

    __asm__ volatile("csrr a0, mhartid;"
                     "bne a0, x0, 1f");

    init_uart();

    // jump to the address
    __asm__ volatile("1:"
                     "li s0, 0x80000000;"
                     "la a1, _dtb;"
                     "jr s0");

    while (1) {
        // do nothing
    }
}

static inline void print_label_hex(const char *label, uint64_t value)
{
    print_uart(label);
    print_uart("0x");
    print_uart_int64(value);
    print_uart("\n");
}

static void handle_trap(void)
{
    uint64_t mcause, mepc, mtval, mhartid;

    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    __asm__ volatile("csrr %0, mhartid" : "=r"(mhartid));

    print_uart("\n[ESP bootrom] Trap detected\n");
    print_label_hex(" hart      : ", mhartid);
    print_label_hex(" mcause    : ", mcause);
    print_label_hex(" mepc      : ", mepc);
    print_label_hex(" mtval     : ", mtval);

    while (1) {
        __asm__ volatile("wfi");
    }
}
