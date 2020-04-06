// encoding: utf-8
// 大括号不换行的跟换行的打，变量名首字母大写的跟不大写的打，私有域前面加下划线的跟不加下划线的打

#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <algorithm>
#include <dolores/dolores.hpp>
#include "mt19937ar.h"

using namespace std;
using namespace cq;
using namespace dolores;
using namespace dolores::matchers;

typedef unsigned long long ull;

namespace ys {
    bool loaded = false;

    struct ys_item {
        std::string name, good, bad;
        bool vis = false;
    };

    vector<ys_item> lst;

    static const std::string ys_type[5] = {"大吉","中吉","小吉","凶","大凶"};

    ull load_txt_data() {
        loaded = false;
        auto fdir = dir::app();
        fdir = fdir + "ys_list.txt";
        logging::info("运势路径", fdir);
        ifstream inp;
        inp.open(fdir);
        if (inp) {
            ull n;
            inp >> n;
            if (n >= 4) {
                ys_item wait;
                wait.vis = false;
                for (register ull i = 1; i <= n; ++i) {
                    inp >> wait.name >> wait.good >> wait.bad;
                    lst.push_back(wait);
                }
                logging::info("加载成功", "文件已载入。");
                loaded = true;
            } else {
                logging::error("加载失败", "n 必须大于等于 4。");
            }
            inp.close();
        } else {
            logging::error("加载失败", "请检查路径。");
        }
        return 0;
    }

    std::string get_context(ull uid) {
        std::string res = "配置文件似乎出了点小问题的说，快点告诉主人吧~";
        if (loaded) {
            ull dt = (time(NULL) + 57600) / 86400; // date
            ull mt_seed = (((dt * 998244353) % 1000000007) ^ uid) % 4294967295;
            init_genrand(mt_seed);
            vector<ys_item> lst2 = lst;
            ull rnd[5];
            rnd[0] = genrand_int32() % 5;
            for (register ull i = 1; i <= 4; ++i) {
                rnd[i] = genrand_int32() % lst2.size();
                while (lst2[rnd[i]].vis) {
                    ++rnd[i];
                    rnd[i] = (rnd[i] >= lst2.size()) ? 0 : rnd[i];
                }
                lst2[rnd[i]].vis = true;
            }
            res = "今天的运势（仅供参考）：" + ys_type[rnd[0]] + "\n"
                              + ((rnd[0] == 4) ? ("诸事不宜")
                                               : ("宜：" + lst2[rnd[1]].name + "/" + lst2[rnd[1]].good + "\n宜："
                                                  + lst2[rnd[2]].name + "/" + lst2[rnd[2]].good))
                              + "\n"
                              + ((rnd[0] == 0) ? ("万事皆宜")
                                               : ("忌：" + lst2[rnd[3]].name + "/" + lst2[rnd[3]].bad + "\n忌："
                                                  + lst2[rnd[4]].name + "/" + lst2[rnd[4]].bad));
            if (rnd[0] == 0) res += "\n即使大吉，也不能太浪的说~";
            if (rnd[0] == 4) res += "\n大凶也不要紧，本机也会一直保佑你的~";
        }
        return res;
    }
} // namespace ys

CQ_INIT {
    dolores::init();
    dolores::on_startup([] { 
        logging::info("准备加载", "在使用本插件前，请确保已经仔细阅读使用说明书，若未按照说明书使用，造成的一切后果与本插件作者无关。");
        ys::load_txt_data();
    });
}

CQ_MENU(ys_reload) {
    logging::info("菜单", "准备重载运势");
    ys::load_txt_data();
}

dolores_on_message("运势_群", group(), to_me(command({"yunshi", "ys", "运势"}))) {
    const auto &event = current.event_as<cq::GroupMessageEvent>();
    auto text = ys::get_context(event.user_id);
    current.reply(text);
    current.event.block();
}

dolores_on_message("运势_讨论组", discuss(), to_me(command({"yunshi", "ys", "运势"}))) {
    const auto &event = current.event_as<cq::DiscussMessageEvent>();
    auto text = ys::get_context(event.user_id);
    current.reply(text);
    current.event.block();
}

dolores_on_message("运势_私", direct(), to_me(command({"yunshi", "ys", "运势"}))) {
    const auto &event = current.event_as<cq::MessageEvent>();
    auto text = ys::get_context(event.user_id);
    current.reply(text);
    current.event.block();
}