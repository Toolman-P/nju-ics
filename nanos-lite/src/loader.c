#include <proc.h>
#include <memory.h>

#include <elf.h>
#include <fs.h>

#define OFFSET_MASK 0xFFFUL

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

void *elf_page = NULL;

size_t ramdisk_read(void *buf, size_t offset, size_t len);

uintptr_t loader(PCB *pcb, const char *filename)
{
  Elf_Ehdr ehdr;
  Elf_Phdr phdr[10];
  void *va,*pa,*pa_start;
  Area va_area;
#ifdef __LP64__
  uint64_t vaddr,memsz,filesz,offset;
#else
  uint32_t vaddr,memsz,filesz,offset;
#endif

  assert(filename);
  
  int fd;
  if((fd = open(filename)) == -1)
    return 0;

  read(fd,&ehdr,sizeof(ehdr));
  lseek(fd,ehdr.e_phoff,SEEK_SET);
  read(fd,&phdr,ehdr.e_phnum*ehdr.e_phentsize);
  
  for(uint16_t i=0;i<ehdr.e_phnum;i++){
    if(phdr[i].p_type == PT_LOAD){

      vaddr = phdr[i].p_vaddr;
      memsz = phdr[i].p_memsz;
      filesz = phdr[i].p_filesz;
      offset = phdr[i].p_offset;
      
      
      va_area = (Area){(void *)(ROUNDDOWN(vaddr,PGSIZE)),(void *)ROUNDUP(vaddr+memsz,PGSIZE)};
      pa_start = pg_alloc(va_area.end-va_area.start);
      
      
      for(va=va_area.start,pa=pa_start;va<va_area.end;va+=PGSIZE,pa+=PGSIZE)
        map(&pcb->as,va,pa,7);
      
      elf_page = va-PGSIZE;
      pa = pa_start+(vaddr&OFFSET_MASK);

      lseek(fd,offset,SEEK_SET);
      read(fd,pa,filesz);
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

