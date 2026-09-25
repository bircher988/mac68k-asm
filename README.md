# mac68k-asm - 68k assembler toolchain for the classic Macintosh

`mac68k-asm` builds applications for the first Macs (Mac 128K/512K/Plus, 64K ROM,
System 3.x) from 68000 assembler sources - on Linux or macOS, without an emulator.
One small C program contains the assembler, the linker, the resource compiler and a
MacBinary writer. The result goes onto a floppy image with `hcopy -m` and runs on real
hardware just as in an emulator.

The source dialect and the project files (`.Job`, `.Link`, `.R`) follow the Macintosh
assembler conventions of the mid-1980s, so projects written for the Macintosh 68000
Development System (MDS) should also work.

## Quick start

```
git clone https://github.com/bircher988/mac68k-asm.git
cd mac68k-asm
make                                  # needs only a C compiler and make
./mac68k-asm build example/Hello.Job -o out
```

`out/Hello.bin` is the finished application as MacBinary. Put it on a floppy disk image
with the companion tool [mac68k-disk](https://github.com/bircher988/mac68k-disk) and boot
the image in an emulator (Mini vMac) or on the real machine:

```
mac68k-disk new Apps.dsk            # 400K MFS, readable by every 68k Mac
mac68k-disk add Apps.dsk out/Hello.bin
```

(hfsutils works too - `hcopy -m out/Hello.bin :Hello` - but it is no longer packaged
in Debian 13.)

## Installation

**Debian / Ubuntu / Raspberry Pi OS.** Every
[release](https://github.com/bircher988/mac68k-asm/releases) has a `.deb` for arm64
(Raspberry Pi) and amd64 (PC). Download it and install it with apt:

```
wget https://github.com/bircher988/mac68k-asm/releases/download/v1.0/mac68k-asm_1.0_arm64.deb
sudo apt install ./mac68k-asm_1.0_arm64.deb      # or _amd64.deb on a PC
```

[mac68k-disk](https://github.com/bircher988/mac68k-disk) puts applications onto disk images.
To build the package yourself: `packaging/debian/build-deb.sh`.

**From source (Linux, macOS, any Unix).** `make` and then `sudo make install` puts the binary into
`/usr/local/bin` and the include files into `/usr/local/share/mac68k-asm`; `make
uninstall` removes them again. `PREFIX=...` changes the location.

In every case `mac68k-asm version` shows what is installed.

## Example

`example/` is a complete, minimal program. It opens a window, draws a line of text
and quits on the next mouse click:

| File | Content |
|---|---|
| `Hello.Asm` | the program: toolbox init, `_GetNewWindow`, `_DrawString`, wait for `_Button`, `_ExitToShell` |
| `Hello.Link` | link specification: output name, file type, list of modules |
| `Hello.R` | resources: `INCLUDE Hello.code` plus one `WIND` template (ID 128) |
| `Hello.Job` | the build recipe (ASM / LINK / RMAKER lines) for `mac68k-asm build` |

The core of `Hello.Asm`:

```
	Include	Traps.D		; toolbox and OS traps
	Include	ToolEqu.D	; toolbox equates (windowSize, ...)

WindID	EQU	128

Start
	PEA	-4(A5)		; QuickDraw globals live just below A5
	_InitGraf
	...
	CLR.L	-(SP)		; FUNCTION GetNewWindow(...): WindowPtr
	MOVE.W	#WindID,-(SP)
	PEA	WindStorage	; DS variable -> assembled as WindStorage(A5)
	MOVE.L	#-1,-(SP)
	_GetNewWindow
	MOVE.L	(SP)+,WindPtr
	...
	PEA	Message		; code label -> assembled PC-relative
	_DrawString

Message	DC.B	MsgEnd-Message-1
	DC.B	'Hello from mac68k-asm. Click to quit.'
MsgEnd

WindPtr	DS.L	1		; DS reserves in the A5 globals area, not in the code
WindStorage	DS.B	windowSize
	END
```

Trap calls follow the Pascal calling convention of the Toolbox: push room for the
result, push the arguments, call the trap, pop the result. Labels defined with `DC`
are read PC-relative and cannot be written to; variables that change go into `DS`.

## Commands

```
mac68k-asm build <File.Job>  [-o outdir] [-I dir]...   run a .Job file (ASM / LINK / RMAKER)
mac68k-asm link  <File.Link> [-o outdir] [-I dir]...   assemble and link -> <Output>.rsrc/.bin/.MAP/.lst
mac68k-asm res   <File.R>    [-o outdir] [-I dir]...   compile a resource file -> <Name>.rsrc/.bin
mac68k-asm asm   <Module.Asm>... [-I dir]... [-o out.raw] [-l listing]
mac68k-asm version | help
```

`-I` adds directories that are searched for modules, includes and resource files
(file names are matched case-insensitively, as on HFS). The built-in include
directory (`inc/`) is found automatically next to the binary or under
`share/mac68k-asm`; `MAC68K_INC` overrides it. It holds `Traps.D` (all Toolbox and
OS traps with their ROM), the equates `SysEqu.D`, `ToolEqu.D`, `QuickEqu.D`,
`FSEqu.D`, `SysErr.D`, `ATalkEqu.Txt` (AppleTalk) and `PrEqu.Txt` (printing), and the
call macros `PackMacs.Txt` (packages) and `SaneMacs.Txt` (SANE floating point), under
the classic names so that existing sources include them unchanged.

Output of a build in `outdir`:

| File | Content |
|---|---|
| `<App>.bin` | the finished application as MacBinary (`hcopy -m` onto an image) |
| `<Output>.bin`, `*.rsrc` | intermediate stages (linker output, resource compiler output) |
| `<Output>.raw` | the bare code of the linked modules |
| `<Output>.MAP` | segment and globals sizes |
| `<Output>.lst` | listing with addresses and bytes |
| `<Output>.ERR` | only on assembler errors |

`build.sh` is a convenience wrapper for a repository layout with one folder per
project (`<repo>/projects/<project>/`, shared sources in `<repo>/shared/`); it can
also put the application onto a copy of a development disk image.

## The assembler

The assembler reads the whole module set, resolves includes once, and iterates until
all addresses are stable. Rules of the dialect:

- **Labels** are addressed PC-relative, **constants** (`$904`) as abs.W when they fit
  in 16 bits, abs.L otherwise. `d(PC)` and `d(PC,Xn)` with a plain number use the
  number as the displacement.
- **Branches**: a `Bcc` to a label not yet seen (further down, or in another module)
  is `.W`, a `BSR`/`BRA` to such a label becomes `JSR`/`JMP d16(PC)`. A known backward
  label gives a short branch when it fits, even with an explicit `.W`, and `BSR.W`/
  `BRA.W` otherwise. An explicit `.S` is short; if it does not reach it is widened
  with a warning; `Bcc.S` to the very next instruction becomes `NOP`. `JSR`/`JMP label`
  stay `JSR`/`JMP`.
- `CMP/ADD/SUB/AND/OR/EOR #imm,<ea>` are encoded as `CMPI/ADDI/...` (`ADDA/SUBA/CMPA`
  with an address register); `ADD/SUB #1..8` become `ADDQ/SUBQ`. `MOVE ea,An` is
  `MOVEA`. Explicit `ADDI`, `ADDA`, `MOVEQ` stay as written; `MOVE.L #5,D0` is not
  turned into `MOVEQ`.
- **`DS.x` does not reserve space in the code** but in the A5 globals area: the first
  DS label sits at `-(256+total)(A5)`, `DS.W`/`DS.L` start on even offsets, the area
  is rounded up to an even size. References become `d16(A5)`, also as destinations.
- `DC.W`/`DC.L` are aligned to even addresses; a label standing alone right before
  them moves along. Instructions at odd addresses get a pad byte, but a label in front
  of them stays odd (and the program crashes with bomb ID=03 - keep data even).
- `#label` and `#variable` are errors (the address is only known at run time; use
  `LEA`); writing to a `DC` label is an error (PC-relative operands are read-only).
- A quoted string as a memory operand (`PEA 'Title'`, `LEA '- Name:',A0`) refers to a
  Pascal string that the assembler places at the end of the module; identical
  strings share one copy.
- Symbols are case-insensitive. Every module has its own namespace; `XDEF` exports a
  symbol, `XREF` is accepted and ignored. `@local` labels are valid between two
  global labels. `Reg EQU A3` defines a register alias. A duplicate `EQU` in one
  module is a warning; the last definition wins.
- `Include`, `.TRAP _Name $Axxx` (OS traps take `,SYS`, `,ASYNC`, `,IMMED`,
  `,CLEAR`), `.MACRO name` / `%1..%9` / `.ENDM`, `IF` / `ELSE` / `ENDIF` with
  `'a' = 'b'`, `'a' <> 'b'` or numeric comparisons, `EQU`, `SET`, `DC`, `DCB`, `DS`,
  `EVEN`, `END`.
- Every `.TRAP` in `inc/Traps.D` carries the ROM that introduced it (`64K` or
  `128K`). A program is assumed to target the 64K ROM (Macintosh 128K/512K); using a
  128K-ROM trap (Macintosh Plus, 512Ke) produces a warning. `MAC68K_ROM=128` declares
  the program a Plus program and turns the warnings off.

Diagnostics name the file and line of the original source, also inside includes.

## MDS projects

Existing MDS projects should also build: `mac68k-asm build` reads their `.Job` files,
and the include files carry the same names as the original ones. Where the original
tools produced code that cannot work (for example a `DS` variable used before its
definition), mac68k-asm produces working code instead.

## Licence

mac68k-asm is MIT-licensed (see `LICENSE`), including the files in `inc/`: they are
generated from the trap numbers and equate values documented in Inside Macintosh and
contain nothing but those names and values, our own headers and macros.

## Not implemented

- `/Segment` and `/Start` in .Link (several CODE segments with a jump table).
- The HFS macros of `Traps.txt` beyond what the `IF` support covers.

## Tests

`make test` builds the example. With `MAC68K_TEST_PROJECTS` (a directory with one
folder per project) and `MAC68K_TEST_REF` (the same tree with reference builds, as
MacBinary) it rebuilds every job and compares file type, creator and every resource
with `tests/rescmp.py`.
