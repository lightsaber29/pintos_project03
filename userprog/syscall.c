#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "filesys/file.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);
void check_address(void *addr);
void get_argument(void *rsp, int *arg, int count);
void halt(void);
void exit(int status);
bool create(const char *file, unsigned initial_size);
bool remove(const char *filename);
int open(const char *filename);
int read (int fd, void *buffer, unsigned size);
/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	uint64_t *sp = f->rsp;
	printf("rsp: %p\n", sp);
	check_address(sp);
	int syscall_number = *sp;
	printf("System call number: %d\n", syscall_number);


	printf ("system call!\n");
	switch(syscall_number){
		case SYS_HALT :	halt(); /* Halt the operating system. */
		case SYS_EXIT : exit(f->R.rdi); /* Terminate this process. */
		// case SYS_FORK : fork(); /* Clone current process. */
		// case SYS_EXEC : exec(); /* Switch current process. */
		// case SYS_WAIT : wait(); /* Wait for a child process to die. */
		case SYS_CREATE : {
			const char *filename = (const char *)f->R.rdi;
			check_address(filename);  // 파일 이름 유효성 검사
			printf("Filename: %s\n", filename);

			unsigned initial_size = (unsigned)f->R.rsi;
			f->R.rax = create(filename, initial_size);
			bool result = create(filename, initial_size);
			printf("Create result: %d\n", result);

		}; /* Create a file. */
		case SYS_REMOVE : {
			char *filename = f->R.rdi;
			remove(filename);
		} /* Delete a file. */
		case SYS_OPEN : open(f->R.rdi);  /* Open a file. */
		// case SYS_FILESIZE : filesize(); /* Obtain a file's size. */
		// case SYS_READ : read(); /* Read from a file. */
		// case SYS_WRITE : write();  /* Write to a file. */
		// case SYS_SEEK : seek(); /* Change position in a file. */
		// case SYS_TELL : tell(); /* Report current position in a file. */
		// case SYS_CLOSE : close(); /* Close a file. */
		default : {
			printf("Invaild system call number. \n");
			exit(-1);
		}
	}
	thread_exit ();
}

void
check_address(void *addr){

if (addr == NULL || !is_kernel_vaddr(addr)) {
	 printf("Invalid address: %p\n", addr);
    exit(-1);
}
}

void 
halt(void){
	printf("Halt called, shutting down...\n");
	power_off();
}

void
exit(int status){
    struct thread *curr = thread_current();
    curr->exit_status = status;
    thread_exit(); // 정상적으로 종료되었으면 0
}

bool
create(const char *filename, unsigned initial_size){
	return filesys_create(filename, initial_size);
}

bool
remove(const char *filename){
	return filesys_remove(filename);
}

int open(const char *filename)
{

/* 파일을 open */
/* 해당 파일 객체에 파일 디스크립터 부여 */
/* 파일 디스크립터 리턴 */
/* 해당 파일이 존재하지 않으면-1 리턴 */

	// 사용자로부터 전달된 filename 포인터가 유효한지 검증
	if (filename == NULL || !is_user_vaddr(filename)) {
			return -1;  // 유효하지 않은 파일 이름일 경우
	}

	// 파일 열기 시도
	struct file *file = filesys_open(filename);
	if (file == NULL) {
			return -1;  // 파일을 열지 못했을 경우
	}

	// 현재 스레드의 파일 디스크립터 테이블에 파일 추가
	struct thread *cur = thread_current();
	int fd = process_add_file(file);
	if (fd == -1) {
			file_close(file);  // 파일 디스크립터 할당에 실패하면 파일을 닫음
	}
    return fd;  // 성공적으로 파일을 열었으면 fd 반환
}

int exec(char *cmd_line){
    // cmd_line이 유효한 사용자 주소인지 확인 -> 잘못된 주소인 경우 종료/예외 발생
    check_address(cmd_line);
    // process.c 파일의 process_create_initd 함수와 유사하다.
    // 단, 스레드를 새로 생성하는 건 fork에서 수행하므로
    // exec는 이미 존재하는 프로세스의 컨텍스트를 교체하는 작업을 하므로
    // 현재 프로세스의 주소 공간을 교체하여 새로운 프로그램을 실행
    // 이 함수에서는 새 스레드를 생성하지 않고 process_exec을 호출한다.
    // process_exec 함수 안에서 filename을 변경해야 하므로
    // 커널 메모리 공간에 cmd_line의 복사본을 만든다.
    // (현재는 const char* 형식이기 때문에 수정할 수 없다.)
    char *cmd_line_copy;
    cmd_line_copy = palloc_get_page(0);
    if (cmd_line_copy == NULL)
        exit(-1);                             // 메모리 할당 실패 시 status -1로 종료한다.
    strlcpy(cmd_line_copy, cmd_line, PGSIZE); // cmd_line을 복사한다.
    // 스레드의 이름을 변경하지 않고 바로 실행한다.
    if (process_exec(cmd_line_copy) == -1)
        exit(-1); // 실패 시 status -1로 종료한다.
}

int read (int fd, void *buffer, unsigned size)
 {
	struct thread *curr = thread_current();
	struct file *file = curr->fd_table[fd];
	int file_bytes;
	if(fd < 0 || fd >= MAX_FD){
		return -1;
	}

	if (fd == 0) {
		for(unsigned i = 0; i < size; i++)
		{
			((uint8_t *)buffer)[i] = input_getc();
		}

		file_bytes = size;
	} else if(fd >= 2){
		lock_acquire(&file_lock);
		file_bytes = (int)file_read(file, buffer, size);
		lock_release(&file_lock);
	}
	//todo fd = 1인경우?
	return file_bytes;
	

 /* 파일에 동시 접근이 일어날 수 있으므로 Lock 사용 */
 /* 파일 디스크립터를 이용하여 파일 객체 검색 */
 /* 파일 디스크립터가 0일 경우 키보드에 입력을 버퍼에 저장 후
버퍼의 저장한 크기를 리턴 (input_getc() 이용) */
 /* 파일 디스크립터가 0이 아닐 경우 파일의 데이터를 크기만큼 저
장 후 읽은 바이트 수를 리턴 */
 }