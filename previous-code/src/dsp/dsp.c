/*
	DSP M56001 emulation
	Dummy emulation, Hatari glue

	(C) 2001-2008 ARAnyM developer team
	Adaption to Hatari (C) 2008 by Thomas Huth

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#include <ctype.h>

#include "main.h"
#include "sysdeps.h"
#include "ioMem.h"
#include "dsp.h"
#include "configuration.h"
#include "cycInt.h"
#include "m68000.h"
#include "sysReg.h"
#include "dma.h"

#if ENABLE_DSP_EMU
#include "debugdsp.h"
#include "dsp_cpu.h"
#include "dsp_disasm.h"
#endif

#define DEBUG 0
#if DEBUG
#define Dprintf(a) printf a
#else
#define Dprintf(a)
#endif

#define LOG_DSP_LEVEL       LOG_DEBUG
#define LOG_DSP_REG_LEVEL   LOG_DEBUG


#define DSP_RAMSIZE_MAX  (3*64*1024)
#define DSP_RAMSIZE_24kB (1<<13)  /*  8k DSP words */
#define DSP_RAMSIZE_96kB (1<<15)  /* 32k DSP words */

static uint32_t dsp_ram[DSP_RAMSIZE_MAX];

#if ENABLE_DSP_EMU
static const char* x_ext_memory_addr_name[] = {
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"PBC", "PCC", "PBDDR", "PCDDR", "PBD", "PCD", "", "",
	"HCR", "HSR", "", "HRX/HTX", "CRA", "CRB", "SSISR/TSR", "RX/TX",
	"SCR", "SSR", "SCCR", "STXA", "SRX/STX", "SRX/STX", "SRX/STX", "",
	"", "", "", "", "", "", "BCR", "IPR"
};

static int32_t save_cycles;
#endif

static bool bDspDebugging;

bool bDspEnabled = false;
bool bDspEmulated = false;


/**
 * Handle TXD interrupt at host CPU
 */
#if ENABLE_DSP_EMU
void DSP_HandleTXD(int set) {
	if (set) {
		Log_Printf(LOG_WARN, "[DSP] Set TXD interrupt");
		dsp_txdn_intr = 1;
	} else {
		Log_Printf(LOG_WARN, "[DSP] Release TXD interrupt");
		dsp_txdn_intr = 0;
	}
	scr_check_dsp_interrupt();
}
#endif


/**
 * Handle HREQ at the host CPU.
 */
#if ENABLE_DSP_EMU
static void DSP_HandleHREQ(int set)
{
	if (dsp_core.dma_mode) {
		dsp_hreq_intr = 0;
		if (set) {
			dsp_core.dma_request = 1;
		} else {
			dsp_core.dma_request = 0;
		}
	} else {
		dsp_core.dma_request = 0;
		if (set) {
			Log_Printf(LOG_DSP_LEVEL, "[DSP] Set HREQ interrupt");
			dsp_hreq_intr = 1;
		} else {
			Log_Printf(LOG_DSP_LEVEL, "[DSP] Release HREQ interrupt");
			dsp_hreq_intr = 0;
		}
	}
	scr_check_dsp_interrupt();
}
#endif


/**
 * Host DSP DMA interface: Set DSP IRQB at the end of a DMA block.
 */
void DSP_SetIRQB(void)
{
#if ENABLE_DSP_EMU
	if (dsp_intr_at_block_end) {
		dsp_set_interrupt(DSP_INTER_IRQB, 1);
	}
#endif
}


/**
 * Host DSP DMA interface: Handling DMA transfers.
 */
