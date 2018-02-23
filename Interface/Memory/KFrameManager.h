///
/// @file KFrameManager.h
/// -------------------------------------------------------------------
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>
///
/// Copyright (C) 2017 - Shukant Pal
///
#ifndef MEMORY_KFRAME_MANAGER_H
#define MEMORY_KFRAME_MANAGER_H

#include "Internal/BuddyAllocator.hpp"
#include "Internal/ZoneAllocator.hpp"
#include "KMemorySpace.h"

#ifdef x86
	#include <IA32/PageTrans.h>
#endif

extern PADDRESS mmLow;
extern PADDRESS mmHigh;
extern PADDRESS mmTotal;
extern PADDRESS mmUsable;
extern unsigned long pgTotal;
extern unsigned long pgUsable;

#define FRSIZE_ORDER 12

typedef struct Memory::Internal::BuddyBlock MMFRAME;

//! Alias for the frame for the given physical address
#define FRAME_AT(pAddress)(MMFRAME*)(KFRAMEMAP + \
		(sizeof(MMFRAME) * ((unsigned long) (pAddress) >> 12)))

//! Alias for the page-frame at the given page-offset in memory
#define FROPAGE(pgOffset)(KFRAMEMAP + pgOffset * sizeof(MMFRAME))

//! Alias for the physical-address for the page-frame
#define FRADDRESS(fAddress)(PADDRESS)(((unsigned long)fAddress - KFRAMEMAP) \
					/ sizeof(MMFRAME)) * KB(4)

void TypifyMRegion(unsigned long typeValue, unsigned long regionAddress,
		unsigned long regionSize) kxhide;

#ifdef NAMESPACE_MAIN
	void SetupKFrameManager(void) kxhide;
#endif

#define ZONE_DMA	0
#define ZONE_DRIVER	1
#define ZONE_CODE	2
#define ZONE_DATA	3
#define ZONE_KERNEL	4

//! Offset of bit of KF_NOINTR flag
#define OFFSET_NOINTR 31

//! Interrupts are not turned on if this flag is set
#define KF_NOINTR (1 << 31)

PADDRESS KeFrameAllocate(unsigned long fOrder, unsigned long prefZone, unsigned long fFlags);
unsigned long KeFrameFree(PADDRESS frameAddress);

//! Atomic allocation; for use in the kernel of one page-frame
#define KiFrameAllocate() KeFrameAllocate(0, ZONE_KERNEL, FLG_ATOMIC)

//! Freeing a page-frame; for use in kernel code only
#define KiFrameFree(fAddress) KeFrameFree(fAddress)

//! Entraps a page-frame atomically without using the page-frame cache
#define KiFrameEntrap(frFlags) KeFrameAllocate(0, ZONE_KERNEL, \
		frFlags | FLG_ATOMIC | FLG_NOCACHE)

#endif/* Memory/KFrameManager.h */
