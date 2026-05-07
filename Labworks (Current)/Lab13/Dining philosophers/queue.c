/*	Queue of messages from philosophers
	A message has the philosopher ID and a message type == REQUEST / RELEASE chopsticks
*/
#define M (N+1)
typedef enum {REQUEST, RELEASE} MessageType_t;
typedef struct {
	int id;	// philosopher ID
	MessageType_t type;
	
} Message_t;

Message_t queue[M];
int queue_head=0, queue_tail=0;

int queue_empty(void) {
	return (queue_head==queue_tail);
}
int queue_full(void) {
	return (queue_head==(queue_tail+1)%M);
}
int get_queue_message(int *pValue, MessageType_t *pType) {
	if (queue_empty()) return 0;
	*pValue = queue[queue_head].id;
	*pType = queue[queue_head].type;
	queue_head = (queue_head+1)%M;
	return 1;
}
int add_queue_request(int value, MessageType_t type) {
	if (queue_full()) return 0;
	queue[queue_tail].id = value;
	queue[queue_tail].type = type;
	queue_tail = (queue_tail+1)%M;
	return 1;
}
// Release message is inserted afront
int insert_queue_release(int value, MessageType_t type) {
	if (queue_full()) return 0;
	queue_head = (queue_head-1+M)%M;
	queue[queue_head].id = value;
	queue[queue_head].type = type;
	
	return 1;
}
int get_queue_size() {
	return (queue_tail - queue_head + M)%M;
}
void print_queue() {
	int head=queue_head;
	printf("\tQueue:");
	while (head!=queue_tail) {
		if (queue[head].type==REQUEST)
			printf("\t%d", queue[head].id+1);
		else printf("\t-%d", queue[head].id+1);
		head = (head+1)%M;
	}
}