#if ENABLE_DSP_EMU
static void DSP_HandleDMA(void)
{
	if (dsp_core.dma_mode && dsp_core.dma_request && dma_dsp_ready()) {
		/* Set the counter according to selected DMA mode */
		if (dsp_core.dma_address_counter==0) {
			dsp_core.dma_address_counter = 4-dsp_core.dma_mode;
			/* Handle unpacked mode on Turbo systems */
			if (dsp_dma_unpacked && ConfigureParams.System.bTurbo) {
					dsp_core.dma_address_counter = 4;
			}
		}
		dsp_core.dma_address_counter--;
		
		/* Read or write via DMA */
		if (dsp_core.dma_direction==(1<<CPU_HOST_ICR_TREQ)) {
			dsp_core_write_host(CPU_HOST_TRXL-dsp_core.dma_address_counter, dma_dsp_read_memory());
		} else {
			dma_dsp_write_memory(dsp_core_read_host(CPU_HOST_TRXL-dsp_core.dma_address_counter));
		}
		
		/* Handle unpacked mode on non-Turbo systems */
		if (dsp_dma_unpacked && dsp_core.dma_address_counter==0 && !ConfigureParams.System.bTurbo) {
			if (dsp_core.dma_direction==(1<<CPU_HOST_ICR_TREQ)) {
				dsp_core_write_host(CPU_HOST_TRX0, dma_dsp_read_memory());
			} else {
				dma_dsp_write_memory(dsp_core_read_host(CPU_HOST_TRX0));
			}
			return;
		}
	}
}
#endif


/**
 * Initialize the DSP emulation
 */
void DSP_Init(void)
{
#if ENABLE_DSP_EMU
	if (bDspEnabled)
		return;
	dsp_core_init(DSP_HandleHREQ);
	dsp56k_init_cpu();
	bDspEnabled = true;
	save_cycles = 0;
#endif
}


/**
 * Shut down the DSP emulation
 */
void DSP_UnInit(void)
{
#if ENABLE_DSP_EMU
	if (!bDspEnabled)
		return;
	dsp_core_shutdown();
	bDspEnabled = false;
#endif
}


/**
 * Reset the DSP emulation
 */
void DSP_Reset(void)
{
#if ENABLE_DSP_EMU
/*	LogTraceFlags = TRACE_DSP_ALL; */
	if (ConfigureParams.System.nDSPType==DSP_TYPE_NONE) {
		bDspEmulated = false;
	} else {
		bDspEmulated = true;
	}
	Statusbar_SetDspLed(false);
	dsp_txdn_intr = 0;

	dsp_core_reset();
	save_cycles = 0;
#endif
}


/**
 * Enable DSP memory
 */
void DSP_EnableMemory(void)
{
#if ENABLE_DSP_EMU
	if (ConfigureParams.System.bDSPMemoryExpansion) {
		dsp_core_config_ramext(dsp_ram, DSP_RAMSIZE_96kB);
	} else {
		dsp_core_config_ramext(dsp_ram, DSP_RAMSIZE_24kB);
	}
#endif
}


/**
 * Disable DSP memory
 */
void DSP_DisableMemory(void)
{
#if ENABLE_DSP_EMU
	dsp_core_config_ramext(NULL, 0);
#endif
}


/**
 * Start the DSP emulation
 */
void DSP_Start(uint8_t mode)
{
	if (!bDspEmulated) {
		return;
	}
#if ENABLE_DSP_EMU
	if (ConfigureParams.System.nDSPType==DSP_TYPE_ACCURATE) {
		dsp_core_start(mode, 1);
	} else if (ConfigureParams.System.nDSPType==DSP_TYPE_EMU) {
		dsp_core_start(mode, 0);
	}
	save_cycles = 0;
#endif
}

/**
 * Run DSP for certain cycles
 */
void DSP_Run(int nHostCycles)
{
#if ENABLE_DSP_EMU
	save_cycles += nHostCycles * 2;
	
	while (save_cycles > 0)
	{
		dsp56k_execute_instruction();
		save_cycles -= dsp_core.instr_cycle;
	}
	
	DSP_HandleDMA();
#endif
}

/**
 * Enable/disable DSP debugging mode
 */
void DSP_SetDebugging(bool enabled)
{
	bDspDebugging = enabled;
}

/**
 * Get DSP program counter (for debugging)
 */
uint16_t DSP_GetPC(void)
{
#if ENABLE_DSP_EMU
	if (bDspEnabled)
		return dsp_core.pc;
	else
#endif
	return 0;
}

