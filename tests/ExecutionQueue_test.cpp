#include "../ExecutionQueue.h"
#include "../ExecutionThread.h"
#include "../ExecutionAtomic.h"
#include "../Utilities.h"

// g++ -o /tmp/test ExecutionQueue_test.cpp -I.. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

#define kTestCount		10000
#define kThreadCount	10
struct QueueElement {
	QueueElement(const std::string &n, uint32_t i):name(n), index(i) {}
	std::string	name;
	uint32_t	index;
};

typedef exec::Queue<QueueElement>	Queue;
class TestThread : public exec::Thread {
	public:
		TestThread(const std::string &name, Queue *queue)
				:_name(name), _queue(queue) {
			_creationCount++;
			start();
		}
		virtual ~TestThread() {
			//printf("........ %s enqueuing %d (last)\n", _name.c_str(), 0);
			_queue->enqueue(QueueElement(_name, 0));
			//printf(" ------- %s enqueuing %d (last)\n", _name.c_str(), 0);
			_creationCount--;
		}
		static bool allDone() {
			return _creationCount.value() == 0;
		}
	protected:
		virtual void *run() {
			for(uint32_t x= 0; x < kTestCount; ++x) {
				//printf("........ %s enqueuing %d\n", _name.c_str(), x);
				_queue->enqueue(QueueElement(_name, x));
				//printf(" ------- %s enqueuing %d\n", _name.c_str(), 0);
			}
			return NULL;
		}
	private:
		static exec::AtomicNumber	_creationCount;
		std::string					_name;
		Queue						*_queue;
		TestThread(const TestThread&); ///< Prevent Usage
		TestThread &operator=(const TestThread&); ///< Prevent Usage
};

exec::AtomicNumber	TestThread::_creationCount;

int main(int, const char *[]) {
	uint32_t	foundCount[kTestCount];
	std::string	name;
	Queue		work(10);
	
	for(int thread= 0; thread < kThreadCount; ++thread) {
		new TestThread(utils::stringifyInteger(thread, name), &work);
	}
	
	uint32_t	iterations= 0;
	
	while(!TestThread::allDone() || !work.empty()) {
		//printf(" --- main --- work.dequeue()\n");
		QueueElement	element= work.dequeue();
		//printf(" --- main ... work.dequeue()\n");
		AssertMessageException(element.index < kTestCount);
		++foundCount[element.index];
		AssertMessageException(iterations < kTestCount * kThreadCount + kThreadCount);
		++iterations;
		//printf("%d Thread #%s Index %d\n", iterations, element.name.c_str(), element.index);
	}
	AssertMessageException(iterations == kTestCount * kThreadCount + kThreadCount);
	AssertMessageException(foundCount[0] == 2 * kThreadCount);
	return 0;
}
