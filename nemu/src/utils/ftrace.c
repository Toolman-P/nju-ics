#include <elf.h>
#include <stdio.h>
#include <utils.h>
#include <common.h>
#include <malloc.h>

#define ElfN(type) concat(MUXDEF(CONFIG_ISA64,Elf64_,Elf32_),type)
#define ELFN(type) concat(MUXDEF(CONFIG_ISA64,ELF64_,ELF32_),type)

static sym_t *sym_start, *sym_end;
bool is_ftraceopen = false;

static void init_sympool(){
    sym_start = malloc(sizeof(sym_t));
    sym_start->start = sym_start->end = 0;
    sym_start->name = NULL;
    sym_start->next = NULL;
    sym_end = sym_start;
}

static void add_symbol(char *strtab,ElfN(Sym) *sym){
    sym_t *s = sym_end;
    s->next = malloc(sizeof(sym_t));
    s=s->next;

    s->name = malloc(strlen(strtab+sym->st_name)+1);
    strcpy(s->name,strtab+sym->st_name);
    s->start = sym->st_value;
    s->end = sym->st_value + sym->st_size;
    s->next = NULL;
    sym_end = s;
}

char *search_symbol(word_t pc){
    for(sym_t *s=sym_start;s;s=s->next){
        if(pc>=s->start&&pc<s->end){
            return s->name;
        }
    }
    return NULL;
}

int init_ftrace(const char *elf_file){

    if(!elf_file)
      return -1;
    
    ElfN(Ehdr) ehdr;
    ElfN(Shdr) *shdr,*sh_strtab_hdr;
    ElfN(Sym) *symtab;

    uint16_t shnum,shentsize,sym_num;
    char *sh_strtab,*strtab;

    init_sympool();

    FILE *fd = fopen(elf_file,"r");
    
    fread(&ehdr,sizeof(ehdr),1,fd);
    shnum = ehdr.e_shnum;
    shentsize = ehdr.e_shentsize;

    fseek(fd,ehdr.e_shoff,SEEK_SET);
    
    shdr = malloc(shnum*shentsize);
    fread(shdr,shentsize,shnum,fd);

    sh_strtab_hdr = shdr+ehdr.e_shstrndx;
    fseek(fd,sh_strtab_hdr->sh_offset,SEEK_SET);
    sh_strtab = malloc(sh_strtab_hdr->sh_size);
    fread(sh_strtab,sh_strtab_hdr->sh_size,1,fd);

    for(uint16_t i=0;i<shnum;i++){
        char *shdr_name = sh_strtab+shdr[i].sh_name;
        if(strcmp(shdr_name,".symtab")==0){
            fseek(fd,shdr[i].sh_offset,SEEK_SET);
            symtab = malloc(shdr[i].sh_size);
            sym_num = shdr[i].sh_size / shdr[i].sh_entsize;
            fread(symtab,shdr[i].sh_size,1,fd);
        }else if(strcmp(shdr_name,".strtab")==0){
            fseek(fd,shdr[i].sh_offset,SEEK_SET);
            strtab = malloc(shdr[i].sh_size);
            fread(strtab,shdr[i].sh_size,1,fd);
        }
    }

    for(uint16_t i=0;i<sym_num;i++)
        if(ELFN(ST_TYPE)(symtab[i].st_info) == STT_FUNC)
            add_symbol(strtab,symtab+i);
    
    free(shdr);
    free(sh_strtab);
    free(strtab);
    free(symtab);

    is_ftraceopen = true;
    
    return 0;
}