/**
 * Get next DSP PC without output (for debugging)
 */
uint16_t DSP_GetNextPC(uint16_t pc)
{
#if ENABLE_DSP_EMU
	/* code is reduced copy from dsp56k_execute_one_disasm_instruction() */
	dsp_core_t dsp_core_save;
	uint16_t instruction_length;

	if (!bDspEnabled)
		return 0;

	/* Save DSP context */
	memcpy(&dsp_core_save, &dsp_core, sizeof(dsp_core));

	/* Disasm instruction */
	dsp_core.pc = pc;
	/* why dsp56k_execute_one_disasm_instruction() does "-1"
	 * for this value, that doesn't seem right???
	 */
	instruction_length = dsp56k_disasm(DSP_DISASM_MODE, stderr);

	/* Restore DSP context */
	memcpy(&dsp_core, &dsp_core_save, sizeof(dsp_core));

	return pc + instruction_length;
#else
	return 0;
#endif
}

/**
 * Get current DSP instruction cycles (for profiling)
 */
uint16_t DSP_GetInstrCycles(void)
{
#if ENABLE_DSP_EMU
	if (bDspEnabled)
		return dsp_core.instr_cycle;
	else
#endif
	return 0;
}


/**
 * Disassemble DSP code between given addresses, return next PC address
 */
uint16_t DSP_DisasmAddress(FILE *out, uint16_t lowerAdr, uint16_t UpperAdr)
{
#if ENABLE_DSP_EMU
	uint16_t dsp_pc;

	for (dsp_pc=lowerAdr; dsp_pc<=UpperAdr; dsp_pc++) {
		dsp_pc += dsp56k_execute_one_disasm_instruction(out, dsp_pc);
	}
	return dsp_pc;
#else
	return 0;
#endif
}


/**
 * Get the value from the given (16-bit) DSP memory address / space
 * exactly the same way as in dsp_cpu.c::read_memory() (except for
 * the host/transmit peripheral register values which access has
 * side-effects). Set the mem_str to suitable string for that
 * address / space.
 * Return the value at given address. For valid values AND the return
 * value with BITMASK(24).
 */
uint32_t DSP_ReadMemory(uint16_t address, char space_id, const char **mem_str)
{
#if ENABLE_DSP_EMU
	static const char *spaces[3][4] = {
		{ "X ram", "X rom", "X", "X periph" },
		{ "Y ram", "Y rom", "Y", "Y periph" },
		{ "P ram", "P rom", "P ext memory", "P ext memory" }
	};
	int idx, space;

	switch (space_id) {
	case 'X':
		space = DSP_SPACE_X;
		idx = 0;
		break;
	case 'Y':
		space = DSP_SPACE_Y;
		idx = 1;
		break;
	case 'P':
		space = DSP_SPACE_P;
		idx = 2;
		break;
	default:
		space = DSP_SPACE_X;
		idx = 0;
	}
	address &= 0xFFFF;

	if (space == DSP_SPACE_P) {
		/* bootstrap ROM ? */
		if (dsp_core.mode == 1) {
			*mem_str = spaces[idx][1];
			return dsp_core.rom[DSP_SPACE_P][address & 0x1f];
		}
		/* Internal RAM ? */
		if (address < 0x200) {
			*mem_str = spaces[idx][0];
			return dsp_core.ramint[DSP_SPACE_P][address];
		}
		/* External RAM, mask address to available ram size */
		*mem_str = spaces[idx][2];
		if (dsp_core.ramext) {
			return dsp_core.ramext[address & (DSP_RAMSIZE-1)];
		}
		return 0;
	}

	/* Internal RAM ? */
	if (address < 0x100) {
		*mem_str = spaces[idx][0];
		return dsp_core.ramint[space][address];
	}

	/* Internal ROM ? */
	if (address < 0x200) {
		if (dsp_core.registers[DSP_REG_OMR] & (1<<DSP_OMR_DE)) {
			*mem_str = spaces[idx][1];
			return dsp_core.rom[space][address];
		}
	}

	/* Peripheral address ? */
	if (address >= 0xffc0) {
		*mem_str = spaces[idx][3];
		/* reading host/transmit regs has side-effects,
		 * so just give the memory value.
		 */
		return dsp_core.periph[space][address-0xffc0];
	}

	/* External RAM, map X,Y to P */
	*mem_str = spaces[idx][2];
	if (dsp_core.ramext) {
		/* Access to contiguous or separated space ? */
		if (address&0x8000) {
			/* Map Y to lower half of available RAM size */
			address &= (DSP_RAMSIZE>>1) - 1;
			if (space == DSP_SPACE_X) {
				/* Map X to upper half of available RAM size */
				address |= DSP_RAMSIZE>>1;
			}
		}
		return dsp_core.ramext[address & (DSP_RAMSIZE-1)];
	}
#endif
	return 0;
}


