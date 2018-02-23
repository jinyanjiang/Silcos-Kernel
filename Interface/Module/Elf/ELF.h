///
/// @file ELF.h
///
/// Contains all the data structures required by the ELF ABI to conform
/// to it.
///
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
#ifndef MODULE_ELF_H
#define MODULE_ELF_H

#include <TYPE.h>
#include <Utils/LinkedList.h>

namespace Module
{

struct ModuleRecord;

namespace Elf
{

#define EI_NIDENT 16

#ifdef ARCH_32
	typedef U32 ELF32_ADDR;
	typedef U16 ELF32_HALF;
	typedef U32 ELF32_OFF;
	typedef S32 ELF32_SWORD;
	typedef U32 ELF32_WORD;
#else
	// Not checked yet
	typedef U64 ELF64_ADDR;
	typedef U32 ELF64_HALF;
	typedef U64 ELF64_OFF;
	typedef S64 ELF64_SWORD;
	typedef U64 ELF32_WORD;
#endif

struct ElfHeader;
struct ElfProgramHeader;
struct ElfSectionHeader;

enum ElfClass
{
	ELFCLASSNONE=0,// Invalid Class
	ELFCLASS32 = 1,// 32-bit Objects
	ELFCLASS64 = 2 // 64-bit Objects
};

enum ElfData {
	ELFDATANONE = 0,// Invalid Data Encoding
	ELFDATA2LSB = 1,// 2's complement values, with the least significant byte occupying the lowest address
	ELFDATA2MSB = 2 // 2's complement values, with the most significant byte occupying the lowest address
};

enum ElfType {
	ET_NONE		= 0,// No file type
	ET_REL		= 1,// Relocatable file
	ET_EXEC		= 2,// Executable file
	ET_DYN		= 3,// Shared-object file
	ET_CORE		= 4,// Core file
	ET_LOPROC	= 0xFF00,// Processor-specific
	ET_HIPROC	= 0xFFFF// Processor-specific
};

enum ElfMachine {
	EM_NONE		= 0,// No machine
	EM_M32  	= 1,// AT&T WE 32100
	EM_SPARC	= 2,// SPARC
	EM_386 		= 3,// Intel 80386
	EM_68K 		= 4,// Motorola 68000
	EM_88K		= 5,// Motorola 88000
	EM_860		= 7,// Intel 80860
	EM_MIPS		= 8// MIPS RS3000
};

#ifdef IA32
	#define EM_RUNNER EM_386
#endif

enum ElfVersion {
	EV_NONE	= 0, // Invalid Version
	EV_CURRENT = 1 // Current Version
};

/**
 * Type: ELF_EHDR
 *
 * Summary:
 * The ELF header is represented semantically in this type. It contains all the
 * information to load a KMT_ELF module. The kernel uses this to parse and fill
 * relocation tables in the binary and dynamically link kmodules to core-kernel
 * & other kmodule-dependencies (iff present).
 *
 * @See Executable and Linkable Format
 * @Version 1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
struct ElfHeader
{
	#define EI_MAG0		0 // File Identification
	#define EI_MAG1		1 // File Identification
	#define EI_MAG2		2 // File Identification
	#define EI_MAG3		3 // File Identification
	#define EI_CLASS	4 // File Class
	#define EI_DATA		5 // Data Encoding
	#define EI_VERSION	6 // File Version
	#define EI_PAD		7 // Start of padding bytes
	#define EI_NIDENT	16// Size of Identifier[]
	
	/* EI_MAG0 to EI_MAG3 */
	#define ELFMAG0		0x7F// EI_MAG0
	#define ELFMAG1		'E' // EI_MAG1
	#define ELFMAG2		'L' // EI_MAG2
	#define ELFMAG3		'F' // EI_MAG3

	unsigned char fileIdentifier[EI_NIDENT];// Identify the file as an object-file & provide machine-independent data
	ELF32_HALF fileType;// Identifies the object-file type
	ELF32_HALF platformRequired;// Required architecture for and individual file
	ELF32_WORD buildVersion;// Object-file version
	ELF32_ADDR entryAddress;// Virtual address to which the system first transfers control; if not present, 0
	ELF32_OFF programHeaderOffset;// Program Header Table's file offset, in bytes
	ELF32_OFF sectionHeaderOffset;// Section Header Table's file offset, in bytes
	ELF32_WORD machineFlags;// Processor-specific flags associated with the file
	ELF32_HALF headerSize;// ELF header's size in bytes
	ELF32_HALF programHeaderEntrySize;// Size of one entry in file's program header table, in bytes
	ELF32_HALF programHeaderEntryCount;// No. of entries in the program header table
	ELF32_HALF sectionHeaderEntrySize;// Size of section header's size in bytes
	ELF32_HALF sectionHeaderEntryCount;// No. of entries in the section header table
	ELF32_HALF sectionStringIndex;// Section header table index of entry associated with section name-table
};

#define PROGRAM_HEADER(eHeader) ((ProgramHeader *)((unsigned long) eHeader \
		+ eHeader->programHeaderOffset))
