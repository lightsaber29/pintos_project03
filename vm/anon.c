/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */

#include "vm/vm.h"
#include "devices/disk.h"
#include "threads/vaddr.h"

/* DO NOT MODIFY BELOW LINE */
static struct disk *swap_disk;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations anon_ops = {
	.swap_in = anon_swap_in,
	.swap_out = anon_swap_out,
	.destroy = anon_destroy,
	.type = VM_ANON,
};

/* Initialize the data for anonymous pages */
void
vm_anon_init (void) {
	/* TODO: Set up the swap_disk. */
	swap_disk = NULL;
}

/* Initialize the file mapping */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &anon_ops;

	struct anon_page *anon_page = &page->anon;
}

/* Swap in the page by read contents from the swap disk. */
static bool
anon_swap_in (struct page *page, void *kva) {
    struct anon_page *anon_page = &page->anon;
    
    /* Step 1: Get the swap slot number */
    // uint64_t slot_num = anon_page->slot_no;  // 해당 페이지의 스왑 슬롯 번호

    // /* Step 2: Read the data from the swap disk */
    // /* Swap 읽기 작업을 위해 disk_read 함수를 사용합니다. */
    // disk_read(swap_disk, slot_num, kva);  // swap_disk는 스왑 디스크를 나타내는 디스크 객체

    // /* Step 3: Ensure the page is successfully loaded into memory */
    // /* 페이지가 성공적으로 로드되었다고 가정하고 상태를 업데이트 */
    // // page->is_loaded = true;
	// //TODO: page 에 is_loaded 를 선언하지 않았기 때문에 차후 해당 처리에 대해 고려해야함

    // /* Step 4: Optionally clear the swap slot if necessary */
    // /* 스왑 슬롯의 내용은 더 이상 필요하지 않으므로 초기화할 수 있습니다. */
    // anon_page->slot_no = -1;  // 스왑 슬롯 초기화

    // return true;  // 성공적으로 복원되었음을 반환
}

size_t swap_allocate_slot(void) {
    static size_t next_slot = 0; // 스왑 슬롯을 할당하기 위한 인덱스
    static size_t total_slots = DISK_SECTOR_SIZE / PGSIZE; // 전체 스왑 슬롯 수
    size_t allocated_slot;

    // // 스왑 슬롯이 모두 사용 중이면 할당 불가
    // if (next_slot >= total_slots) {
    //     PANIC("No swap slots available!");
    // }

    // // 스왑 슬롯 할당 (next_slot 값을 증가시키며 순차적으로 할당)
    // allocated_slot = next_slot;
    // next_slot++;

    // return allocated_slot;
}

/* Swap out the page by writing contents to the swap disk. */
static bool
anon_swap_out (struct page *page) {
    struct anon_page *anon_page = &page->anon;
    
    // /* Step 1: Allocate a swap slot for the page */
    // uint64_t slot_no = swap_allocate_slot();  // 스왑 슬롯 할당 함수 (가상의 함수)
    // if (slot_no == -1) {
    //     return false;  // 스왑 슬롯 할당 실패
    // }

    // /* Step 2: Write the page data to the swap disk */
    // disk_write(swap_disk, slot_no, page->frame->kva);  // 스왑 디스크에 데이터 저장
    // anon_page->slot_no = slot_no;  // 해당 페이지에 할당된 스왑 슬롯 번호 저장

    // /* Step 3: Free the frame in physical memory */
    // palloc_free_page(page->frame);  // 해당 페이지의 프레임 해제
    // page->frame = NULL;  // 프레임 해제 후 NULL로 설정

    // return true;  // 스왑 아웃 성공
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
static void
anon_destroy (struct page *page) {
    struct anon_page *anon_page = &page->anon;

    // // 1. 스왑 슬롯 해제 (이 경우는 swap_slot이 존재하는 경우)
    // if (anon_page->slot_no != NULL) {
    //     // 해당 스왑 슬롯을 해제합니다.
    //     memset(&anon_page->slot_no, 0, sizeof(anon_page->slot_no));
    //     anon_page->slot_no = NULL; // 스왑 슬롯 번호 초기화
    // }

    // // 2. 페이지와 관련된 자원 해제
    // palloc_free_page(page->frame->kva);  // 물리 메모리에서 할당된 페이지 해제

    // // 3. 페이지의 frame과 관련된 자원 정리
    // page->frame = NULL; // 페이지와 프레임을 연결해제
}