/**
 * Output memory values between given addresses in given DSP address space.
 * Return next DSP address value.
 */
uint16_t DSP_DisasmMemory(FILE *fp, uint16_t dsp_memdump_addr, uint16_t dsp_memdump_upper, char space)
{
#if ENABLE_DSP_EMU
	uint32_t mem, mem2, value;
	const char *mem_str;

	for (mem = dsp_memdump_addr; mem <= dsp_memdump_upper; mem++) {
		/* special printing of host communication/transmit registers */
		if (space == 'X' && mem >= 0xffc0) {
			if (mem == 0xffeb) {
				fprintf(fp, "X periph:%04x  HTX : %06x   RTX:%06x\n",
					mem, dsp_core.dsp_host_htx, dsp_core.dsp_host_rtx);
			}
			else if (mem == 0xffef) {
				fprintf(fp, "X periph:%04x  SSI TX : %06x   SSI RX:%06x\n",
					mem, dsp_core.ssi.transmit_value, dsp_core.ssi.received_value);
			}
			else {
				value = DSP_ReadMemory(mem, space, &mem_str);
				fprintf(fp, "%s:%04x  %06x\t%s\n", mem_str, mem, value, x_ext_memory_addr_name[mem-0xffc0]);
			}
			continue;
		}
		/* special printing of X & Y external RAM values */
		if ((space == 'X' || space == 'Y') && 
		    (mem & 0x8000) &&
		    mem >= 0x200 && mem < 0xffc0) {
			mem2 = mem & ((DSP_RAMSIZE>>1)-1);
			if (space == 'X') {
				mem2 += (DSP_RAMSIZE>>1);
			}
			fprintf(fp, "%c:%04x (P:%04x): %06x\n", space,
				mem, mem2, dsp_core.ramext?dsp_core.ramext[mem2 & (DSP_RAMSIZE-1)]:0);
			continue;
		}
		value = DSP_ReadMemory(mem, space, &mem_str);
		fprintf(fp, "%s:%04x  %06x\n", mem_str, mem, value);
	}
#endif
	return dsp_memdump_upper+1;
}

/**
 * Show information on DSP core state which isn't
 * shown by any of the other commands (dd, dm, dr).
 */
void DSP_Info(FILE *fp, uint32_t dummy)
{
#if ENABLE_DSP_EMU
	int i, j;
	const char *stackname[] = { "SSH", "SSL" };

	fputs("\nDSP core information:\n", fp);

	for (i = 0; i < ARRAY_SIZE(stackname); i++) {
		fprintf(fp, "  %s stack:", stackname[i]);
		for (j = 0; j < ARRAY_SIZE(dsp_core.stack[0]); j++) {
			fprintf(fp, " %04hx", dsp_core.stack[i][j]);
		}
		fputs("\n", fp);
	}

	fprintf(stderr, "\nInterrupts:\n");
	for (i = 0; i < 32; i++) {
		fprintf(stderr, "  %s: ", dsp_interrupt_name[i]);
		if ((1U << i) & dsp_core.interrupt_status & (dsp_core.interrupt_mask|DSP_INTER_NMI_MASK)) {
			fprintf(stderr, "Pending ");
		}
		if ((1U << i) & DSP_INTER_NMI_MASK) {
			fprintf(stderr, "at level 3");
		} else {
			for (j = 2; j>=0; j--) {
				if ((1<<i) & dsp_core.interrupt_mask_level[j]) {
					fprintf(stderr, "at level %i", j);
				}
			}
		}
		fputs("\n", stderr);
	}

	fprintf(fp, "\nHostport:");
	for (i = 0; i < ARRAY_SIZE(dsp_core.hostport); i++) {
		fprintf(fp, " %02x", dsp_core.hostport[i]);
	}
	fputs("\n", fp);
#endif
}