#define SECTION_HEADER(eHeader) (eHeader->sectionHeaderOffset) ? \
		((SectionHeader *)((unsigned long) eHeader + \
				eHeader->sectionHeaderOffset)) : NULL

/* Special Section Indexes */
enum SectionIndex
{
	SHN_UNDEF 		= 0, // Undefined, missing, irrelevant or otherwise meaningless section
	SHN_LORESERVE	= 0xFF00, // Lower bound of the range of reserved indexes
	SHN_LOPROC		= 0xFF00, // <Values in this inclusive range are reserved for
	SHN_HIPROC		= 0xFF1F, // processor-specific semantics>
	SHN_ABS			= 0xFFF1, // Absolute values for the corresponding reference
	SHN_COMMON		= 0xFFF2, // Relative to this section are common symbols
	SHN_HIRESERVCE	= 0xFFFF, // Upper bound of the range of reserved indexes
};

/* ELF Section Types */
enum SectionType
{
	SHT_NULL	= 0,
	SHT_PROGBITS	= 1,
	SHT_SYMTAB	= 2,
	SHT_STRTAB	= 3,
	SHT_RELA	= 4,
	SHT_HASH	= 5,
	SHT_DYNAMIC	= 6,
	SHT_NOTE	= 7,
	SHT_NOBITS	= 8,
	SHT_REL		= 9,
	SHT_SHLIB	= 10,
	SHT_DYNSYM	= 11,
	SHT_LOPROC	= 0x70000000,
	SHT_HIPROC	= 0x7FFFFFFF,
	SHT_LOUSER	= 0x80000000,
	SHT_HIUSER	= 0xFFFFFFFF
};

/* Section Flags */
enum SectionFlag
{
	SHF_WRITE		= 0x1, // Section contains data that should be writable during process execution
	SHF_ALLOC		= 0x2, // Section occupies memory during process exeuction
	SHF_EXECINSTR	= 0x4, // Executuable machine instructions
	SHF_MASKPROC	= 0xF0000000 // Reserved for processor-specific semantics
};

struct SectionHeader
{
	ELF32_WORD Name;/* Index into the section header table section (null-terminated) */
	ELF32_WORD Type;/* Section's content and semantics */
	ELF32_WORD Flags;/* Support 1-bit flags that describe miscellaneous attributes */
	ELF32_ADDR Address;/* Address of section's first byte */
	ELF32_OFF Offset;/* Byte offset from the beginning of the file to the first byte in the section */
	ELF32_WORD Size;/* Section's size in bytes */
	ELF32_WORD Link;/* Section header table index link */
	ELF32_WORD Info;/* Holds extra information */
	ELF32_WORD AddressAlign;/* Address alignment constraints */
	ELF32_WORD EntrySize;/* For sections holding entries of fixed-size of this value */
};

//--------------------------------ELF Symbol-----------------------------------

#define STN_UNDEF			0
#define ELF32_ST_BIND(value) ((value) >> 4)

enum SymbolBind
{
	STB_LOCAL	 	= 0, // Local symbols, not visible outside the object file
	STB_GLOBAL 		= 1, // Global symbols, visible outside the object files
	STB_WEAK   		= 2, // Represent global symbols of lower precedence
	STB_LOPROC 		= 13,// Values in range STB_LOPROC
	STB_HIPROC 		= 15 // to STB_HIPROC are reserved for processor-specific semantics
};

#define ELF32_ST_TYPE(value) ((value) & 0xF)

enum SymbolType
{
	STT_NOTYPE 		= 0,// Symbol's type is not specified
	STT_OBJECT 		= 1,// Symbol is associated with a data object
	STT_FUNC   		= 2,// Symbol is associated with a function or other executable code
	STT_SECTION		= 3,// Symbol is associated with a section, probably for relocation
	STT_FILE   		= 4,// Symbol's name gives the name of the source file associated
	STT_LOPROC 		= 13,
	STT_HIPROC 		= 15
};

#define ELF32_ST_INFO(b, t) (((b) << 4) + ((t)&0xF))

