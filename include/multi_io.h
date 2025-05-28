#ifndef MULTI_IO_H
#define MULTI_IO_H

// 최대 I/O 작업 수 정의
#define MAX_IO_OPERATIONS 3

// I/O 작업 구조체
typedef struct {
    int io_start; // I/O가 시작되는 CPU 실행 시점
    int io_burst; // I/O 지속 시간
} IOOperation;

#endif
