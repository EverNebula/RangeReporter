#include "utils.h"
#include "BOBHash32.h"
#include "range.h"
#include "treebf.h"

#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <stdint.h>
using namespace std;

#include <boost/program_options.hpp>
using namespace boost::program_options;

vector<PID> flow;
string file_name;

void ParseArg(int argc, char *argv[])
{
    options_description opts("Benchmark Options");

    opts.add_options()
/*      ("verbose,v", "print more info")
        ("debug,d", "print debug info")
        ("filename,f", value<string>()->required(), "file dir")*/
        ("help,h", "print help info")
        ;
    variables_map vm;
    
    store(parse_command_line(argc, argv, opts), vm);

    if(vm.count("help"))
    {
        cout << opts << endl;
        return;
    }
}

void LoadData_CAIDA(char file[])
{
    int len = 1e5;

    BOBHash32 hash_id;
    // hash_id.initialize(0);
    hash_id.initialize(rand()%MAX_PRIME32);
    
    ifstream is(file, ios::in | ios::binary);
    char buf[2000] = {0};
    vector<long long> ts;

    for (int i = 1; i <= len; i++)
    {
        if(!is.read(buf, 16))
        {
            panic("Data Loading Error.\n");
        }
        flow.push_back(mp(hash_id.run(buf+8, 8), 0));
        long long t = *((long long*)buf);
        ts.push_back(t);
    }

    long long begin = ts[0], delta = ts[ts.size()-1] - ts[0];
    for (int i = 0; i < len; i++)
    {
        flow[i].sc = (double)(ts[i]-begin) / delta * 10000;
        // printf("%lf\n", flow[i].sc);
    }

    cout << "Loading complete." << endl;
}

void LoadData_WebPage(char file[])
{
    BOBHash32 hash_id;
    // hash_id.initialize(0);
    hash_id.initialize(rand()%MAX_PRIME32);
    
    ifstream is(file, ios::in | ios::binary);
    char buf[2000] = {0};

    for (int i = 1; i <= 1e7; i++)
    {
        if(!is.read(buf, 13))
        {
            panic("Data Loading Error.\n");
        }
        flow.push_back(mp(hash_id.run(buf, 4), 0));
    }

    cout << "Loading complete. " << flow.size() << endl;
}

void QueryTest(vector<PID>& flow, int max_len = INT_MAX)
{
    static map<int, int> flow_cnt;
    static map<int, vector<double>> flow_pos;
    int n = min((int)flow.size(), max_len);
    
    flow_cnt.clear();
    flow_pos.clear();
    for (int i = 0; i < n; ++i)
    {
        flow_cnt[ flow[i].ft ] = 0;
        flow_pos[ flow[i].ft ] = {};
    }
    for (int i = 0; i < n; ++i)
    {
        flow_cnt[ flow[i].ft ]++;
        flow_pos[ flow[i].ft ].push_back(flow[i].sc);
    }

    // summary
    printf("Range Query Test\n");
    printf("-------------------------------------\n");

    // 100KB
    for (int mem = 50; mem <= 500; mem += 50)
    {
        int beta = 10;
        printf("\n* Mem = %d KB\n", mem);

        RangeBF rbf(16*1024*mem, beta, 10000.);
        TreeBF tbf(32*1024*mem/beta, beta, 10000., 3);
        rbf.init();
        tbf.init();
        for (int i = 0; i < n; ++i)
        {
            rbf.insert(flow[i].ft, flow[i].sc);
            tbf.insert(flow[i].ft, flow[i].sc);
        }

        // fstream fout(file_name, ios::out | ios::app);
        double pr = 0, avg = 0, ar = 0;
        
        // auto itr = flow_cnt.end();
        // for (int i = 0; i < 10; ++i)
        // {
        //     itr--;
        //     vector<PDD> res = tbf.query((*itr).ft);
        //     vector<double> ans = flow_pos[(*itr).ft];
        //     for (int j = 0; j < ans.size(); ++j)
        //     {
        //         printf("%lf ", ans[j]);
        //     }
        //     printf("\n");
        //     for (int j = 0; j < res.size(); ++j)
        //     {
        //         printf("[%lf,%lf] ", res[j].ft, res[j].sc);
        //     }
        //     printf("\n\n");
        // }

        {
            double pr = 0, avg = 0, ar = 0;
            int sz = flow_pos.size(), ans_num = 0;
            for (auto f : flow_pos)
            {
                int id = f.ft;
                vector<double> ans = f.sc;
                vector<PDD> res = rbf.query(f.ft);

                int r_sz = ans.size(), e_sz = res.size();
                if (r_sz == e_sz)
                    pr += 1;
                ar += (e_sz - r_sz) / r_sz;
                for (int i = 0; i < e_sz; ++i)
                    avg += res[i].sc - res[i].ft;
                ans_num += e_sz;
            }

            avg /= ans_num;
            pr /= sz;
            ar /= sz;
            printf("RangeBF\n- Pr: %.3lf   Avg.Len: %.3lf   AR: %.3lf\n", pr, avg, ar);
        }

        {
            double pr = 0, avg = 0, ar = 0;
            int sz = flow_pos.size(), ans_num = 0;
            for (auto f : flow_pos)
            {
                int id = f.ft;
                vector<double> ans = f.sc;
                vector<PDD> res = tbf.query(f.ft);

                int r_sz = ans.size(), e_sz = res.size();
                if (r_sz == e_sz)
                    pr += 1;
                ar += (e_sz - r_sz) / r_sz;
                for (int i = 0; i < e_sz; ++i)
                    avg += res[i].sc - res[i].ft;
                ans_num += e_sz;
            }

            avg /= ans_num;
            pr /= sz;
            ar /= sz;
            printf("TreeBF\nPr: %.3lf   Avg.Len: %.3lf   AR: %.3lf\n", pr, avg, ar);
        }
    }
}


int main(int argc, char *argv[])
{
    srand(2020);
    // parse args
    ParseArg(argc, argv);

    // load data
    LoadData_CAIDA("../topk/data/formatted00.dat");
    // LoadData_WebPage("data/webdocs_form00.dat");
    
    file_name = string("log/log_tp_caida2");

    QueryTest(flow);

    return 0;
}