/**
 * Show DSP register contents
 */
void DSP_DisasmRegisters(FILE *fp)
{
#if ENABLE_DSP_EMU
	uint32_t i;
	char stack_disasm[16][20];

	/* Prepare the stack disasm */
	for (i=0; i<16; i++) {
               if ((dsp_core.registers[DSP_REG_SP] & BITMASK(4)) == i)
			sprintf(stack_disasm[i], " >%02d:  %04x   %04x\n", i, dsp_core.stack[0][i], dsp_core.stack[1][i]);
		else
			sprintf(stack_disasm[i], "  %02d:  %04x   %04x\n", i, dsp_core.stack[0][i], dsp_core.stack[1][i]);
	}

	/* Display the DSP registers and stack state */
	fprintf(fp, "\t\t\t\t|  SP   SSH    SSL\n");

	fprintf(fp, "A2: %02x  A1: %06x  A0: %06x\t|%s",
		dsp_core.registers[DSP_REG_A2], dsp_core.registers[DSP_REG_A1], dsp_core.registers[DSP_REG_A0], stack_disasm[0]);
	fprintf(fp, "B2: %02x  B1: %06x  B0: %06x\t|%s",
		dsp_core.registers[DSP_REG_B2], dsp_core.registers[DSP_REG_B1], dsp_core.registers[DSP_REG_B0], stack_disasm[1]);
	fprintf(fp, "        X1: %06x  X0: %06x\t|%s",
		dsp_core.registers[DSP_REG_X1], dsp_core.registers[DSP_REG_X0], stack_disasm[2]);
	fprintf(fp, "        Y1: %06x  Y0: %06x\t|%s",
		dsp_core.registers[DSP_REG_Y1], dsp_core.registers[DSP_REG_Y0], stack_disasm[3]);
	fprintf(fp, "\t\t\t\t|%s", stack_disasm[4]);

	for (i=0; i<8; i++) {
		fprintf(fp, "R%01x: %04x   N%01x: %04x   M%01x: %04x\t|%s",
			i, dsp_core.registers[DSP_REG_R0+i],
			i, dsp_core.registers[DSP_REG_N0+i],
			i, dsp_core.registers[DSP_REG_M0+i],
			stack_disasm[i+5]);
	}

	fprintf(fp, "\t\t\t\t|%s", stack_disasm[13]);

	fprintf(fp, "LA: %04x   LC: %04x   PC: %04x\t|%s",
		dsp_core.registers[DSP_REG_LA], dsp_core.registers[DSP_REG_LC], dsp_core.pc, stack_disasm[14]);
	fprintf(fp, "SR: %04x   OMR: %02x    SP: %02x\t|%s",
		dsp_core.registers[DSP_REG_SR], dsp_core.registers[DSP_REG_OMR], dsp_core.registers[DSP_REG_SP], stack_disasm[15]);

	fprintf(fp, "\n");
#endif
}


/**
 * Get given DSP register address and required bit mask.
 * Works for A0-2, B0-2, LA, LC, M0-7, N0-7, R0-7, X0-1, Y0-1, PC, SR, SP,
 * OMR, SSH & SSL registers, but note that the SP, SSH & SSL registers
 * need special handling (in DSP*SetRegister()) when they are set.
 * Return the register width in bits or zero for an error.
 */
