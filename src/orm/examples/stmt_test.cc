#include "orm/stmt.h"
#include "orm/cols.h"
#include "orm/opts.h"

#include <iostream>

using namespace std;
using namespace flame::orm;

class VolumeGroupTbl : public Table {
public:
    VolumeGroupTbl() : Table("volume_group") {}

    BigIntCol   vg_id   {this, "vg_id", ColFlag::PRIMARY_KEY | ColFlag::AUTO_INCREMENT};
    StringCol   name    {this, "name", 64};
    BigIntCol   ctime   {this, "ctime"};
    IntCol      volumes {this, "volumes"};
    BigIntCol   size    {this, "size"};
    BigIntCol   alloced {this, "alloced"};
    BigIntCol   used    {this, "used"};
}; // class VolumeGroupTbl

class VolumeTbl : public Table {
public:
    VolumeTbl() : Table("volume") {}

    BigIntCol   vol_id  {this, "vol_id", ColFlag::PRIMARY_KEY | ColFlag::AUTO_INCREMENT};
    BigIntCol   vg_id   {this, "vg_id"};
    StringCol   name    {this, "name", 64};
    BigIntCol   ctime   {this, "ctime"};
    BigIntCol   size    {this, "size"};
    BigIntCol   alloced {this, "alloced"};
    BigIntCol   used    {this, "used"};
}; // class VolumeTbl

int main() {
    VolumeGroupTbl vg_tbl;
    VolumeTbl vol_tbl;

    cout << "Create Table:" << endl;
    cout << vg_tbl.stmt_create() << endl << endl;

    cout << "Drop Table:" << endl;
    cout << vg_tbl.stmt_drop() << endl << endl;

    SelectStmt select;
    select.table(vg_tbl)
        .column({vg_tbl.vg_id, vg_tbl.name, vg_tbl.ctime})
        .join(vol_tbl, vg_tbl.vg_id == vol_tbl.vg_id)
        .where({vg_tbl.vg_id == vol_tbl.vg_id, vg_tbl.vg_id == vol_tbl.vg_id})
        .group_by(vol_tbl.vg_id)
        .having(or_(vg_tbl.vg_id == vol_tbl.vg_id, vg_tbl.vg_id == vol_tbl.vg_id))
        .order_by(vol_tbl.size)
        .limit(10)
        .offset(10);
    cout << "Select:" << endl;
    cout << select.to_str() << endl << endl;
    
    InsertStmt insert;
    insert.table(vg_tbl)
        .column({vg_tbl.vg_id, vg_tbl.name, vg_tbl.ctime})
        .value({123, 123.5, "a789"});
    cout << "Insert:" << endl;
    cout << insert.to_str() << endl << endl;
    
    InsertStmt insert_select;
    insert_select.table(vg_tbl)
        .column({vg_tbl.vg_id, vg_tbl.name, vg_tbl.ctime})
        .select(select);
    cout << "Insert Select:" << endl;
    cout << insert_select.to_str() << endl << endl;

    UpdateStmt update;
    update.table(vg_tbl)
        .assign({
            set_(vg_tbl.size, 1000),
            set_(vg_tbl.alloced, vg_tbl.size),
            set_(vg_tbl.used, vg_tbl.size - vg_tbl.alloced)
        })
        .order_by(vg_tbl.volumes)
        .limit(10);
    cout << "Update:" << endl;
    cout << update.to_str() << endl << endl;

    DeleteStmt del;
    del.table(vol_tbl)
        .where({
            or_(vg_tbl.vg_id == vol_tbl.vg_id, vg_tbl.vg_id == vol_tbl.vg_id),
            or_(vg_tbl.vg_id == vol_tbl.vg_id, vg_tbl.vg_id == vol_tbl.vg_id)
        })
        .order_by(vol_tbl.size)
        .limit(10);
    cout << "Delete:" << endl;
    cout << del.to_str() << endl << endl;

    cout << "Test:" << endl;
    cout << not_(vg_tbl.name > 100).to_str() << endl;
    cout << exists_(select).to_str() << endl;
    cout << not_exists_(select).to_str() << endl;
    cout << and_(vg_tbl.size > 1000, vg_tbl.size < 2000).to_str() << endl;
    cout << or_(vg_tbl.size < 1000, vg_tbl.size > 2000).to_str() << endl;
    cout << between_(vg_tbl.size, 1000, 2000).to_str() << endl;
    cout << not_between_(vg_tbl.size, 1000, 2000).to_str() << endl;
    cout << like_(vg_tbl.name, "*big*").to_str() << endl;
    cout << not_like_(vg_tbl.name, "*sm*").to_str() << endl;
    cout << glob_(vg_tbl.name, "**Big*").to_str() << endl;
    cout << not_glob_(vg_tbl.name, "**SM*").to_str() << endl;
    cout << (vg_tbl.name == 100).to_str() << endl;
    cout << (vg_tbl.name != 100).to_str() << endl;
    cout << (vg_tbl.name > 100).to_str() << endl;
    cout << (vg_tbl.name < 100).to_str() << endl;
    cout << (vg_tbl.name >= 100).to_str() << endl;
    cout << (vg_tbl.name <= 100).to_str() << endl;
    cout << (vg_tbl.name + 100).to_str() << endl;
    cout << (vg_tbl.name - 100).to_str() << endl;
    cout << (vg_tbl.name * 100).to_str() << endl;
    cout << (vg_tbl.name / 100).to_str() << endl;
    cout << (vg_tbl.name % 100).to_str() << endl;
}