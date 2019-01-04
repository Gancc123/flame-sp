#include "common/cmdline.h"

#include <grpcpp/grpcpp.h>

#include "include/internal.h"
#include "proto/internal.grpc.pb.h"
#include "service/internal_client.h"
#include "util/utime.h"

#include <cstdio>
#include <cstdint>
#include <string>
#include <fstream>
#include <memory>
#include <list>

using namespace std;
using namespace flame;
using namespace flame::cli;

static int success__() {
    printf("Success!\n");
    return 0;
}

static void check_faild__(int r, const std::string& msg) {
    if (r != 0) {
        printf("faild(%d): %s\n", r, msg.c_str());
        exit(-1);
    }
}

namespace flame {

static unique_ptr<InternalClientContext> make_internal_client_context() {
    FlameContext* fct = FlameContext::get_context();

    InternalClient* client = new InternalClientImpl(fct, grpc::CreateChannel(
        "localhost:6666", grpc::InsecureChannelCredentials()
    ));
    
    if (!client) {
        printf("connect internal cluster faild.\n");
        exit(-1);
    }

    InternalClientContext* fcct = new InternalClientContext(fct, client);

    if (!fcct) {
        printf("create flame client context faild.\n");
        exit(-1);
    }

    return unique_ptr<InternalClientContext>(fcct);
}

class CsdRegisterCli : public Cmdline {
public:
    CsdRegisterCli(Cmdline* parent)
    : Cmdline(parent, "reg", "register to mgr") {}

    Serial<string>  csd_name   {this, 1, "csd_name", "the name of csd"};
    Serial<int>     size       {this, 2, "size", "the size of csd, unit(GB)"};
    Serial<int>     io_addr    {this, 3, "io_addr", "the io address of csd"};
    Serial<int>     admin_addr {this, 4, "admin_addr", "the admin address of csd"};
    Serial<int>     stat       {this, 5, "stat", "the status of csd"};
   
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();
        reg_res_t res;
        reg_attr_t att;
        att.csd_name = csd_name.get();
        att.size = size.get();
        att.io_addr = io_addr.get();
        att.admin_addr = admin_addr.get();
        att.stat = stat.get();

        int r = cct->client()->register_csd(res, att);
        check_faild__(r, "csd register");

        printf("csd_id\tctime\n");
        printf("%llu\t%llu\n",
              res.csd_id,
              res.ctime);
        
        return 0;
    }
};


class CsdUnregisterCli : public Cmdline {
public:
    CsdUnregisterCli(Cmdline* parent)
    : Cmdline(parent, "unreg", "unregister csd") {}

    Serial<int> csd_id {this, 1, "csd_id", "the id of csd"};

    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();

        int r = cct->client()->unregister_csd(csd_id.get());
        check_faild__(r, "csd unregister");
        
        return success__();
    }
};

class CsdSignUpCli : public Cmdline {
public:
    CsdSignUpCli(Cmdline* parent)
    : Cmdline(parent, "signup", "csd sign up") {}

    Serial<int>     csd_id     {this, 1, "csd_id", "the id of csd"};
    Serial<int>     stat       {this, 2, "stat", "the status of csd"};
    Serial<int>     io_addr    {this, 3, "io_addr", "the io address of csd"};
    Serial<int>     admin_addr {this, 4, "admin_addr", "the admin address of csd"};
    
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();

        int r = cct->client()->sign_up(csd_id.get(), stat.get(), io_addr.get(), admin_addr.get());
        check_faild__(r, "csd sign up");

        return success__();
    }
};

class CsdSignOutCli : public Cmdline {
public:
    CsdSignOutCli(Cmdline* parent)
    : Cmdline(parent, "signout", "csd sign out") {}

    Serial<int>     csd_id     {this, 1, "csd_id", "the id of csd"};
    
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();

        int r = cct->client()->sign_out(csd_id.get());
        check_faild__(r, "csd sign out");

        return success__();
    }
};

class CsdPushHeartBeatCli : public Cmdline {
public:
    CsdPushHeartBeatCli(Cmdline* parent)
    : Cmdline(parent, "heart", "csd push heart beat") {}

    Serial<int>     csd_id     {this, 1, "csd_id", "the id of csd"};
    
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();

        int r = cct->client()->push_heart_beat(csd_id.get());
        check_faild__(r, "csd push heart beat");

        return success__();
    }
};

class CsdPushStatusCli : public Cmdline {
public:
    CsdPushStatusCli(Cmdline* parent)
    : Cmdline(parent, "status", "csd push status") {}

    Serial<int>     csd_id     {this, 1, "csd_id", "the id of csd"};
    Serial<int>     stat       {this, 2, "stat", "the status of csd"};
    
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();

        int r = cct->client()->push_status(csd_id.get(), stat.get());
        check_faild__(r, "csd push status");

        return success__();
    }
};

//todo
class CsdPushHealthCli : public Cmdline {
public:
    CsdPushHealthCli(Cmdline* parent)
    : Cmdline(parent, "health", "csd push health info") {}

    Serial<string>  path   {this, 1, "path", "the path of chunk health info"};
    
    HelpAction help {this};


