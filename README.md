# CPU 스케줄링 시뮬레이터

다양한 CPU 스케줄링 알고리즘을 시뮬레이션하고 성능을 비교하는 텀프로젝트

## 디렉터리 구조

- `src/`: 소스 코드 파일들
  - `process.h/c`: 프로세스 구조체 및 관련 함수
  - `queue.h/c`: 스케줄링을 위한 큐 구현
  - `scheduler.h/c`: 다양한 스케줄링 알고리즘 구현
  - `evaluation.h/c`: 성능 평가 및 결과 출력 기능
  - `main.c`: 프로그램 진입점
- `test/`: 결과 파일이 저장되는 디렉터리
- `include/`: 헤더 파일들

## 구현된 스케줄링 알고리즘

1. **FCFS (First-Come, First-Served)**: 도착 순서대로 프로세스 실행
2. **SJF (Shortest Job First)**
   - 비선점형(Non-preemptive): 가장 짧은 실행 시간을 가진 프로세스 먼저 실행
   - 선점형(Preemptive): 더 짧은 실행 시간을 가진 프로세스가 도착하면 선점
3. **Priority Scheduling**
   - 비선점형(Non-preemptive): 우선순위가 높은 프로세스 먼저 실행
   - 선점형(Preemptive): 더 높은 우선순위를 가진 프로세스가 도착하면 선점
4. **Round Robin**: 시간 할당량을 기준으로 프로세스를 순환 실행

## 컴파일 및 실행 방법

### 컴파일 방법

```bash
make
```

### 실행 방법

```bash
./cpu_scheduling
```

### 정리하기

```bash
make clean
```

## 프로그램 사용법

1. 프로그램 실행 시 사용할 스케줄링 알고리즘 선택 가능
2. 프로세스 개수, 도착 시간, CPU 버스트 시간, 우선순위 등은 랜덤 부여

## 출력 결과

1. **간트 차트**: 각 프로세스의 실행 시간을 시각화
2. **성능 지표**:
   - 평균 대기 시간(Average Waiting Time)
   - 평균 반환 시간(Average Turnaround Time)
   - CPU 이용률(CPU Utilization)
   - 처리량(Throughput)
3. **프로세스 상세 정보**: 각 프로세스의 세부 정보를 테이블 형태로 출력
4. **비교 보고서**: 모든 알고리즘의 성능을 비교한 보고서가 `test/scheduling_comparison_report.txt` 파일로 저장됨

## 주의사항

- test 디렉터리가 없을 경우 자동으로 생성되지 않으므로, 프로그램 실행 전 반드시 test 디렉터리 유무 확인 필수
```bash
mkdir -p test
```

