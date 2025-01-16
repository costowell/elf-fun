#include <bsd/vis.h>
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LIBBSD_OPENBSD_VIS 1

int main(int argc, char **argv) {
  int fd;
  Elf *e;
  Elf_Scn *scn;
  Elf_Data *data;
  GElf_Shdr shdr;
  GElf_Sym sym;
  size_t n, shstrndx, sz;
  char *name, *p, pc[4];

  if (argc != 2)
    errx(EXIT_FAILURE, "usage: %s [file]", argv[0]);
  if (elf_version(EV_CURRENT) == EV_NONE)
    errx(EXIT_FAILURE, "ELF lib init failed: %s", elf_errmsg(-1));
  if ((fd = open(argv[1], O_RDONLY)) < 0)
    errx(EXIT_FAILURE, "failed to open '%s'", argv[1]);
  if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
    errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));
  if (elf_kind(e) != ELF_K_ELF)
    errx(EXIT_FAILURE, "%s is not an ELF object", argv[1]);
  if (elf_getshdrstrndx(e, &shstrndx))
    errx(EXIT_FAILURE, "elf_getshdrstrndx() failed: %s", elf_errmsg(-1));

  scn = NULL;

  putchar('\n');

  while ((scn = elf_nextscn(e, scn)) != NULL) {
    if (gelf_getshdr(scn, &shdr) != &shdr)
      errx(EXIT_FAILURE, "getshdr() failed: %s", elf_errmsg(-1));
    if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL)
      errx(EXIT_FAILURE, "elf_strptr() failed: %s", elf_errmsg(-1));
    printf("%-4.4jd %s\n", (uintmax_t)elf_ndxscn(scn), name);
    if (shdr.sh_type == SHT_SYMTAB && (data = elf_getdata(scn, NULL)) != NULL) {
      printf("flags: %jx %jd %x\n", shdr.sh_flags, data->d_align, shdr.sh_info);
      printf("SYMBOL TABLE:\n");
      int count = shdr.sh_size / shdr.sh_entsize;
      for (int i = 0; i < count; ++i) {
        GElf_Sym sym;
        gelf_getsym(data, i, &sym);
        printf("--- %d ---\n", i);
        printf("st_name: %d (%s)\n", sym.st_name, elf_strptr(e, shdr.sh_link, sym.st_name));
        printf("st_info: %d (bind) %d (type)\n", ELF64_ST_BIND(sym.st_info), ELF64_ST_TYPE(sym.st_info));
        printf("st_value: %jd\n", sym.st_value);
        printf("st_size: %jd\n", sym.st_size);
        printf("st_other: %d\n", sym.st_other);
        printf("st_shndx: %d\n", sym.st_shndx);
        printf("-----------------\n");
      }
      putchar('\n');
    }
  }
  if ((scn = elf_getscn(e, shstrndx)) == NULL)
    errx(EXIT_FAILURE, "getscn() failed: %s", elf_errmsg(-1));
  if (gelf_getshdr(scn, &shdr) != &shdr)
    errx(EXIT_FAILURE, "getshdr(shstrndex) failed: %s", elf_errmsg(-1));
  printf(".shstrab: size=%jd\n", (uintmax_t)shdr.sh_size);

  data = NULL;
  n = 0;
  while (n < shdr.sh_size && (data = elf_getdata(scn, data)) != NULL) {
    p = (char *)data->d_buf;
    while (p < (char *)data->d_buf + data->d_size) {
      if (vis(pc, *p, VIS_WHITE, 0))
        printf("%s", pc);
      n++;
      p++;
      /* if (n % 16 == 0) */
      /*   putchar('\n'); */
      // putchar((n % 16) ? ' ' : '\n');
    }
  }
  putchar('\n');
  elf_end(e);
  close(fd);
  return 0;
}
