
#include <bsd/vis.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void main(int argc, char **argv) {
  Elf *elf;
  Elf_Scn *scn = NULL;
  GElf_Shdr shdr;
  Elf_Data *data;
  int fd, ii, count;

  Elf_Data *strtab_data;

  Elf_Scn *symtab_scn = NULL;
  GElf_Shdr symtab_shdr;

  Elf_Scn *strtab_scn = NULL;
  GElf_Shdr strtab_shdr;

  elf_version(EV_CURRENT);

  fd = open(argv[1], O_RDONLY);
  elf = elf_begin(fd, ELF_C_READ, NULL);

  int flags = 0;

  while ((scn = elf_nextscn(elf, scn)) != NULL) {
    gelf_getshdr(scn, &shdr);
    if (shdr.sh_type == SHT_SYMTAB) {
      flags |= 0b01;
      symtab_scn = scn;
      symtab_shdr = shdr;
    }
    if (shdr.sh_type == SHT_STRTAB) {
      flags |= 0b10;
      strtab_scn = scn;
      strtab_shdr = shdr;
    }
    if (flags == 0b11)
      break;
  }

  if (flags != 0b11) {
    printf("nah\n");
    return;
  }

  strtab_data = elf_getdata(strtab_scn, NULL);
  data = elf_getdata(symtab_scn, NULL);
  count = symtab_shdr.sh_size / symtab_shdr.sh_entsize;
  char *p = (char *)strtab_data->d_buf;
  char pc[4];
  while (p < (char *)strtab_data->d_buf + strtab_data->d_size) {
    if (vis(pc, *p, VIS_WHITE, 0))
      printf("%s", pc);
    p++;
  }
  putchar('\n');

  /* print the symbol names */
  for (ii = 0; ii < count; ++ii) {
    GElf_Sym sym;
    gelf_getsym(data, ii, &sym);
    printf("%s\n", (char *)strtab_data->d_buf + sym.st_name);
  }
  elf_end(elf);
  close(fd);
}