int DSP_GetRegisterAddress(const char *regname, uint32_t **addr, uint32_t *mask)
{
#if ENABLE_DSP_EMU
#define MAX_REGNAME_LEN 4
	typedef struct {
		const char name[MAX_REGNAME_LEN];
		uint32_t *addr;
		size_t bits;
		uint32_t mask;
	} reg_addr_t;

	/* sorted by name so that this can be bisected */
	static const reg_addr_t registers[] = {

		/* 56-bit A register */
		{ "A0",  &dsp_core.registers[DSP_REG_A0],  32, BITMASK(24) },
		{ "A1",  &dsp_core.registers[DSP_REG_A1],  32, BITMASK(24) },
		{ "A2",  &dsp_core.registers[DSP_REG_A2],  32, BITMASK(8) },

		/* 56-bit B register */
		{ "B0",  &dsp_core.registers[DSP_REG_B0],  32, BITMASK(24) },
		{ "B1",  &dsp_core.registers[DSP_REG_B1],  32, BITMASK(24) },
		{ "B2",  &dsp_core.registers[DSP_REG_B2],  32, BITMASK(8) },

		/* 16-bit LA & LC registers */
		{ "LA",  &dsp_core.registers[DSP_REG_LA],  32, BITMASK(16) },
		{ "LC",  &dsp_core.registers[DSP_REG_LC],  32, BITMASK(16) },

		/* 16-bit M registers */
		{ "M0",  &dsp_core.registers[DSP_REG_M0],  32, BITMASK(16) },
		{ "M1",  &dsp_core.registers[DSP_REG_M1],  32, BITMASK(16) },
		{ "M2",  &dsp_core.registers[DSP_REG_M2],  32, BITMASK(16) },
		{ "M3",  &dsp_core.registers[DSP_REG_M3],  32, BITMASK(16) },
		{ "M4",  &dsp_core.registers[DSP_REG_M4],  32, BITMASK(16) },
		{ "M5",  &dsp_core.registers[DSP_REG_M5],  32, BITMASK(16) },
		{ "M6",  &dsp_core.registers[DSP_REG_M6],  32, BITMASK(16) },
		{ "M7",  &dsp_core.registers[DSP_REG_M7],  32, BITMASK(16) },

		/* 16-bit N registers */
		{ "N0",  &dsp_core.registers[DSP_REG_N0],  32, BITMASK(16) },
		{ "N1",  &dsp_core.registers[DSP_REG_N1],  32, BITMASK(16) },
		{ "N2",  &dsp_core.registers[DSP_REG_N2],  32, BITMASK(16) },
		{ "N3",  &dsp_core.registers[DSP_REG_N3],  32, BITMASK(16) },
		{ "N4",  &dsp_core.registers[DSP_REG_N4],  32, BITMASK(16) },
		{ "N5",  &dsp_core.registers[DSP_REG_N5],  32, BITMASK(16) },
		{ "N6",  &dsp_core.registers[DSP_REG_N6],  32, BITMASK(16) },
		{ "N7",  &dsp_core.registers[DSP_REG_N7],  32, BITMASK(16) },

		{ "OMR", &dsp_core.registers[DSP_REG_OMR], 32, 0x5f },

		/* 16-bit program counter */
		{ "PC",  (uint32_t*)(&dsp_core.pc),  16, BITMASK(16) },

		/* 16-bit DSP R (address) registers */
		{ "R0",  &dsp_core.registers[DSP_REG_R0],  32, BITMASK(16) },
		{ "R1",  &dsp_core.registers[DSP_REG_R1],  32, BITMASK(16) },
		{ "R2",  &dsp_core.registers[DSP_REG_R2],  32, BITMASK(16) },
		{ "R3",  &dsp_core.registers[DSP_REG_R3],  32, BITMASK(16) },
		{ "R4",  &dsp_core.registers[DSP_REG_R4],  32, BITMASK(16) },
		{ "R5",  &dsp_core.registers[DSP_REG_R5],  32, BITMASK(16) },
		{ "R6",  &dsp_core.registers[DSP_REG_R6],  32, BITMASK(16) },
		{ "R7",  &dsp_core.registers[DSP_REG_R7],  32, BITMASK(16) },

		{ "SP",  &dsp_core.registers[DSP_REG_SP],  32, BITMASK(6) },

		/* 16-bit status register */
		{ "SR",  &dsp_core.registers[DSP_REG_SR],  32, 0xefff },

		{ "SSH", &dsp_core.registers[DSP_REG_SSH], 32, BITMASK(16) },
		{ "SSL", &dsp_core.registers[DSP_REG_SSL], 32, BITMASK(16) },

		/* 48-bit X register */
		{ "X0",  &dsp_core.registers[DSP_REG_X0],  32, BITMASK(24) },
		{ "X1",  &dsp_core.registers[DSP_REG_X1],  32, BITMASK(24) },

		/* 48-bit Y register */
		{ "Y0",  &dsp_core.registers[DSP_REG_Y0],  32, BITMASK(24) },
		{ "Y1",  &dsp_core.registers[DSP_REG_Y1],  32, BITMASK(24) }
	};
	/* left, right, middle, direction */
	int l, r, m, dir = 0;
	unsigned int i, len;
	char reg[MAX_REGNAME_LEN];

	if (!bDspEnabled) {
		return 0;
	}

	for (i = 0; i < sizeof(reg) && regname[i]; i++) {
		reg[i] = toupper((unsigned char)regname[i]);
	}
	if (i < 2 || regname[i]) {
		/* too short or longer than any of the names */
		return 0;
	}
	len = i;

	/* bisect */
	l = 0;
	r = ARRAY_SIZE(registers) - 1;
	do {
		m = (l+r) >> 1;
		for (i = 0; i < len; i++) {
			dir = (int)reg[i] - registers[m].name[i];
			if (dir) {
				break;
			}
		}
		if (dir == 0) {
			*addr = registers[m].addr;
			*mask = registers[m].mask;
			return registers[m].bits;
		}
		if (dir < 0) {
			r = m-1;
		} else {
			l = m+1;
		}
	} while (l <= r);
#undef MAX_REGNAME_LEN
#endif
	return 0;
}


