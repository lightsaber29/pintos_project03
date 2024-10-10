/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
// 인자로 받은 vaddr에 해당하는 vm_entry 를 검색 후 반환
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct vm_entry entry;
	struct page *page = NULL;
	struct hash_elem *found_elem;
	/* TODO: Fill this function. */
	/* pg_round_down()으로 vaddr의 페이지 번호를 얻음 */
	// 가상 메모리 주소에 해당하는 페이지 번호 추출
	// #define pg_round_down(va) (void *) ((uint64_t) (va) & ~PGMASK)
	entry.vaddr = pg_round_down(va);

	// 모르는거!!
	// hash_find 에 어떻게 va를 전달할 지
	// 어떻게 va를 hash_elem 형태로 바꿔서 전달할 지 모르겠따

	// va와 hash_elem 의 관계?


	/* hash_find() 함수를 사용해서 hash_elem 구조체 얻음 */
	// vm_entry 검색 후 반환
	found_elem = hash_find(&spt->page_table, &entry.elem);
	/* 만약 존재하지 않는다면 NULL 리턴 */
	/* hash_entry()로 해당 hash_elem의 vm_entry 구조체 리턴 */

	if (found_elem != NULL) {
		// vm_entry 에서 page 를 어떻게 구할까?
		return hash_entry(found_elem, struct vm_entry, elem);
	} else {
		return NULL;
	}
	// return page;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int insert_succ = false;
	/* TODO: Fill this function. */
	if (hash_insert(&spt->page_table, page) == NULL) {
		insert_succ = true;
	}
	return insert_succ;
}

// 이거 리턴타입 void 에서 bool 로 바꿈
bool
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	int remove_succ = false;
	if (hash_delete(&spt->page_table, page) != NULL) {
		remove_succ = true;
	}
	return remove_succ;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = NULL;
	/* TODO: Fill this function. */

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */

	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	// hash_init(초기화할구조체, 해시함수포인터, 비교함수포인터, 사용자정의데이터)
	hash_init(&spt->page_table, vm_entry_hash, vm_entry_less, NULL);
}

uint64_t
vm_entry_hash (const struct hash_elem *e, void *aux) {
    struct vm_entry *entry = hash_entry(e, struct vm_entry, elem);
    return hash_bytes(&entry->vaddr, sizeof(entry->vaddr));  // vaddr을 해시
}

bool
vm_entry_less (const struct hash_elem *a, const struct hash_elem *b, void *aux) {
    struct vm_entry *vme_a = hash_entry(a, struct vm_entry, elem);
    struct vm_entry *vme_b = hash_entry(b, struct vm_entry, elem);

    /* 가상 주소가 더 작은 항목을 우선순위가 높다고 판단 */
    return vme_a->vaddr < vme_b->vaddr;
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
	struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	hash_destroy(&spt->page_table, hash_delete);
}