struct Symbol {
	ELF32_WORD Name;/* Index into object file's string, holding string of symbol name (no name if 0) */
	ELF32_ADDR Value;/* Value of associated symbol */
	ELF32_WORD Size;/* Associated size of the symbol in bytes */
	unsigned char Info;/* Specifies the symbol's type and binding attributes */
	unsigned char Other;/* Currently holds 0 */
	ELF32_HALF SectionIndex;/* Relevant section header table index */
};

//------------------------------ELF Relocation---------------------------------
enum RelocationType {
	R_386_NONE	= 0,
	R_386_32	= 1,
	R_386_PC32	= 2,
	R_386_GOT32	= 3,
	R_386_PLT32	= 4,
	R_386_COPY	= 5,
	R_386_GLOB_DAT	= 6,
	R_386_JMP_SLOT	= 7,
	R_386_RELATIVE	= 8,
	R_386_GOTOFF	= 9,
	R_386_GOTPC	= 10
};

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((unsigned char) (i))
#define ELF32_R_INFO(s, t) (((s) << 8) + (unsigned char) (t))

struct RelEntry {
	ELF32_ADDR Offset;
	ELF32_WORD Info;
};

struct RelaEntry
{
	ELF32_ADDR Offset;/* Location at which to apply relocation action */
	ELF32_WORD Info;/* Symbol table index and type of relocation */
	ELF32_SWORD AddEnd;/* Constant addend used to compute the value to be stored into the relocatable field.*/
};

///
/// Describes the values by which segments/program-header can be
/// described by the ProgramHeader::entryType field.
///
enum PhdrType
{
	PT_NULL		= 0,//!< PT_NULL - unused entry

	PT_LOAD		= 1,//!< PT_LOAD - specifies loadable segment

	PT_DYNAMIC	= 2,//!< PT_DYNAMIC - specifies dynamic linking info

	PT_INTERP	= 3,//!< PT_INTERP - specifies location and size of
			    //! null terminated path to invoke interpreter. Not
			    //! used by this kernel as ModuleLoader is used.
			    
	PT_NOTE		= 4,//!< PT_NOTE - specifies location and size of any
			    //! auxiliary information. ElfAnalyzer may be used
			    //! to access such kind of notes.

	PT_SHLTB	= 5,//!< PT_SHLTB - reserved as of now

	PT_PHDR		= 6,//!< PT_PHDR - specifies location and size of the
			    //! program-header table in file & memory. It is
			    //! optional but only exists singly.

	PT_LOPROC	= 0x70000000,//!< PT_LOPROC - beginning of CPU-reserved
					//! values

	PT_HIPROC	= 0x7FFFFFFF //!< PT_HIPROC - end of CPU-reserved vals
};

///
/// A kernel module's program-headers describe a segment or other
/// information the kernel-host needs to prepare it for
/// running.
///
/// @version 1.0
/// @since Circuit 2.03
/// @author Shukant Pal
///
struct ProgramHeader
{
	ELF32_WORD entryType;//!< Tells what kind of infomation it gives
	ELF32_OFF fileOffset;//!< Gives the offset in the file at which first
			     //!< byte of segment resides
	ELF32_ADDR virtualAddress;//!< Gives the virtual address at which the
				  //! first byte in the segment resides
	ELF32_ADDR physicalAddress;//!< Not relevant in the kernel-context
	ELF32_WORD fileSize;//!< Size of the segment in the file, may be zero
	ELF32_WORD memorySize;//!< Size of the segment in virtual memory, it
			      //! can be zero
	ELF32_WORD flagSet;//!< Contains the flags relevant to this entry
	ELF32_WORD alignBoundary;//!< Values to which segment are aligned in
				 //! memory and the file
};

