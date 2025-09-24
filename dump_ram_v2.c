/*  NOTE: 
 *  This code is written specifically for x86_64 Linux.
 *  Any attempts to run it on other platforms unchanged, may and very likely
 *  will lead to undefined behavior.
 *
 *  Written by Mark H. in 2025.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

/* NOTE: Returns NULL on error */
void *mmap_anon_mem(size_t length, bool ask_huge_pages);

int main(void) {
    size_t length = 0;
    bool ask_huge_pages;
    char buf[128], *end;

    printf("Welcome to small memory mapping test utility!\n\n");
    printf("It will map X bytes of anonymous memory \n(either with or without advice for huge pages), and then fault all pages.\n\n");

    /* Read length */
    while(1) {
        printf("Write length in BYTES to allocate (will by rounded to page size *by kernel): ");
        if (!fgets(buf, sizeof(buf), stdin)) return EXIT_FAILURE;
        
        errno = 0;
        unsigned long long tmp_length;

        /* NOTE: Won't throw error if compiled for x86_64 but is running on 32 bit system */
        if (sizeof(tmp_length) != 8 || sizeof(void*) != 8) return EXIT_FAILURE;

        if (buf[0] == '-' || buf[0] == '0') {
            printf("Input cant be lead by zero or be negative! Try again.\n");
            continue;
        }

        tmp_length = strtoull(buf, &end, 10);

        if (errno == ERANGE || end == buf || (*end != '\n' && *end != '\0') || tmp_length > SIZE_MAX) {
            printf("Invalid input! try again.\n");
            continue;
        }
        length = (size_t)tmp_length;
        break;
    }

    /* Read bool */
    while(1) {
        printf("Ask for huge pages? (0=no / 1=yes): ");
        if (!fgets(buf, sizeof(buf), stdin)) return EXIT_FAILURE;

        if (buf[0] == '0' && (buf[1] == '\n' || buf[1] == '\0')) {
            ask_huge_pages = false; break;
        }
    
        if (buf[0] == '1' && (buf[1] == '\n' || buf[1] == '\0')) {
            ask_huge_pages = true; break;
        }
        printf("Invalid input! Please enter 1 or 0.\n");
    }

    void *ptr = mmap_anon_mem(length, ask_huge_pages);
    if (!ptr) {
        fprintf(stderr, "Allocation failed!\n");
        return EXIT_FAILURE;
    }
    printf("Allocated %zu bytes at %p\n", length, ptr);


    printf("Press 'Enter' to munmap() and exit...\n");
    int ch;
    do {
        ch = getchar();
        if (ch == EOF) { // closed stdin
            munmap(ptr, length);
            return EXIT_FAILURE;
        }
    } while (ch != '\n');

    munmap(ptr, length);
    return EXIT_SUCCESS;
}

void *mmap_anon_mem(size_t length, bool ask_huge_pages) {
    void *addr = mmap(NULL,
         length,
         PROT_READ | PROT_WRITE,
         MAP_PRIVATE | MAP_ANONYMOUS,
         -1,
         0);
    
    if (addr == MAP_FAILED) {
        perror("mmap()");
        return NULL;
    }

    if (ask_huge_pages) {
        printf("Asking for THP...\n");
        if (madvise(addr, length, MADV_HUGEPAGE) != 0) {
            perror("madvise()");
            munmap(addr, length);
            return NULL;
        }

        /* We will try to fault at 2MB steps first, to higher 
        chances of getting transparent huge pages from kernel,
        then walk again with 4KB steps to guarantee faulting */
        size_t fault_stride_step = (2*1024*1024); // 2MB
        for (size_t a = 0; a < length; a += fault_stride_step) {
            ((volatile char*)addr)[a] = 0;
        }
    }
    /* 4KB walk */
    for (size_t i = 0; i < length; i += 4096) {
        ((volatile char*)addr)[i] = 0;
    }

    return addr;
}
