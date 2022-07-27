#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Addr Elf64_Addr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Addr Elf32_Addr
#endif

extern uint8_t ramdisk_start;

size_t ramdisk_read(void *buf, size_t offset, size_t len);

uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  Elf_Phdr phdr[10];

#ifdef __LP64__
  uint64_t vaddr,memsz,filesz,offset;
#else
  uint32_t vaddr,memsz,filesz,offset;
#endif

  assert(filename);

  int fd = open(filename);
  read(fd,&ehdr,sizeof(ehdr));
  lseek(fd,ehdr.e_phoff,SEEK_SET);
  read(fd,&phdr,ehdr.e_phnum*ehdr.e_phentsize);

  for(uint16_t i=0;i<ehdr.e_phnum;i++){
    if(phdr[i].p_type == PT_LOAD){
      vaddr = phdr[i].p_vaddr;
      memsz = phdr[i].p_memsz;
      filesz = phdr[i].p_filesz;
      offset = phdr[i].p_offset;
      memset((char *)vaddr,0,memsz);
      lseek(fd,offset,SEEK_SET);
      read(fd,(void *)vaddr,filesz);
    }
  }
  close(fd);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  assert(entry!=0);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