/**
 * Set given DSP register value, return false if unknown register given
 */
bool DSP_Disasm_SetRegister(const char *arg, uint32_t value)
{
#if ENABLE_DSP_EMU
	uint32_t *addr, mask, sp_value;
	int width;

	/* first check registers needing special handling... */
	if (arg[0]=='S' || arg[0]=='s') {
		if (arg[1]=='P' || arg[1]=='p') {
			dsp_core.registers[DSP_REG_SP] = value & BITMASK(6);
			value &= BITMASK(4);
			dsp_core.registers[DSP_REG_SSH] = dsp_core.stack[0][value];
			dsp_core.registers[DSP_REG_SSL] = dsp_core.stack[1][value];
			return true;
		}
		if (arg[1]=='S' || arg[1]=='s') {
			sp_value = dsp_core.registers[DSP_REG_SP] & BITMASK(4);
			if (arg[2]=='H' || arg[2]=='h') {
				if (sp_value == 0) {
					dsp_core.registers[DSP_REG_SSH] = 0;
					dsp_core.stack[0][sp_value] = 0;
				} else {
					dsp_core.registers[DSP_REG_SSH] = value & BITMASK(16);
					dsp_core.stack[0][sp_value] = value & BITMASK(16);
				}
				return true;
			}
			if (arg[2]=='L' || arg[2]=='l') {
				if (sp_value == 0) {
					dsp_core.registers[DSP_REG_SSL] = 0;
					dsp_core.stack[1][sp_value] = 0;
				} else {
					dsp_core.registers[DSP_REG_SSL] = value & BITMASK(16);
					dsp_core.stack[1][sp_value] = value & BITMASK(16);
				}
				return true;
			}
		}
	}

	/* ...then registers where address & mask are enough */
	width = DSP_GetRegisterAddress(arg, &addr, &mask);
	switch (width) {
	case 32:
		*addr = value & mask;
		return true;
	case 16:
		*(uint16_t*)addr = value & mask;
		return true;
	}
#endif
	return false;
}

/**
 * Read SSI transmit value
 */
