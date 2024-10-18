/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"

#include "threads/vaddr.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);

bool lazy_load_segment (struct page *page, void *aux);
static bool install_page (void *upage, void *kpage, bool writable);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
		struct file_page *file_page UNUSED = &page->file;
		// dirty한 경우
		if (pml4_is_dirty(thread_current()->pml4, page->va)) {
				file_write_at(file_page->file, page->va, file_page->read_bytes, file_page->ofs);
				pml4_set_dirty(thread_current()->pml4, page->va, false);
		}
		file_close (file_page->file);
		if (page->frame) {
				list_remove(&page->frame->frame_elem);
				page->frame = NULL;
				page->frame->page = NULL;
				free(page->frame);
		}
}

/* Do the mmap */
/* mmap을 위한 do_mmap 구현 */
void *
do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset) {
    // 파일을 reopen하여 새로운 파일 핸들 사용
    file = file_reopen(file); 
    if (file == NULL) return NULL;

    // 페이지 수 계산
    size_t page_count = (offset + length) / PGSIZE + 1;
    off_t ofs = offset;
    void *start_addr = addr;

    for (size_t i = 0; i < page_count; i++) {
        // 파일 페이지 정보 생성
        struct file_page *file_info = (struct file_page *) malloc(sizeof(struct file_page));
        if (file_info == NULL) return NULL;

        file_info->file = file;
        file_info->ofs = ofs;
        file_info->read_bytes = (i == page_count - 1) ? length % PGSIZE : PGSIZE;
        file_info->zero_bytes = PGSIZE - file_info->read_bytes;

        // 페이지를 lazy loading할 수 있도록 vm_alloc_page_with_initializer 호출
        if (!vm_alloc_page_with_initializer(VM_FILE, addr, writable, lazy_load_segment, file_info)) {
            free(file_info);
            return NULL;
        }

        // 다음 페이지로 이동
        addr += PGSIZE;
        ofs += PGSIZE;
    }

    return start_addr;
}

/* 페이지 폴트 시 파일에서 데이터를 읽어오는 함수 */
bool
lazy_load_segment (struct page *page, void *aux) {
    struct file_page *file_info = (struct file_page *) aux;

    // 페이지를 위한 물리 메모리 할당
    void *kpage = palloc_get_page(PAL_USER);
    if (kpage == NULL)
        return false;

    // 파일에서 데이터를 읽어와 페이지에 로드
    file_seek(file_info->file, file_info->ofs);
    if (file_read(file_info->file, kpage, file_info->read_bytes) != (int) file_info->read_bytes) {
        palloc_free_page(kpage);
        return false;
    }

    // 남은 바이트를 0으로 채움
    memset(kpage + file_info->read_bytes, 0, file_info->zero_bytes);

    // 페이지를 프로세스의 페이지 테이블에 매핑
    if (!install_page(page->va, kpage, page->writable)) {
        palloc_free_page(kpage);
        return false;
    }

    return true;
}

static bool
install_page (void *upage, void *kpage, bool writable) {
	struct thread *t = thread_current ();

	/* Verify that there's not already a page at that virtual
	 * address, then map our page there. */
	return (pml4_get_page (t->pml4, upage) == NULL
			&& pml4_set_page (t->pml4, upage, kpage, writable));
}

/* Do the munmap */
void
do_munmap (void *addr) {
 		struct thread *curr_thread = thread_current();
    struct supplemental_page_table *curr_spt = &curr_thread->spt;
    struct page *page;
    // addr을 통해서 매핑된 모든 페이지를 처리할 때까지 루프
    while (page = spt_find_page(curr_spt, addr)) {
        file_backed_destroy(page);
        pml4_clear_page(curr_thread->pml4, page->va);
        spt_remove_page(curr_spt, page);
        free(page);
        addr += PGSIZE;
    }
}
