#include "monitor.h"
#include <stdio.h>
#include <queue>
#include <list>
#include <pthread.h>
#include <string>
#include <iostream>

using namespace std;

int GWriters = 1;
int GWritersVIP = 1;
int GReaders = 1;
int GWritersMess = 10;
int GQueueLength = 5;
double* GWritersSleep;
double* GWritersVIPSleep;
double* GReadersSleep;
struct timespec GStart;

double get_time() {
    struct timespec at;
    clock_gettime(CLOCK_MONOTONIC, &at);
    float tf = (at.tv_sec - GStart.tv_sec) + (at.tv_nsec - GStart.tv_nsec) / 1000000000.0;
    return tf;
}

class Message {
private:
    string text;
public:
    float createtime;
    float semaphoretime;
    float readtime;
    Message(string _text, float _createtime) {
        text = _text;
        createtime = _createtime;
    }
    string get_message() {
        return text;
    }
    void set_message(string _text) {
        text = _text;
    }
    void log() {
        cout << "Message " << text << " created" << endl;
    }
};

class Queue : public Monitor {
public:
    Queue(int _QueueLength) {
        QueueLength = _QueueLength;
    }
    void PutMessage(Message _mess) {
        enter();

        if(Messages.size() == QueueLength)
            wait(NotFull);

        _mess.semaphoretime = get_time();
        Messages.push_back(_mess);
    
        if(Messages.size() == 1)
            signal(NotEmpty);
        
        leave();
    }
    void PutVIPMessage(Message _mess) {
        enter();

        if(Messages.size() == QueueLength)
            wait(NotFull);
        
        _mess.semaphoretime = get_time();
        
        Messages.push_back(Messages.back());

        for (int i = Messages.size()-2;i >= vip;i--) {
            Messages[i+1] = Messages[i];
        }
        Messages[vip] = _mess;
        vip++;

        if(Messages.size() >= 1)
            signal(NotEmpty);
        
        leave();
    }
    Message GetMessage() {
        enter();

        if(Messages.size() == 0)
            wait(NotEmpty);
        
        Message _mess = Messages.front();
        _mess.readtime = get_time();


        //PrintQueue();
        Messages.pop_front();
        if (vip>0)
            vip--;

        if (Messages.size() == QueueLength - 1) {
            signal(NotFull);
        }

        leave();
        return _mess;
    }
    void PrintQueue() {
        deque<Message>::iterator it;
        for (it=Messages.begin();it!=Messages.end();++it)
            cout << it->get_message() << endl;
    }
    int QueueLength;
protected:
    Condition NotFull;
    Condition NotEmpty;
    std::deque<Message> Messages;
    int vip;
};

Queue Gmessage_queue = Queue(GQueueLength);

void* Writer(void* arg) {
    int myid = pthread_self() % 10000 + 10000;

	int sleep = *GWritersSleep;
    usleep(sleep * 1000000);
    GWritersSleep++;
	for(int n = 0; n < GWritersMess; n++) {
		Message _mess = Message(to_string(myid) + "n" + to_string(n), get_time());
		Gmessage_queue.PutMessage(_mess);
	}
}
void* WriterVIP(void* arg) {
    int myid = pthread_self() % 10000 + 10000;

    int sleep = *GWritersVIPSleep;
    usleep(sleep * 1000000);
    GWritersVIPSleep++;
	for(int n = 0; n < GWritersMess; n++) {
		Message _mess = Message(to_string(myid) + "v" + to_string(n), get_time());
        Gmessage_queue.PutVIPMessage(_mess);
	}
}
void* Reader(void* arg) {
    int sleep = *GReadersSleep;
    GReadersSleep++;
    usleep(sleep * 1000000);

	for(int n = 0; n < (GWritersMess*(GWriters+GWritersVIP)); n++) {
		Message _mess = Gmessage_queue.GetMessage();
        cout << _mess.get_message() << " (" << _mess.createtime << ", " << _mess.semaphoretime << ", " << _mess.readtime << ")" << endl;
    }
}

void make_test(int _Writers, int _WritersVIP, int _Readers, int _WritersMess, int _QueueLength, double _WritersSleep[], double _WritersVIPSleep[], double _ReadersSleep[]) {
    clock_gettime(CLOCK_MONOTONIC, &GStart);GWriters = _Writers;
    GWritersVIP = _WritersVIP;
    GReaders = _Readers;
    GWritersMess = _WritersMess;
    GQueueLength = _QueueLength;
    GWritersSleep = _WritersSleep;
    GWritersVIPSleep = _WritersVIPSleep;
    GReadersSleep = _ReadersSleep;

    Gmessage_queue.QueueLength = GQueueLength;
    
    pthread_t* ReaderT= new pthread_t[GReaders];
	pthread_t* WriterT = new pthread_t[GWriters];
	pthread_t* WriterVIPT = new pthread_t[GWritersVIP];

	for(int I = 0; I < GWriters; I++)
		pthread_create(&WriterT[I], NULL, &Writer, NULL);
	for(int I = 0; I < GWritersVIP; I++)
		pthread_create(&WriterVIPT[I], NULL, &WriterVIP, NULL);
	for(int I = 0; I < GReaders; I++)
     	pthread_create(&ReaderT[I], NULL, &Reader, NULL);
     
	for(int I = 0; I < GWriters; I++)
		pthread_join(WriterT[I], NULL);
	for(int I = 0; I < GWritersVIP; I++)
		pthread_join(WriterVIPT[I], NULL);
	for(int I = 0; I < GReaders; I++)
		pthread_join(ReaderT[I], NULL);
}
	
int main(int argc, char const *argv[]) {
    printf("Test 1: Reading from empty queue. [2,0,0] queue 5\n");
    double t1wr_sl[1] = {2};
    double t1wrv_sl[1] = {0};
    double t1r_sl[1] = {0};
    make_test(1, 0, 1, 10, 5, t1wr_sl, t1wrv_sl, t1r_sl);
	printf("\n");

    printf("Test 2: Reading from full queue. [0,0,2] queue 5\n");
    double t2wr_sl[1] = {0};
    double t2wrv_sl[1] = {0};
    double t2r_sl[1] = {2};
    make_test(1, 0, 1, 10, 5, t2wr_sl, t2wrv_sl, t2r_sl);
    printf("\n");

    printf("Test 3: Only normal writers, last vip. [0,1,3] queue 6\n");
    double t3wr_sl[1] = {0};
    double t3wrv_sl[1] = {1};
    double t3r_sl[1] = {3};
    make_test(1, 1, 1, 5, 6, t3wr_sl, t3wrv_sl, t3r_sl);
    printf("\n");

    printf("Test 4: Only vip writers, last normal. [0,1,3] queue 6\n");
    double t4wr_sl[1] = {1};
    double t4wrv_sl[1] = {0};
    double t4r_sl[1] = {3};
    make_test(1, 1, 1, 5, 6, t4wr_sl, t4wrv_sl, t4r_sl);
    printf("\n");

    printf("Test 5: Mixed writers. [0,0,3] queue 5\n");
    double t5wr_sl[1] = {0};
    double t5wrv_sl[1] = {0};
    double t5r_sl[1] = {3};
    make_test(1, 1, 1, 10, 5, t5wr_sl, t5wrv_sl, t5r_sl);
    printf("\n");
	return 0;
}