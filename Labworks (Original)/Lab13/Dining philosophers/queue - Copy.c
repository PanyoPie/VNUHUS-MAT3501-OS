#define M 1000
int queue[M], queue_front=0, queue_rear=0;
int queue_empty(void) {
	return (queue_front==queue_rear);
}
int queue_full(void) {
	return (queue_front==(queue_rear+1)%M);
}
int get_queue_request(int *pValue) {
	down(&queue_lock);
	if (!queue_empty())	{
		*pValue = queue[queue_front];
		queue_front = (queue_front+1)%M;
		up(&queue_lock);
		return 1;
	} else {
		up(&queue_lock);
		return 0;
	}
}
int add_queue_request(int value) {
	down(&queue_lock);
	if (!queue_full()) {
		queue[queue_rear] = value;
		queue_rear = (queue_rear+1)%M;
		up(&queue_lock);
		return 1;
	} else {
		up(&queue_lock);
		return 0;
	}
}
int insert_queue_request(int value) {
	down(&queue_lock);
	if (!queue_full()) {
		queue_front = (queue_front-1+M)%M;
		queue[queue_front] = value;
		up(&queue_lock);
		return 1;
	} else {
		up(&queue_lock);
		return 0;
	}
}
int get_queue_size() {
	return (queue_rear - queue_front + M)%M;
}