enum DynamicTag
{
	DT_NULL 	= 0, //! marks end of dynamic array
	DT_NEEDED 	= 1, //! offset of required library's name
	DT_PLTRELSZ	= 2, //! the total size
	DT_PLTGOT	= 3, //! address of GOT
	DT_HASH		= 4, //! address of the symbol hash table
	DT_STRTAB	= 5, //! address of the string table
	DT_SYMTAB	= 6, //! address of the symbol table
	DT_RELA		= 7, //! address of a relocation table
	DT_RELASZ	= 8, //! total size of the DT_RELA relocation table
	DT_RELAENT	= 9, //! size of the DT_RELA relocation entry
	DT_STRSZ	= 10,//! size of the string table
	DT_SYMENT	= 11,//! size of the symbol table entry
	DT_INIT		= 12,//! address of the initialization function
	DT_FINI		= 13,//! address of the termination function
	DT_SONAME	= 14,//! build name of module
	DT_RPATH	= 15,//! presence in .so file resolution algorithm for references within the library
	DT_SYMBOLIC	= 16,//! similar to DT_RELA, has implicit addends
	DT_REL		= 17,//! total size, in bytes, of the DT_REL table
	DT_RELSZ	= 18,//! Element holds the size, in bytes, of the DT_REL relocation entry
	DT_RELENT	= 19,//! Memory specifies the type of relocation entry referred by procedure linkage table
	DT_PLTREL	= 20,//! Member is used for debugging, contents not specified for the ABI, used KAF software only
	DT_DEBUG	= 21,//! absence signifies that no relocation entry causes no change to a non-writable segment
	DT_TEXTREL	= 22,
	DT_JMPREL	= 23,//! pointer to relocation entries for PLT
	DT_LOPROC	= 0x70000000,//! lower-bound for arch-dependent entries
	DT_HIPROC	= 0x7FFFFFFF//! upper-bound for arch-dependent entries
};

typedef
struct DynamicEntry
{
	ELF32_SWORD Tag;
	union {
		ELF32_WORD refValue;
		ELF32_ADDR refPointer;
	};
} ELF32_DYN;

// -- KMod-ELF-Loader CACHE ---

struct SymbolTable
{
	char *nameTable;
	struct Symbol *entryTable;
	unsigned long entryCount;
};

/*
 * Struct: HashTable
 *
 * Summary:
 * Default-hashTable for storing elf-symbols and getting low lookup-delays
 * while querying symbols.
 *
 * Author: Shukant Pal
 */
struct HashTable
{
	struct SectionHeader *hashSectionHdr;/* Section-header, optional (@Deprecated) */
	unsigned long bucketEntries;/* No. of bucket entries */
	unsigned long chainEntries;/* No. of chain entries */
	unsigned long *bucketTable;/* Pointer to bucket table */
	unsigned long *chainTable;/* Pointer to chain table */
};

struct RelaTable
{
	struct RelaEntry *entryTable;
	unsigned long entryCount;
	unsigned long entrySize;
};

struct RelTable
{
	struct RelEntry *entryTable;
	unsigned long entryCount;
	unsigned long entrySize;
};

struct RelocationTable
{
	union
	{
		struct RelaEntry *relaEntries;
		struct RelEntry *relEntries;
		unsigned long tableLocation;
	};
	unsigned long entryCount;
	unsigned long entrySize;
	unsigned long relocType;
};

struct DynamicTable {
	struct ElfDynamicEntry *EntryTable;
	unsigned long EntryCount;
};

struct ProgramCache {
	unsigned long PageCount;/* Total number of pages */
	struct ProgramHeader *Dynamic;
	struct DynamicTable DynamicTable;
};

/**
 * Type: KMOD_ECACHE
 *
 * Summary:
 * Module ELF-Cache contains the direct references to important parts of an ELF
 * kmodule. Without this caching mechanism, the kmodule-loader would experience
 * a overhead due to 'repeated' searches for the required ELF sections, symbol 
 * tables, program headers, dynamic tags, etc. whatsoever.
 *
 * @See ModuleRecord::KMOD_RECORD <KTERM_SYNTAX>
 * @Version 1.1
 * @Since Circuit 2.03
 * @Author Shukant Pal
 */
typedef
struct ElfCache {
	struct SymbolTable dsmTable;/* Dynamic-symbol table */
	struct HashTable dsmHash;/* Dynamic-symbol hash-table (normal hash-table, not GNU-Hash) */
	struct {	
		struct RelaEntry *eRelaTable;
		SIZE eRelaCount;
	};
	struct {
		struct RelEntry *eRelTable;
		SIZE eRelCount;
	};

	struct ProgramCache ProgramHdrCache;
	Void *ePLT;
	Void *eGOT;
	struct Header *eHeader;
} KMOD_ECACHE;

bool MdCheckELFCompat(
		struct ProgramHeader *eHeader
);

#ifdef ARCH_32

void MdLoadELF(
		struct ElfHeader *eHeader,
		struct Module::ModuleRecord *
);

#else // ARCH_64

void MdLoadELF(
		ELF64_EHDR *eHeader
);

#endif

unsigned long
EHashSymbolName(
		char *eSymbolName
);

Symbol *ESearchForSymbol(char *eRequiredSymbolNme, SymbolTable *eSymbolTbl,
					struct HashTable *eHashTbl);
Symbol * ESearchForSymbolName(char *eRequiredSymbolName);

void ESetupLoader(void);

}// namespace Elf

}// namespace Module
#endif/* Module/ELF.h */
