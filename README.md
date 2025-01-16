# ELF Fun (🧝?)

I am working on a [compiler](https://github.com/costowell/dumlang) and wanted
to write the object file from scratch instead of using assembly as an intermediary language.

This is the result of messing around while following a [tutorial for libelf](https://atakua.org/old-wp/wp-content/uploads/2015/03/libelf-by-example-20100112.pdf).

## Structure

There are 4 programs that I've written, each more useful than the last!

`phdr` - Prints all the Program Headers of an ELF
  - Not useful to my end goal since relocatable ELFs don't have Program Headers, but still useful to know.

`shdr` - Prints all the Section Headers + some data associated of an ELF
  - Will print the symbol table and the section header string table data too
  - Some other random bits will get printed too because I was debugging my generated binary which brings me too...

`write` - Writes an object file/relocatable ELF with a small amount of assembly
  - This is the thing I was after
  - Program generated exits with unique status code to make sure its running
  - Linkable with `ld`

`prog` - Prints the symbols of a file
  - Made this so that I could debug why I couldn't print symbols in my program
  - Turned it into a fun exercise in manually indexing the strtab buffer to map the symbol names instead of using `elf_strptr()`
  
## Compiling

``` sh
make
```

All binaries are in the `out/` folder.

All objects are in the `obj/` folder.

## Compatability

[This commit](https://github.com/costowell/elf-fun/commit/f8b2bd3ec527b6744cf4f12f632fb748c894b91c) fixed the problem with certain linkers not working.

Thank god to LLVM's lld which told me what I was doing wrong.

### Tested Linkers

- [ld](https://www.gnu.org/software/binutils/) ✅
- [lld](https://github.com/llvm/llvm-project) ✅
- [mold](https://github.com/rui314/mold) ✅
- Maybe 🫵 YOU could find a linker that fails...

## Thanks to...

- Joseph Koshy's [libelf by Example](https://atakua.org/old-wp/wp-content/uploads/2015/03/libelf-by-example-20100112.pdf)
- [libelf](https://sourceware.org/elfutils/)
- OSDev Wiki's [ELF Tutorial](https://wiki.osdev.org/ELF_Tutorial)
- k3170's article [Introduction to the ELF Format (Part VI) : The Symbol Table and Relocations (Part 1)](https://blog.k3170makan.com/2018/10/introduction-to-elf-format-part-vi.html)
- And many random forums I stumbled across while hacking this together