    int def_run() {
        auto cct = make_internal_client_context();
        csd_hlt_attr_t att;
        
        fstream fp;
        fp.open(path, fstream::in);
        if(fp.is_open()) {
            uint64_t t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
            fp >> t1 >> t2 >> t3 >> t4 >> t5 >> t6 >> t7 >> t8 >> t9;
            att.csd_hlt_sub.csd_id = t1;
            att.csd_hlt_sub.size = t2;
            att.csd_hlt_sub.alloced = t3;
            att.csd_hlt_sub.used = t4;
            att.csd_hlt_sub.period.ctime = t5;
            att.csd_hlt_sub.period.wr_cnt = t6;
            att.csd_hlt_sub.period.rd_cnt = t7;
            att.csd_hlt_sub.period.lat = t8;
            att.csd_hlt_sub.period.alloc = t9;
            
            string str;
            while (getline(fp, str)) {
                chk_hlt_attr_t item;
                uint64_t m1, m2, m4, m5, m6, m7, m8, m9, m10, m11;
                uint32_t m3;
                istringstream rec(str);

                rec >> m1 >> m2 >> m3 >> m4 >> m5 >> m6 >> m7 >> m8 >> m9 >> m10 >> m11;
                item.chk_id = m1;
                item.size = m2;
                item.stat = m3;
                item.used = m4;
                item.csd_used = m5;
                item.dst_used = m6;
                item.period.ctime = m7;
                item.period.wr_cnt = m8;
                item.period.rd_cnt = m9;
                item.period.lat = m10;
                item.period.alloc = m11;

                att.chk_hlt_list.push_back(item);
            } 

        }
        else {
            printf("file open erroe!\n");
            exit(-1);
        }

        int r = cct->client()->push_health(att);
        check_faild__(r, "csd push health info");

        return success__();
    }
};

class CsdCli : public Cmdline {
public:
    CsdCli(Cmdline* parent)
    : Cmdline(parent, "csd", "Flame Csd") {}

    CsdRegisterCli      reg      {this};
    CsdUnregisterCli    unreg    {this};
    CsdSignUpCli        signup   {this};
    CsdSignOutCli       signout  {this};
    CsdPushHeartBeatCli heart    {this};
    CsdPushStatusCli    status   {this};
    CsdPushHealthCli    health   {this};

    HelpAction help {this};
}; // class CsdCli

class ChkPullRelatedChkCli : public Cmdline {
public:
    ChkPullRelatedChkCli(Cmdline* parent)
    : Cmdline(parent, "related", "csd pull related chunk info") {}

    Serial<string>  path   {this, 1, "path", "the path of chunk id"};
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();
        list<related_chk_attr_t> res;

        list<uint64_t> chk_list;
        fstream fp;
        fp.open(path, fstream::in);
        if (fp.is_open()) {
            uint64_t temp;
            while (fp >> temp) {
                chk_list.push_back(temp);
            }
        }
        else
        {
            printf("file open erroe!\n");
            exit(-1);
        }

        int r = cct->client()->pull_related_chunk(res, chk_list);
        check_faild__(r, "csd pull related chunk info");

        printf("org_id\tchk_id\tcsd_id\tdst_id\n");
        for(auto it = res.begin(); it != res.end(); ++it)
        {
            printf("%llu\t%llu\t%llu\t%llu\n",
                   it->org_id,
                   it->chk_id,
                   it->csd_id,
                   it->dst_id);
        }
    }
};

class ChkPushChkStat : public Cmdline {
public:
    ChkPushChkStat(Cmdline* parent)
    : Cmdline(parent, "pushstat", "csd push chunk status") {}

    Serial<string>  path   {this, 1, "path", "the path of chunk push info"};
    
    HelpAction help {this};

    int def_run() {
        auto cct = make_internal_client_context();

        list<chk_push_attr_t> chk_list;
        fstream fp;
        fp.open(path, fstream::in);
        if(fp.is_open()){
            uint64_t chk_id;
            uint32_t stat;
            uint64_t csd_id;
            uint64_t dst_id;
            uint64_t dst_mtime;
            while (fp >> chk_id >> stat >> csd_id >> dst_id >> dst_mtime) {
                chk_push_attr_t att;
                att.chk_id = chk_id;
                att.stat = stat;
                att.csd_id = csd_id;
                att.dst_id = dst_id;
                att.dst_mtime = dst_mtime;
                chk_list.push_back(att);
            }
        }
        else
        {
            printf("file open erroe!\n");
            exit(-1);
        }

        int r = cct->client()->push_chunk_status(chk_list);
        check_faild__(r, "csd push chunk status");

        return success__();
    }
};

class ChunkCli : public Cmdline {
public:
    ChunkCli(Cmdline* parent)
    : Cmdline(parent, "chunk", "Flame Chunk") {}

    ChkPullRelatedChkCli  related  {this};
    ChkPushChkStat        pushstat {this};    

    HelpAction help {this};
}; // class ChunkCli

class InternalCli : public Cmdline {
public:
    InternalCli() : Cmdline("flame", "Flame Command Line Interface") {}

    CsdCli      csd     {this};
    ChunkCli    chk     {this};

    HelpAction  help    {this};

}; // class InternalCli

} // namespace flame

int main(int argc, char** argv) {
    InternalCli internal_cli;
    return internal_cli.run(argc, argv);
}