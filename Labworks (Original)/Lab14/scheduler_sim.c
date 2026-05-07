#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 100
#define MAX_TIME 1000

typedef struct {
    int pid;		// process ID
    int arrival;	// time when process first enter ready queue
    int burst;		// total CPU time needed by the process (CPU execution time)
    int remaining;	// remaining CPU time needed by the process
    int start_time;	// time when process first get CPU
    int finish_time;// time when process finishes 
    int waiting;	// total time waiting for CPU := turnaround - burst
    int turnaround; // time the process stays in the system := finish - start
    int completed;	// process execution is complete
} Process;

/* ================= QUEUE ================= */

typedef struct Node {
    int index;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
} Queue;

void initQueue(Queue* q) {
    q->front = q->rear = NULL;
}

int isEmpty(Queue* q) {
    return q->front == NULL;
}

void enqueue(Queue* q, int index) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->index = index;
    temp->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
}

int dequeue(Queue* q) {
    if (isEmpty(q)) return -1;

    Node* temp = q->front;
    int index = temp->index;

    q->front = temp->next;
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
    return index;
}

/* ============== GANTT LOG ============== */

int gantt[MAX_TIME];
int gantt_size = 0;

void log_execution(int pid) {
    gantt[gantt_size++] = pid;
}

/* ============== SIMULATION ============== */

// Fist Come First serve alg.
//	p points to the array of process structures
//	n is the number of processes in the array
void simulate_fcfs(Process p[], int n) {

    Queue ready;
    initQueue(&ready);

    int time = 0;
    int completed = 0;
    int current = -1;

    while (completed < n) {

        // Check arrival
        for (int i = 0; i < n; i++) {
            if (p[i].arrival == time) {
                enqueue(&ready, i);
            }
        }

        // If CPU is idle
        if (current == -1 && !isEmpty(&ready)) {
            current = dequeue(&ready);

            if (p[current].remaining == p[current].burst)
                p[current].start_time = time;
        }

        // Execute
        if (current != -1) {
            p[current].remaining--;
            log_execution(p[current].pid);

            if (p[current].remaining == 0) {
                p[current].finish_time = time + 1;
                p[current].turnaround = 
                    p[current].finish_time - p[current].arrival;
                p[current].waiting = 
                    p[current].turnaround - p[current].burst;
                p[current].completed = 1;
                completed++;
                current = -1;
            }
        } else {
            log_execution(0); // idle
        }

        time++;
    }
}

/* ============== ROUND ROBIN ============== */

// Round Robin alg.
// Fist Come First serve alg.
//	p points to the array of process structures
//	n is the number of processes in the array
//	quantum is CPU time slice allocated to a process in each turn
void simulate_rr(Process p[], int n, int quantum) {

    Queue ready;
    initQueue(&ready);

    int time = 0;
    int completed = 0;
    int current = -1;
    int q_counter = 0;

    while (completed < n) {

        // arrival
        for (int i = 0; i < n; i++) {
            if (p[i].arrival == time) {
                enqueue(&ready, i);
            }
        }

        if (current == -1 && !isEmpty(&ready)) {
            current = dequeue(&ready);
            q_counter = 0;

            if (p[current].remaining == p[current].burst)
                p[current].start_time = time;
        }

        if (current != -1) {

            p[current].remaining--;
            q_counter++;
            log_execution(p[current].pid);

            if (p[current].remaining == 0) {
                p[current].finish_time = time + 1;
                p[current].turnaround =
                    p[current].finish_time - p[current].arrival;
                p[current].waiting =
                    p[current].turnaround - p[current].burst;

                completed++;
                current = -1;
                q_counter = 0;
            }
            else if (q_counter == quantum) {
                enqueue(&ready, current);
                current = -1;
            }
        }
        else {
            log_execution(0);
        }

        time++;
    }
}

/* ============== PRINT RESULTS ============== */

void print_gantt() {
    printf("\nGantt Chart:\n|");
    for (int i = 0; i < gantt_size; i++) {
        if (gantt[i] == 0)
            printf(" Idle |");
        else
            printf(" P%d |", gantt[i]);
    }
    printf("\n");
}

void print_metrics(Process p[], int n) {

    double avg_wait = 0, avg_turn = 0;

    printf("\nPID\tArrival\tBurst\tWaiting\tTurnaround\n");

    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].arrival,
               p[i].burst,
               p[i].waiting,
               p[i].turnaround);

        avg_wait += p[i].waiting;
        avg_turn += p[i].turnaround;
    }

    printf("\nAverage Waiting Time: %.2f\n", avg_wait / n);
    printf("Average Turnaround Time: %.2f\n", avg_turn / n);
}

/* ================= MAIN ================= */

int main() {

    int n;
    printf("Number of processes: ");
    scanf("%d", &n);

    Process p[MAX_PROCESSES];

    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        printf("\nProcess %d\n", i + 1);
        printf("Arrival: ");
        scanf("%d", &p[i].arrival);
        printf("Burst: ");
        scanf("%d", &p[i].burst);

        p[i].remaining = p[i].burst;
        p[i].completed = 0;
    }

    int choice;
    printf("\n1. FCFS\n2. Round Robin\nChoose: ");
    scanf("%d", &choice);

    if (choice == 1) {
        simulate_fcfs(p, n);
    }
    else {
        int quantum;
        printf("Quantum: ");
        scanf("%d", &quantum);
        simulate_rr(p, n, quantum);
    }

    print_gantt();
    print_metrics(p, n);

    return 0;
}
