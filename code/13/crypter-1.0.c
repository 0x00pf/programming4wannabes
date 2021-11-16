#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <elf.h>

#define DIE(s) {perror(s);exit(1);}
#define SWAP(a,b) a += b; b= a -b; a-=b;

int rc4 (unsigned char *msg, int mlen, unsigned char *key, int klen) {
  int           i,j;
  unsigned char S[256]; // Permutation matrix
  
  // KSA: Key-Schedulling Algorithm
  for (i = 0; i < 255; S[i] = i,i++);   
  for (j = 0, i = 0; i < 256; i++) {
    j = (j + S[i] + key[i % klen] ) % 256;
    SWAP(S[i],S[j]);
  }
  // Encoding
  i = j = 0;
  int cnt = 0;
  while (cnt < mlen) {
    i = (i + 1) % 256;
    j = (j + S[i]) % 256;
    
    SWAP(S[i],S[j]);
    
    msg[cnt] = msg[cnt] ^ S[(S[i] + S[j]) % 256];
    cnt++;
  }
  printf (" [%d bytes encoded]", cnt);
  return 0;
}

int main (int argc, char *argv[]) {
  
  if (argc != 2) {
    fprintf (stderr, "Invalid number of parameters\n");
    fprintf (stderr, "Usage: crypter binary\n");
    exit (-1);
  }
  // Open file
  int fd;
  if ((fd = open (argv[1], O_RDWR, 0)) < 0) DIE ("open");
  // get size
  struct stat _st;
  if (fstat (fd, &_st) < 0) DIE ("fstat");
  // Map file
  unsigned char *p;
  if ((p = mmap (0, _st.st_size, PROT_READ | PROT_WRITE,
		 MAP_SHARED, fd, 0)) == MAP_FAILED) DIE ("mmap");
  // Find code segment
  Elf64_Ehdr *elf_hdr = (Elf64_Ehdr*) p;
  // Sanity checks oimitted
  printf ("Section Table located at : %ld\n", elf_hdr->e_shoff);
  printf ("Section Table entry size : %d\n",  elf_hdr->e_shentsize);
  printf ("Section Table entries    : %d\n",  elf_hdr->e_shnum);  

  int           i;
  Elf64_Shdr    *sh = (Elf64_Shdr*)(p + elf_hdr->e_shoff) ;
  //Elf64_Shdr    *sh_strtab = sh + elf_hdr->e_shstrndx;
  //char *s_name = p + sh_strtab->sh_offset;
  char *s_name = p + sh[elf_hdr->e_shstrndx].sh_offset;
  
  char *key ="0x00Sec!\0";  // Use 8 characters to make asm simpler.
  char *name = NULL;

  for (i = 0; i < elf_hdr->e_shnum; i++) {
    name = s_name + sh[i].sh_name;
    printf ("Section %02d [%20s]: Type: %d Flags: %lx | Off: %lx Size: %lx => ",
	    i, name,
	    sh[i].sh_type, sh[i].sh_flags,
	    sh[i].sh_offset, sh[i].sh_size);
    //Find `.text` and `.rodata`
    if (!strcmp (name, ".text") || !strcmp (name, ".rodata")) {
      // encrypt section
      rc4 (p + sh[i].sh_offset, sh[i].sh_size, (unsigned char*)key, strlen (key));
      printf (" - Crypter!\n");
    }     else printf ("\n");
    
    
  }

  // Inject stub here
  munmap (p, _st.st_size); 
  close (fd);
  return 0;
}
