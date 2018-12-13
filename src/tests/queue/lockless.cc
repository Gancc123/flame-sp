#include "common/thread/thread.h"
#include "common/atom_queue.h"
#include "util/utime.h"
#include "common/cmdline.h"

#include <cstdio>
#include <memory>
#include <list>
#include <map>
#include <vector>

using namespace std;
using namespace flame;

struct test_task_t {
    int prod_id;
    int item_id;
};

enum TestMod {
    TEST_MOD_INDEP = 0, // 独立模式
    TEST_MOD_INTER = 1  // 干扰模式
};

struct test_attr_t {   
    int que_num;    // 队列数量
    int prod_mod {TEST_MOD_INDEP}; 
    int prod_que;   // 每个队列的生产者数量
    int cons_que;   // 每个队列的消费者数量
    int task_num;   // 每个生成队列生成任务数量
    bool verify {false}; // 是否校验正确性

    int prod_num() const { return que_num * prod_que; }
    int cons_num() const { return que_num * cons_que; }
    int prod_task_num() const { return task_num; }
    int cons_task_num() const { return prod_que * task_num / cons_que; }
    int all_task_num() const { return task_num * que_num * prod_que; }
};

class Prod : public Thread {
public:
    Prod(int prod_id, test_attr_t& tattr, vector<AtomQueue<test_task_t>*>& ques)
    : prod_id_(prod_id), tattr_(tattr), ques_(ques), Thread() {}

    virtual void entry() override {
        // fprintf(stdout, "P_%d start!\n", prod_id_);
        test_task_t item;
        item.prod_id = prod_id_;
        int qidx = 0;
        while (idx_ < tattr_.prod_task_num()) {
            item.item_id = idx_++;
            if (tattr_.prod_mod == TEST_MOD_INDEP) {
                // 独立模式
                while (!ques_[prod_id_ % tattr_.que_num]->push(item));
            } else if (tattr_.prod_mod == TEST_MOD_INTER) {
                // 干扰模式
                while (!ques_[qidx]->push(item));
                if (++qidx == tattr_.que_num) qidx = 0;
            }
            
            // fprintf(stdout, "P_%d %d\n", prod_id_, item.item_id);
        }
        // fprintf(stdout, "P_%d end: event(%d)!\n", prod_id_, idx_);
    }

private:
    int prod_id_;
    test_attr_t& tattr_;
    vector<AtomQueue<test_task_t>*>& ques_;

    int idx_ {0};
};

class Cons : public Thread {
public:
    Cons(int cons_id, test_attr_t& tattr, vector<AtomQueue<test_task_t>*>& ques)
    : cons_id_(cons_id), tattr_(tattr), ques_(ques), Thread() {}

    virtual void entry() override {
        // fprintf(stdout, "C_%d start!\n", cons_id_);
        test_task_t item;
        while (idx_ < tattr_.cons_task_num()) {
            AtomQueue<test_task_t>* que = ques_[cons_id_ % tattr_.que_num];
            if (que->pop(item)) {
                // fprintf(stdout, "C_%d <%d, %d>\n", cons_id_, item.prod_id, item.item_id);
                idx_++;
                count_[item.prod_id]++;
            }
            // else
                // fprintf(stdout, "C_%d %d\n", cons_id_, idx_);
        }
        // fprintf(stdout, "C_%d end: event(%d)!\n", cons_id_, idx_);
    }

    void print() {
        printf("C_%d:\n", cons_id_);
        printf("\ttotal: %d\n", idx_);
        for (auto it = count_.begin(); it != count_.end(); it++) {
            printf("\tP_%d: %d (%.4f%%)\n", it->first, it->second, (double)it->second / idx_ * 100);
        }
    }

private:
    int cons_id_;
    test_attr_t& tattr_;
    vector<AtomQueue<test_task_t>*>& ques_;

    int idx_ {0};
    map<int, int> count_;
};

typedef AtomQueue<test_task_t>* (*aq_create_func_t)();

