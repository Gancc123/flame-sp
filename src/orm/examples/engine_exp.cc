#include "orm/orm.h"
#include <memory>
#include <iostream>
#include <list>

namespace form = flame::orm;
using namespace std;

class TestTbl : public form::DBTable {
public:
    TestTbl(shared_ptr<form::DBEngine>& engine) 
    : DBTable(engine, "test") { auto_create(); }

    form::BigIntCol   a {this, "a"};
    form::BigIntCol   b {this, "b"};

    list<int64_t> all() {
        list<int64_t> li;
        shared_ptr<form::Result> res = query().exec();
        if (res && res->OK()) {
            shared_ptr<form::DataSet> ds = res->data_set();
            for (auto it = ds->cbegin(); it != ds->cend(); it++) {
                li.push_back(it->get(a));
                li.push_back(it->get(b));
            }
        }
        return li;
    }
}; // class TestTbl

int main() {
    shared_ptr<form::DBEngine> engine 
        = form::DBEngine::create_engine("mysql://flame:123456@127.0.0.1:3306/flame_mgr_db");
    if (!engine) {
        cout << "connect faild" << endl;
        return 0;
    } else {
        cout << "connect success" << endl;
    }

    TestTbl test(engine);

    cout << "Methos Call:" << endl;
    list<int64_t> li = test.all();
    for (auto it = li.begin(); it != li.end(); it++) {
        cout << *it << ", ";
    }
    cout << endl << endl;

    cout << "Outter Call:" << endl;
    shared_ptr<form::Result> res = test.query().exec();
    if (res && res->OK()) {
        cout << "rows: " << res->len() << endl;
        shared_ptr<form::DataSet> ds = res->data_set();
        cout << "cols: " << ds->cols() << endl;
        for (int idx = 0; idx < ds->cols(); idx++) {
            cout << ds->col_name(idx) << ", ";
        }
        cout << endl;
        
        for (auto it = ds->cbegin(); it != ds->cend(); it++) {
            for (int i = 0; i < ds->cols(); i++) {
                cout << it->get_int(i) << ", ";
            }
            cout << endl;
        }
    } else {
        cout << "select faild" << endl;    
    }
}
