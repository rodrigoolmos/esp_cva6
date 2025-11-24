/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * ESP + CVA6 platform integration for OpenSBI
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_const.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_platform.h>
#include <sbi_utils/fdt/fdt_fixup.h>
#include <sbi_utils/fdt/fdt_helper.h>
#include <sbi_utils/ipi/aclint_mswi.h>
#include <sbi_utils/irqchip/plic.h>
#include <sbi_utils/serial/gaisler-uart.h>
#include <sbi_utils/timer/aclint_mtimer.h>

#ifndef BASE_FREQ_MHZ
#define BASE_FREQ_MHZ 50
#endif

#define ESP_UART_ADDR			0x60000100UL
#define ESP_BASE_FREQ			(BASE_FREQ_MHZ * 1000000UL)
#define ESP_UART_BAUDRATE		38400
#define ESP_PLIC_ADDR			0x6c000000UL
#define ESP_PLIC_NUM_SOURCES		30
#define ESP_HART_COUNT			16
#define ESP_CLINT_ADDR			0x02000000UL
#define ESP_ACLINT_MSWI_ADDR		(ESP_CLINT_ADDR + CLINT_MSWI_OFFSET)
#define ESP_ACLINT_MTIMER_ADDR		(ESP_CLINT_ADDR + CLINT_MTIMER_OFFSET)
#define ESP_ACLINT_MTIMER_FREQ		ESP_BASE_FREQ

static struct plic_data plic = {
	.addr = ESP_PLIC_ADDR,
	.num_src = ESP_PLIC_NUM_SOURCES,
};

static struct aclint_mswi_data mswi = {
	.addr = ESP_ACLINT_MSWI_ADDR,
	.size = ACLINT_MSWI_SIZE,
	.first_hartid = 0,
	.hart_count = ESP_HART_COUNT,
};

static struct aclint_mtimer_data mtimer = {
	.mtime_freq = ESP_ACLINT_MTIMER_FREQ,
	.mtime_addr = ESP_ACLINT_MTIMER_ADDR + ACLINT_DEFAULT_MTIME_OFFSET,
	.mtime_size = ACLINT_DEFAULT_MTIME_SIZE,
	.mtimecmp_addr = ESP_ACLINT_MTIMER_ADDR +
			 ACLINT_DEFAULT_MTIMECMP_OFFSET,
	.mtimecmp_size = ACLINT_DEFAULT_MTIMECMP_SIZE,
	.first_hartid = 0,
	.hart_count = ESP_HART_COUNT,
	.has_64bit_mmio = TRUE,
};

static int esp_early_init(bool cold_boot)
{
	return 0;
}

static int esp_final_init(bool cold_boot)
{
	void *fdt;
	sbi_printf("[CVA6][FINAL INIT] esp_final_init(cold_boot=%d)\n", cold_boot);

	if (!cold_boot)
		return 0;

	fdt = fdt_get_address();
	fdt_fixups(fdt);

	sbi_printf("[CVA6][FINAL INIT] FDT fixups done\n");
	return 0;
}

static int esp_console_init(void)
{
	return gaisler_uart_init(ESP_UART_ADDR, ESP_BASE_FREQ,
				 ESP_UART_BAUDRATE);
}

static int plic_esp_warm_irqchip_init(int m_cntx_id, int s_cntx_id)
{
	size_t i, ie_words = plic.num_src / 32 + 1;

	sbi_printf("[CVA6][IRQCHIP] plic_esp_warm_irqchip_init start\n");
	/* By default, enable all IRQs for M-mode of target HART */
	if (m_cntx_id > -1) {
		for (i = 0; i < ie_words; i++)
			plic_set_ie(&plic, m_cntx_id, i, 1);
	}
	/* Enable all IRQs for S-mode of target HART */
	if (s_cntx_id > -1) {
		for (i = 0; i < ie_words; i++)
			plic_set_ie(&plic, s_cntx_id, i, 1);
	}
	/* By default, enable M-mode threshold */
	if (m_cntx_id > -1)
		plic_set_thresh(&plic, m_cntx_id, 1);
	/* By default, disable S-mode threshold */
	if (s_cntx_id > -1)
		plic_set_thresh(&plic, s_cntx_id, 0);

	sbi_printf("[CVA6][IRQCHIP] plic_esp_warm_irqchip_init done\n");
	return 0;
}


static int esp_irqchip_init(bool cold_boot)
{
	sbi_printf("[CVA6][IRQCHIP] esp_irqchip_init(cold_boot=%d)\n", cold_boot);
	u32 hartid = current_hartid();
	int ret;

	if (cold_boot) {
		ret = plic_cold_irqchip_init(&plic);
		if (ret)
			return ret;
	}
	sbi_printf("[CVA6][IRQCHIP] calling plic_warm_irqchip_init(hartid=%u)\n", hartid);

	return plic_esp_warm_irqchip_init(2 * hartid, 2 * hartid + 1);
}

static int esp_ipi_init(bool cold_boot)
{
	int ret;
	sbi_printf("[CVA6][IPI] esp_ipi_init(cold_boot=%d)\n", cold_boot);

	if (cold_boot) {
		ret = aclint_mswi_cold_init(&mswi);
		if (ret)
			return ret;
	}
	sbi_printf("[CVA6][IPI] calling aclint_mswi_warm_init()\n");

	return aclint_mswi_warm_init();
}

static int esp_timer_init(bool cold_boot)
{
	int ret;

	sbi_printf("[CVA6][TIMER] esp_timer_init(cold_boot=%d)\n", cold_boot);

	if (cold_boot) {
		sbi_printf("[CVA6][TIMER] cold init: mtime=0x%lx size=0x%lx mtimecmp=0x%lx size=0x%lx freq=%lu hart_count=%u\n",
			   mtimer.mtime_addr, mtimer.mtime_size,
			   mtimer.mtimecmp_addr, mtimer.mtimecmp_size,
			   mtimer.mtime_freq, mtimer.hart_count);
		ret = aclint_mtimer_cold_init(&mtimer, NULL);
		sbi_printf("[CVA6][TIMER] aclint_mtimer_cold_init() -> %d\n", ret);
		if (ret)
			return ret;
	}

	sbi_printf("[CVA6][TIMER] calling aclint_mtimer_warm_init()\n");
	ret = aclint_mtimer_warm_init();
	sbi_printf("[CVA6][TIMER] aclint_mtimer_warm_init() -> %d\n", ret);

	return ret;
}

const struct sbi_platform_operations platform_ops = {
	.early_init = esp_early_init,
	.final_init = esp_final_init,
	.console_init = esp_console_init,
	.irqchip_init = esp_irqchip_init,
	.ipi_init = esp_ipi_init,
	.timer_init = esp_timer_init,
};

const struct sbi_platform platform = {
	.opensbi_version = OPENSBI_VERSION,
	.platform_version = SBI_PLATFORM_VERSION(0x0, 0x01),
	.name = "ESP+CVA6 RISC-V",
	.features = SBI_PLATFORM_DEFAULT_FEATURES,
	.hart_count = ESP_HART_COUNT,
	.hart_stack_size = SBI_PLATFORM_DEFAULT_HART_STACK_SIZE,
	.platform_ops_addr = (unsigned long)&platform_ops,
};
