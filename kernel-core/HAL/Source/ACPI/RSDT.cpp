/**
 * @file RSDT.cpp
 * -------------------------------------------------------------------
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#include <ACPI/RSDP.h>
#include <ACPI/RSDT.h>
#include <ACPI/XSDT.h>
#include <Memory/Pager.h>
#include <Memory/KMemorySpace.h>
#include <KERNEL.h>
#include <Utils/Memory.h>

using namespace ACPI;

RSDT *SystemRsdt;
XSDT *SystemXsdt;

static VirtualRSDT *loadedTables;

const char *sysRSDTNotFound = "RSDT NOT FOUND";

///
///
/// This function establishes the virtual-RSDT inplace by mapping all the ACPI
/// tables into specific 4KB blocks. Virtual addresses of these tables are
/// placed into a global virtual-RSDT which is used later to search for other
/// tables by their name.
///
/// Origin:
/// This was implemented to save time to search for a table. Re-mapping of all
/// tables was avoided by pre-mapping all of them at once.
///
/// @version 1.0
/// @since Silcos 2.05
/// @author Shukant Pal
///
static void formVirtualRSDT()
{
	loadedTables = (VirtualRSDT*) (stcConfigBlock * KPGSIZE);
	Pager::use((unsigned long) loadedTables, KF_NOINTR, KernelData);

	++(stcConfigBlock);

	loadedTables->physTable = SystemRsdt;
	loadedTables->matrixBase = stcConfigBlock * 4096;
	loadedTables->stdTableCount = SystemRsdt->entryCount();

	PhysAddr stdTablePAddr;
	for(unsigned long stdTableIndex = 0;
			stdTableIndex < loadedTables->stdTableCount;
			stdTableIndex++)
	{
		stdTablePAddr = (PhysAddr) SystemRsdt->ConfigurationTables[stdTableIndex];
		Pager::map(stcConfigBlock * 4096, stdTablePAddr &
				0xFFFFF000, KF_NOINTR,
				KernelData);

		loadedTables->stdTableAddr[stdTableIndex] =
				(stcConfigBlock << KPGOFFSET) +
				(unsigned long) stdTablePAddr % KPGSIZE;

		++(stcConfigBlock);
	}
}

/**
 * Function: SetupRSDTHolder
 *
 * Summary:
 * Initializes all data structs that depend directly upon the RSDT. That also
 * includes the global virtual-RSDT.
 *
 * Version: 1.02
 * Since: Circuit 2.03
 * Author: Shukant Pal
 */
void SetupRSDTHolder()
{
	if(SystemRsdp->Revision == 0) {
		Pager::map(stcConfigBlock * 4096,
				SystemRsdp -> RsdtAddress & 0xfffff000,
				KF_NOINTR, 3);

		RSDT *Rsdt = (RSDT *) (stcConfigBlock * 4096 + (SystemRsdp -> RsdtAddress % 4096));
		SystemRsdt = Rsdt;
		++stcConfigBlock;

		formVirtualRSDT();

		if(!VerifySdtChecksum(&(SystemRsdt -> RootHeader))) {
			DbgLine(sysRSDTNotFound);
			asm volatile("cli; hlt;");
		}
	}
}

/**
 * Function: SearchACPITableByName
 *
 * Summary:
 * Searches for a ACPI table in the virtual-RSDT by its name. As per specs,
 * it only compares first 4 bytes of the given string.
 *
 * Args:
 * const char *tblSign - signature of the table
 *
 * Changes:
 * # Uses the virtual-RSDT instead of mapping all tables every time
 *
 * Author: Shukant Pal
 */
void *SearchACPITableByName(const char *tblSign, SDTHeader *stdTable)
{
	unsigned long tableIdx;
	SDTHeader *thisTable;

	if(stdTable != null) {
		tableIdx = (((unsigned) stdTable & KPGOFFSET)
				- loadedTables->matrixBase) >> KPGOFFSET;
	} else {
		tableIdx = 0;
	}

	while(tableIdx < loadedTables->stdTableCount) {
		thisTable = (SDTHeader*) loadedTables->stdTableAddr[tableIdx];
		if(memcmp(thisTable, tblSign, 4))
			return ((void*) thisTable);
		++(tableIdx);
	}

	return (null);
}