void atom_queue_test(test_attr_t& tattr, aq_create_func_t func) {
    printf("Mod: %s\n", tattr.prod_mod ? "INTERFERENCE" : "INDEPDENT");
    printf("Queue Number: %d\n", tattr.que_num);
    printf("Producer per Queue: %d\n", tattr.prod_que);
    printf("Consumer per Queue: %d\n", tattr.cons_que);
    printf("Event Number per Producer: %d\n", tattr.task_num);
    printf("---------------------------------------\n");

    vector<AtomQueue<test_task_t>*> ques;

    for (int i = 0; i < tattr.que_num; i++) {
        ques.push_back(func());
    }

    utime_t st = utime_t::now();
    list<Prod*> prods;
    for (int i = 0; i < tattr.prod_num(); i++) {
        Prod* p = new Prod(i, tattr, ques);
        prods.push_back(p);
        p->create("prod");
    }

    list<Cons*> conss;
    for (int i = 0; i < tattr.cons_num(); i++) {
        Cons* c = new Cons(i, tattr, ques);
        conss.push_back(c);
        c->create("cons");
    }

    for (auto it = prods.begin(); it != prods.end(); it++) {
        (*it)->join();
    }

    for (auto it = conss.begin(); it != conss.end(); it++) {
        (*it)->join();
    }
    utime_t et = utime_t::now();

    printf("--------------------------------------\n");
    printf("Result:\n");
    printf("All Event Number: %d\n", tattr.all_task_num());
    printf("Used Time: %llu us\n", (et - st).to_usec());
    printf("Event per Second (EPS): %llu K\n", (unsigned long long)tattr.all_task_num() * 1000 / (et - st).to_usec());

    for (auto it = prods.begin(); it != prods.end(); it++) {
        delete *it;
    }

    for (auto it = conss.begin(); it != conss.end(); it++) {
        (*it)->print();
        delete *it;
    }

    for (auto it = ques.begin(); it != ques.end(); it++) {
        delete *it;
    }
}

AtomQueue<test_task_t>* create_alq() {
    return new AtomLinkedQueue<test_task_t>();
}

AtomQueue<test_task_t>* create_rq() {
    return new RingQueue<test_task_t>(1 << 16);
}

AtomQueue<test_task_t>* create_rb() {
    return new RandomRingQueue<test_task_t>(1 << 16);
}

class LocklessTestApp : public cli::Cmdline {
public:
    LocklessTestApp()
    : Cmdline("lockless", "Lockless Queue Test APP") {}

    cli::Argument<int> que_num  {this, 'q', "queue", "queue number", 1};
    cli::Argument<int> prod     {this, 'p', "prod", "producer number", 1};
    cli::Argument<int> cons     {this, 'c', "cons", "consumer number", 1};
    cli::Argument<int> event    {this, 'e', "event", "the number of event, that would be produced, per producer", 10000};
    cli::Switch interference    {this, 'i', "interference", "interference between queues (only producer)"};
    cli::Switch verify          {this, 'v', "verify", "verify the content"};

    cli::Argument<string> qn    {this, 'n', "qname", "queue type: Linked/Ring/RandomRing", "Ring"};

    cli::Argument<int> times    {this, 't', "times", "test times, 0 means that allway run", 1};

    cli::HelpAction help {this};

    aq_create_func_t create_func(const string& name) {
        if (name == "Linked") {
            return create_alq;
        } else if (name == "Ring") {
            return create_rq;
        } else if (name == "RandomRing") {
            return create_rb;
        } else
            return nullptr;
    }

    virtual int def_run() {
        test_attr_t tattr;
        tattr.que_num = que_num;
        tattr.prod_que = prod;   // 每个队列的生产者数量
        tattr.cons_que = cons;   // 每个队列的消费者数量
        tattr.task_num = event;  // 每个生成队列生成任务数量
        tattr.verify = verify;

        if (interference) 
            tattr.prod_mod = TEST_MOD_INTER;
        else
            tattr.prod_mod = TEST_MOD_INDEP; 

        int run_times = times == 0 || times > INT16_MAX ? INT16_MAX : times; 
        int idx = 0;

        aq_create_func_t func = create_func(qn);
        if (func == nullptr) {
            printf("Queue Type Not Existed!\n");
            return 0;
        }

        while (idx < run_times) {
            printf("================[ %d ]================\n", idx);
            printf("Queue Name: %s\n", qn.get().c_str());
            atom_queue_test(tattr, create_rq);
            printf("\n");
            idx++;
        }
        return 0;
    }
}; // class LocklessTestApp

int main(int argc, char** argv) {
    LocklessTestApp app;
    
    return app.run(argc, argv);
}