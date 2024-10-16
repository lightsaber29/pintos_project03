/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/vaddr.h"

struct list frame_table;

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
	// frame_table init
	list_init(&frame_table);
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
// pending 중인 페이지 객체를 초기화하고 생성합니다.
// 페이지를 생성하려면 직접 생성하지 말고 이 함수나 vm_alloc_page를 통해 만드세요.
// init과 aux는 첫 page fault가 발생할 때 호출된다.
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	// ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {

		/* TODO: Create the page, fetch the initialier according to the VM type,*/
		// 페이지를 생성하고,
		struct page *p = (struct page *)malloc(sizeof(struct page));
		// VM 유형에 따라 초기화 함수를 가져와서
		bool (*page_initializer)(struct page *, enum vm_type, void *);

		switch (VM_TYPE(type)) {
			case VM_ANON:
				page_initializer = anon_initializer;
				break;
			case VM_FILE:
				page_initializer = file_backed_initializer;
				break;
			default:
				// uninit_initialize
				break;
		}
		/* TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		// uninit_new를 호출해 "uninit" 페이지 구조체를 생성하세요.
		uninit_new(p, upage, init, type, aux, page_initializer);

		// uninit_new를 호출한 후에는 필드를 수정해야 합니다.
		p->writable = writable;

		/* TODO: Insert the page into the spt. */
		return spt_insert_page(spt, p);
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
// 인자로 받은 vaddr에 해당하는 vm_entry 를 검색 후 반환
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	struct page *page;
	struct hash_elem *found_elem;
	/* TODO: Fill this function. */
	/* pg_round_down()으로 vaddr의 페이지 번호를 얻음 */
	// 가상 메모리 주소에 해당하는 페이지 번호 추출
	// #define pg_round_down(va) (void *) ((uint64_t) (va) & ~PGMASK)
	page = malloc(sizeof(struct page));
	// pg_round_down: 페이지 크기에 맞게 내림(round down) 하여 페이지의 시작 주소를 구하는 매크로
	// 이 함수가 호출될 때 들어오는 va는 모두 page의 시작 주소이다 -> pg_round_down 주석처리
	page->va = pg_round_down(va);
	// page->va = va;

	// 모르는거!!
	// hash_find 에 어떻게 va를 전달할 지
	// 어떻게 va를 hash_elem 형태로 바꿔서 전달할 지 모르겠따

	// va와 hash_elem 의 관계?


	/* hash_find() 함수를 사용해서 hash_elem 구조체 얻음 */
	// vm_entry 검색 후 반환
	found_elem = hash_find(&spt->page_table, &page->hash_elem);
	/* 만약 존재하지 않는다면 NULL 리턴 */
	/* hash_entry()로 해당 hash_elem의 vm_entry 구조체 리턴 */

	if (found_elem != NULL) {
		// vm_entry 에서 page 를 어떻게 구할까?
		return hash_entry(found_elem, struct page, hash_elem);
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
	if (hash_insert(&spt->page_table, &page->hash_elem) == NULL) {
		insert_succ = true;
	}
	return insert_succ;
}

// 이거 리턴타입 void 에서 bool 로 바꿈
void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	// vm_dealloc_page (page);
	// int remove_succ = false;
	// if (hash_delete(&spt->page_table, page) != NULL) {
	// 	remove_succ = true;
	// }
	// return remove_succ;

	hash_delete(&spt->page_table, &page->hash_elem);
	vm_dealloc_page (page);
	return true;
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
	// palloc_get_page 함수를 호출하여 사용자 풀에서 새로운 physical page(frame)를 가져온다
	void *kva = palloc_get_page(PAL_USER);

	// 페이지 할당을 실패할 경우, PANIC ("todo")로 표시한다. (swap out을 구현한 이후 변경한다.)
	if (kva == NULL) {
		PANIC("todo");
	}

	// 사용자 풀에서 페이지를 성공적으로 가져오면, 프레임을 할당하고 해당 프레임의 멤버를 초기화한 후 반환한다.
	frame = malloc(sizeof(struct frame));
	frame->kva = kva;

	// 리스트에 추가
	list_push_back(&frame_table, &frame->frame_elem);
	frame->page = NULL;

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

	if (addr == NULL)
		return false;

	if (is_kernel_vaddr(addr))
		return false;

	struct thread *curr = thread_current();
	uint64_t *curr_stack_bottom = (uintptr_t *)curr->stack_bottom;
	//printf("Fault at addr: %p, rsp: %p, stack bottom: %p\n", addr, f->rsp, (uintptr_t *)curr_stack_bottom);

	// 만약 커널 모드에서 발생한 page fault일 경우 page fault가 발생한 주소를 직접 설정해줘야 함
	//addr = curr->tf.rsp;

	// stack overflow 검증
		if (f->rsp - (uintptr_t)addr <= USER_STACK - *curr_stack_bottom) {
        // 유효한 스택 확장 시도일 경우 스택을 확장
        // 스택 페이지를 할당하는 로직
    } else {
        // 스택 오버플로우로 간주
        return false;
    }

	// if (addr < curr_stack_bottom) {
	// 		if (f->rsp - (uintptr_t)addr <= USER_STACK - *curr_stack_bottom) {
  //       // 유효한 스택 확장 시도일 경우 스택을 확장
  //       // 스택 페이지를 할당하는 로직
  //   } else {
  //       // 스택 오버플로우로 간주
  //       return false;
  //   }
	// }

	/* TODO: Your code goes here */
	// 접근한 메모리의 physical page가 존재하지 않은 경우
	if (not_present) {
		page = spt_find_page(spt, addr);
		if (page == NULL)
			return false;

		// write 불가능한 페이지에 write 요청한 경우
		if (write == 1 && page->writable == 0)
			return false;
		return vm_do_claim_page(page);
	}
	return false;
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
    // spt에서 va에 해당하는 page 찾기
    page = spt_find_page(&thread_current()->spt, va);
    if (page == NULL)
        return false;

	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	if (!page || !is_user_vaddr(page->va)) {
		return false;
	}

	struct frame *frame = vm_get_frame ();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	// 가상 주소와 물리 주소 매핑
	struct thread *curr = thread_current();
	pml4_set_page(curr->pml4, page->va, frame->kva, page->writable);

	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	// hash_init(초기화할구조체, 해시함수포인터, 비교함수포인터, 사용자정의데이터)
	hash_init(&spt->page_table, vm_entry_hash, vm_entry_less, NULL);
}

uint64_t
vm_entry_hash (const struct hash_elem *e, void *aux UNUSED) {
    struct page *page = hash_entry(e, struct page, hash_elem);
    return hash_bytes(&page->va, sizeof(page->va));  // vaddr을 해시
}

bool
vm_entry_less (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
    struct page *page_a = hash_entry(a, struct page, hash_elem);
    struct page *page_b = hash_entry(b, struct page, hash_elem);

    /* 가상 주소가 더 작은 항목을 우선순위가 높다고 판단 */
    return page_a->va < page_b->va;
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
	struct supplemental_page_table *src UNUSED) {
}

void action_func(struct hash_elem *e, void *aux) {
	struct page *page = hash_entry(e, struct page, hash_elem);
	destroy(page);
	free(page);
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	// hash_destroy(&spt->page_table, hash_delete);
	hash_clear(&spt->page_table, action_func);
}