uint32_t DSP_SsiReadTxValue(void)
{
#if ENABLE_DSP_EMU
	return dsp_core.ssi.transmit_value;
#else
	return 0;
#endif
}

/**
 * Write SSI receive value
 */
void DSP_SsiWriteRxValue(uint32_t value)
{
#if ENABLE_DSP_EMU
	dsp_core.ssi.received_value = value & 0xffffff;
#endif
}

/**
 * Signal SSI clock tick to DSP
 */

void DSP_SsiReceive_SC0(void)
{
#if ENABLE_DSP_EMU
	dsp_core_ssi_Receive_SC0();
#endif
}

void DSP_SsiTransmit_SC0(void)
{
#if ENABLE_DSP_EMU
#endif
}

void DSP_SsiReceive_SC1(uint32_t FrameCounter)
{
#if ENABLE_DSP_EMU
	dsp_core_ssi_Receive_SC1(FrameCounter);
#endif
}

void DSP_SsiTransmit_SC1(void)
{
#if ENABLE_DSP_EMU
#endif
}

void DSP_SsiReceive_SC2(uint32_t FrameCounter)
{
#if ENABLE_DSP_EMU
	dsp_core_ssi_Receive_SC2(FrameCounter);
#endif
}

void DSP_SsiTransmit_SC2(uint32_t frame)
{
#if ENABLE_DSP_EMU
#endif
}

void DSP_SsiReceive_SCK(void)
{
#if ENABLE_DSP_EMU
	dsp_core_ssi_Receive_SCK();
#endif
}

void DSP_SsiTransmit_SCK(void)
{
#if ENABLE_DSP_EMU
#endif
}

/**
 * DSP Host interface port is accessed by the 68k in Byte mode.
 */

/* Register bits */

#define ICR_INIT    0x80
#define ICR_HM1     0x40
#define ICR_HM0     0x20
#define ICR_HF1     0x10
#define ICR_HF0     0x08
#define ICR_TREQ    0x02
#define ICR_RREQ    0x01

#define CVR_HC      0x80
#define CVR_HV      0x1F

#define ISR_HREQ    0x80
#define ISR_DMA     0x40
#define ISR_HF3     0x10
#define ISR_HF2     0x08
#define ISR_TRDY    0x04
#define ISR_TXDE    0x02
#define ISR_RXDF    0x01


void DSP_ICR_Read(void) { /* 0x02008000 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_ICR));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0x7F);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] ICR read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_ICR_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_ICR, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] ICR write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_CVR_Read(void) { /* 0x02008001 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_CVR));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0xFF);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] CVR read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_CVR_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_CVR, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] CVR write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_ISR_Read(void) { /* 0x02008002 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_ISR));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0xFF);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] ISR read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_ISR_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_ISR, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] ISR write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_IVR_Read(void) { /* 0x02008003 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_IVR));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0xFF);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] IVR read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_IVR_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_IVR, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] IVR write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data0_Read(void) { /* 0x02008004 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_TRX0));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0x00);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data0 read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data0_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_TRX0, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data0 write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data1_Read(void) { /* 0x02008005 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_TRXH));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0x00);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data1 read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data1_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_TRXH, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data1 write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data2_Read(void) { /* 0x02008006 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_TRXM));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0x00);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data2 read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data2_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_TRXM, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data2 write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data3_Read(void) { /* 0x02008007 */
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		IoMem_WriteByte(IoAccessCurrentAddress, dsp_core_read_host(CPU_HOST_TRXL));
	else
#endif
		IoMem_WriteByte(IoAccessCurrentAddress, 0x00);
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data3 read at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}

void DSP_Data3_Write(void) {
#if ENABLE_DSP_EMU
	if (bDspEmulated)
		dsp_core_write_host(CPU_HOST_TRXL, IoMem_ReadByte(IoAccessCurrentAddress));
#endif
	Log_Printf(LOG_DSP_REG_LEVEL,"[DSP] Data3 write at $%08x val=$%02x PC=$%08x\n", IoAccessCurrentAddress, IoMem_ReadByte(IoAccessCurrentAddress), m68k_getpc());
}
