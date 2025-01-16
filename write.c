#include <err.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 0x48 = 64-bit operands
// 0xc7 = MOV
// 0xc0..0xcf = register num
// 0x0f 0x05 = Fast System call
uint8_t text[] = {
    0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov %rax $60
    0x48, 0xc7, 0xc7, 0x45, 0x00, 0x00, 0x00, // mov %rdi $69
    0x0f, 0x05                                // syscall
};

char shstrtab[] = {
    '\0',                                                 // NULL (offset 0)
    '.',  't', 'e', 'x', 't', '\0',                       // .text (offset 1)
    '.',  's', 'h', 's', 't', 'r',  't', 'a',  'b', '\0', // .shstrtab (offset 6)
    '.',  's', 'y', 'm', 't', 'a',  'b', '\0',            // .symtab (offset 16)
    '.',  's', 't', 'r', 't', 'a',  'b', '\0',            // .strtab (offset 24)
};

char strtab[] = {
    '\0',                               // NULL (offset 0)
    '_',  's', 't', 'a', 'r', 't', '\0' // _start (offset 1)
};

int main(int argc, char **argv) {
  int fd;
  Elf *e;
  Elf64_Ehdr *ehdr;
  Elf64_Shdr *shdr;
  Elf_Scn *scn;
  Elf_Data *data;
  size_t textscn_index;
  size_t strtabscn_index;

  if (argc != 2)
    errx(EXIT_FAILURE, "usage: %s [file]", argv[0]);
  if (elf_version(EV_CURRENT) == EV_NONE)
    errx(EXIT_FAILURE, "elf lib init failed: %s", elf_errmsg(-1));
  if ((fd = open(argv[1], O_WRONLY | O_CREAT, 0755)) < 0)
    errx(EXIT_FAILURE, "failed to open '%s'", argv[1]);
  if ((e = elf_begin(fd, ELF_C_WRITE, NULL)) == NULL)
    errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));

  if ((ehdr = elf64_newehdr(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newehdr() failed: %s", elf_errmsg(-1));

  ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
  ehdr->e_machine = EM_X86_64;
  ehdr->e_type = ET_REL;

  // Create .text
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  data->d_align = 8;
  data->d_off = 0LL;
  data->d_type = ELF_T_BYTE;
  data->d_buf = text;
  data->d_size = sizeof(text);
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_getshdr() failed: %s", elf_errmsg(-1));

  textscn_index = elf_ndxscn(scn);

  shdr->sh_name = 1;
  shdr->sh_type = SHT_PROGBITS;
  shdr->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
  shdr->sh_entsize = 0;

  // Create .strtab
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  strtabscn_index = elf_ndxscn(scn);

  data->d_align = 1;
  data->d_buf = strtab;
  data->d_off = 0LL;
  data->d_size = sizeof(strtab);
  data->d_type = ELF_T_BYTE;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf64_getshdr() failed: %s", elf_errmsg(-1));

  shdr->sh_name = 25;
  shdr->sh_type = SHT_STRTAB;
  shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
  shdr->sh_entsize = 0;

  // Create .symtab
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  Elf64_Sym symtab[2] = { 0 };
  Elf64_Sym sym = {
      .st_name = 1,
      .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_SECTION),
      .st_other = STV_DEFAULT,
      .st_shndx = textscn_index,
      .st_value = 0, // Offset into section for objects
      .st_size = sizeof(text),
  };
  symtab[1] = sym;

  data->d_align = 8;
  data->d_buf = (void*)&symtab;
  data->d_off = 0LL;
  data->d_size = sizeof(symtab);
  data->d_type = ELF_T_SYM;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf64_getshdr() failed: %s", elf_errmsg(-1));

  shdr->sh_name = 17;
  shdr->sh_type = SHT_SYMTAB;
  shdr->sh_flags = SHF_ALLOC;
  shdr->sh_entsize = sizeof(Elf64_Sym);
  shdr->sh_link = strtabscn_index;
  shdr->sh_info = 1; // # of symbols

  // Create .shstrtab
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  data->d_align = 1;
  data->d_buf = shstrtab;
  data->d_off = 0LL;
  data->d_size = sizeof(shstrtab);
  data->d_type = ELF_T_BYTE;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf64_getshdr() failed: %s", elf_errmsg(-1));

  shdr->sh_name = 7;
  shdr->sh_type = SHT_STRTAB;
  shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
  shdr->sh_entsize = 0;

  ehdr->e_shstrndx = elf_ndxscn(scn);


  // Write ELF

  /* if (elf_update(e, ELF_C_NULL) < 0) */
  /*   errx(EXIT_FAILURE, "elf_update(NULL) failed: %s", elf_errmsg(-1)); */

  if (elf_update(e, ELF_C_WRITE) < 0)
    errx(EXIT_FAILURE, "elf_update() failed: %s", elf_errmsg(-1));

  elf_end(e);
  close(fd);

  return 0;
}
