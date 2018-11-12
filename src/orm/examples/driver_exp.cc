#include "orm/driver.h"
#include "orm/my_impl/my_impl.h"
#include "orm/stmt.h"
#include "orm/cols.h"
#include <memory>
#include <iostream>

namespace form = flame::orm;
using namespace std;

class TestTbl : public form::Table {
public:
    TestTbl() : Table("test") {}

    form::BigIntCol   a {this, "a"};
    form::BigIntCol   b {this, "b"};
}; // class TestTbl

int main() {
    shared_ptr<form::Driver> driver(new form::MysqlDriver(
        "tcp://127.0.0.1:3306", "flame_mgr_db", "flame", "123456"));
    shared_ptr<form::Stub> stub = driver->create_stub();

    if (!stub.get()) {
        cout << "connect faild" << endl;
        return 0;
    } else {
        cout << "connect success" << endl;
    }

    TestTbl test;

    form::SelectStmt stmt;
    stmt.table(test);

    cout << "Execute:" << endl;
    shared_ptr<form::Result> res = stub->execute(stmt);

    cout << "Result:" << endl;
    if (!res) {
        cout << "res error" << endl;
        return 0;
    }
    if (res->OK()) {
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