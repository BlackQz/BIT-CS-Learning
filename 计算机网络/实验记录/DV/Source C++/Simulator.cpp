#include<iostream>
#include<fstream>
#include<thread>
#include<map>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<windows.h>
#include <ctime>
#include"Physical.cpp"
#include"Utils.cpp"

using namespace std;

const string config_file = R"(config\params.txt)";

typedef pair<string, time_t> str_time;

class PiQueue
{
private:
    mutex mtx;
    condition_variable cv;
    queue<str_time> q;

public:
    PiQueue()
    {
        while (!q.empty())
        {
            q.pop();
        }
    }

    inline void push_pi(const str_time &s)
    {
        unique_lock<mutex> lock(mtx);
        q.push(s);
        cv.notify_one();
    }

    inline str_time get_pi()
    {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]
        { return !q.empty(); });
        auto s = q.front();
        q.pop();
        return s;
    }

    inline void clear()
    {
        unique_lock<mutex> lock(mtx);
        while (!q.empty())
        {
            q.pop();
        }
    }
};

class RoutTable
{
private:
    string local;
    float unreachdis;    // ���ɴ����
    // rout ·�ɱ�: dest -> (neighbor, dis)
    map<string, pair<string, float> > rout;
    // history ��ʷ·�ɱ�: (dest, neighbor) -> dis
    map<pair<string, string>, float> history;

    mutex mtx;

public:
    RoutTable(const string &id)
    {
        local = id;
        rout.clear();
        history.clear();
    }

    void set_unreachdis(float dis)
    {
        unreachdis = dis;
    }

    /* ��ĳ��Ŀ��·����Ϊ���ɴ� */
    void set_unreachable(const string &dest)
    {
        unique_lock<mutex> lock(mtx);
        auto item = rout[dest];
        history[make_pair(dest, item.first)] = unreachdis;
        update_rout();
    }

    /* ����·���������ʷ·�ɱ������ڳ�ʼ�� */
    void update_history(const string &dest, const string &neighbor, float dis)
    {
        unique_lock<mutex> lock(mtx);
        if (dis < 0)
            dis = unreachdis;
        history[make_pair(dest, neighbor)] = dis;
    }

    /* ���ݷ��͵�·�ɱ����±�����ʷ·�ɱ� */
    void update_history(const string &sender, const vector<pair<string, float> > &recv_rout)
    {
        unique_lock<mutex> lock(mtx);

        // �ж��Ƿ�Ϊ�������ϵ��ھ�
        pair<string, string> direct_key(sender, sender);
        if (!history.count(direct_key))
        {
            for (const auto &item : recv_rout)
            {
                if (item.first == local)
                {
                    history[direct_key] = item.second;
                }
            }
        }

        // ��ȡ local ֱ�ӵ��� sender �ľ���
        float direct_dis = history[direct_key];

        // ������ʷ·��
        for (auto &item : recv_rout)
        {             /*                         dest     sender */
            pair<string, string> update_key(item.first, sender);
            if (item.first != local)    // ����Ŀ���� local ��·����
            {
                history[update_key] = min(direct_dis + item.second, unreachdis);
            }
        }
    }

    /* ������ʷ·�ɱ�����·�ɱ� */
    void update_rout()
    {
        unique_lock<mutex> lock(mtx);
        map<string, pair<string, float> > new_rout;
        // ���ݵ�ǰ�� history ����·�ɱ�
        for (auto &item : history)
        {
            if (!new_rout.count(item.first.first))
            {
                new_rout[item.first.first] = make_pair(item.first.second, item.second);
            }
            else if (item.second < new_rout[item.first.first].second)
            {
                new_rout[item.first.first].first = item.first.second;   // neighbor
                new_rout[item.first.first].second = item.second;    // distance
            }
        }
        rout = new_rout;
    }

    /* ��·�ɱ����л�Ϊ�ַ��� */
    string to_string()
    {
        unique_lock<mutex> lock(mtx);
        string res = "#" + local;
        for (auto &item : rout)
        {
            res += "#" + item.first + " " + std::to_string(item.second.second);
        }
        res += "#";
        return res; // ��ʽ��#id#dst1 dis1#dst2 dis2#...#
    }

    /* ��·�ɱ����л�Ϊ�ַ��� */
    string log(int num)
    {
        unique_lock<mutex> lock(mtx);
        string res = "## Sent. SourceNode = " + local + "; SequenceNumber = " + std::to_string(num) + "\n";
        for (auto &item : rout)
        {
            res += "DestNode = " + item.first;
            res += ";\tDistance = " + std::to_string(item.second.second);
            res += ";\tNeighbor = " + item.second.first + "\n";
        }
        return res;
    }

    /* ��ӡ·�ɱ� */
    void debug()
    {
        unique_lock<mutex> lock(mtx);
        cout << "--------------------------------------------" << endl;
        cout << "Router Id: " << local << endl;

        for (auto &item : history)
        {
            // dst dis nei
            cout << "DestNode: " << item.first.first;
            cout << "\tDistance: " << item.second;
            cout << "\tNeighbor: " << item.first.second;
            cout << endl;
        }

        cout << "---------history------------" << endl;

        for (auto &item : rout)
        {
            // dst dis nei
            cout << "DestNode: " << item.first;
            cout << "\tDistance: " << item.second.second;
            cout << "\tNeighbor: " << item.second.first;
            cout << endl;
        }
        cout << "--------------------------------------------" << endl;
    }
};

class Simulator
{
private:
    int state;           // 0��ͣ��1���У�-1����
    string id;           // ��·������
    int frequency;       // ÿ���������·�ɷ�һ����Ϣ
    float unreachdis;    // ���ɴ����
    int maxtime;         // ���ȴ�ʱ��
    string init_file;
    string log_file;

    Physical *p;
    PiQueue Q;        //������յ����ݵĶ���

    // ·�ɱ�
    RoutTable rout;

    // �жϽ��ܳ�ʱ
    fd_set rset;
    map<string, time_t> timeout;
    // timeout map ��д������
    mutex timeout_mtx;

    // run �����д����߳��� recv��send��timeout��process
    int alive_thread_num;
    mutex alive_num_mtx;
    condition_variable alive_num_cv;

    // ���ڹ���ʱʹ·����ͣ����
    mutex mtx;
    condition_variable cv;

    // ���ַ��������л�Ϊ·�ɱ����һ��Ԫ�ش���sender��֮ǰ����·�ɱ�
    static vector<pair<string, float> > str2table(const string &s, string &sender)
    {
        vector<pair<string, float> > res;
        int l = 0, r = 0;
        while (true)
        {
            r = s.find('#', l + 1);
            string tmp = s.substr(l + 1, r - l - 1);
            vector<string> info = split(tmp, set<char>{' '});
            if (info.size() == 1)
            {
                sender = info[0];
            }
            else
            {
                res.emplace_back(info[0], atof(info[1].data()));
            }
            l = r;
            if (r == s.size() - 1)
                break;
        }
        return res;
    }

public:
    Simulator(string id, int port, string &init_file) : rout(id)
    {
        state = 1;
        this->alive_thread_num = 0;
        this->id = move(id);
        this->init_file = init_file;
        this->log_file = init_file;
        this->log_file.replace(init_file.find("txt"), 3, "log");

        char in[500];
        // ���������ļ�
        ifstream config(config_file.data());
        while (config.getline(in, 500))
        {
            auto x = split(in, set<char>({' ', ':', '\t'}));
            if (x[0] == "Frequency")
            {
                frequency = atoi(x[1].data());
            }
            else if (x[0] == "Unreachable")
            {
                unreachdis = atof(x[1].data());
            }
            else if (x[0] == "MaxValidTime")
            {
                maxtime = atoi(x[1].data());
            }
        }

        p = new Physical(port);
        rout.set_unreachdis(unreachdis);

        // ��ʼ��·�ɱ�
        ifstream f(init_file.data());
        while (f.getline(in, 1000))
        {
            auto x = split(in, set<char>({' ', ':', '\t'}));
            if (x.size() < 3)
            {
                cout << "Initial file content error" << endl;
                continue;
            }
            // ��ӵ� udp ����Ŀ��
            p->add_nei(x[0], atoi(x[2].data()));
            // ��¼ÿ���ھӵĳ�ʱʱ��
            timeout_mtx.lock();
            timeout[x[0]] = time(nullptr) + maxtime / 1000;
            timeout_mtx.unlock();
            // ��ӵ�·�ɱ�
            rout.update_history(x[0], x[0], atof(x[1].data()));
        }
        rout.update_rout();
    }

    void run()
    {
        // �����ʼ�ڽӱ�
        debug();
        // �����߳�
        thread([this]
               {
                   alive_num_mtx.lock();
                   this->alive_thread_num++;
                   alive_num_mtx.unlock();

                   int fd = p->get_sockid();
                   FD_ZERO(&rset);
                   FD_SET(fd, &rset);
                   struct timeval tv{};
                   //����תΪ��
                   int sec = frequency / 1000;
                   int msec = frequency % 1000;
                   tv.tv_sec = sec;
                   tv.tv_usec = msec * 1000;

                   int flag;
                   while (true)
                   {
                       // cout << "recv thread start" << endl;
                       while (state != 1)
                       {
                           unique_lock<mutex> lock(mtx);
                           if (state == -1)          // state=-1������ѭ��
                               break;
                           if (state == 0)
                               Q.clear();
                           cv.wait(lock);
                       }
                       if (state == -1)          // state=-1������ѭ��
                           break;
                       // ����������
                       flag = select(fd + 1, &rset, nullptr, nullptr, nullptr);
                       if (flag > 0)
                       {
                           string s = p->recv();
                           time_t pi_time = time(nullptr);
                           if (state != 0)
                               Q.push_pi((str_time) {s, pi_time});
                       }
                   }

                   cout << "Receive thread stop." << endl;

                   alive_num_mtx.lock();
                   this->alive_thread_num--;
                   alive_num_mtx.unlock();
                   alive_num_cv.notify_all();
               }).detach();
        // ��ʱ�����߳�
        thread([this]
               {
                   alive_num_mtx.lock();
                   this->alive_thread_num++;
                   alive_num_mtx.unlock();

                   int seq_num = 0;
                   ofstream log;
                   log.open(this->log_file.data(), ios_base::out);
                   if (!log.is_open())
                   {
                       cout << "error open " << this->log_file << endl;
                   }
                   while (true)
                   {
                       // cout << "send thread start" << endl;
                       while (state != 1)
                       {
                           unique_lock<mutex> lock(mtx);
                           if (state == -1)          // state=-1������ѭ��
                               break;
                           cv.wait(lock);
                       }
                       if (state == -1)             // state=-1������ѭ��
                           break;
                       string ss = rout.to_string();
                       p->send_to_nei(ss);
                       log << rout.log(seq_num) << endl;
                       seq_num++;
                       // cout << "sent success" << endl;

                       Sleep(frequency);
                   }
                   log.close();
                   cout << "Send thread stop." << endl;

                   alive_num_mtx.lock();
                   this->alive_thread_num--;
                   alive_num_mtx.unlock();
                   alive_num_cv.notify_all();
               }).detach();
        // ��ʱ��鳬ʱ����
        thread([this]
               {
                   alive_num_mtx.lock();
                   this->alive_thread_num++;
                   alive_num_mtx.unlock();

                   bool all_dead;

                   while (true)
                   {
                       // cout << "timeout thread start" << endl;
                       while (state != 1)
                       {
                           unique_lock<mutex> lock(mtx);
                           if (state == -1)          // state=-1������ѭ��
                               break;
                           cv.wait(lock);
                       }
                       if (state == -1)          // state=-1������ѭ��
                           break;
                       time_t now = time(nullptr);
                       // ���� timeout ��鳬ʱ
                       timeout_mtx.lock();
                       auto it = timeout.begin();
                       auto itEnd = timeout.end();
                       while (it != itEnd)
                       {
                           if (now > it->second)
                           {
                               cout << it->first << "->" << id << " timeout." << endl;
                               p->disconnect(it->first);
                               // ��Ϊ���ɴ�
                               rout.set_unreachable(it->first);
                           }
                           it++;
                       }
                       timeout_mtx.unlock();
                       Sleep(maxtime);
                   }
                   cout << "Timeout thread stop." << endl;

                   alive_num_mtx.lock();
                   this->alive_thread_num--;
                   alive_num_mtx.unlock();
                   alive_num_cv.notify_all();
               }).detach();
        // ������յ���·����Ϣ
        thread([this]
               {
                   alive_num_mtx.lock();
                   this->alive_thread_num++;
                   alive_num_mtx.unlock();

                   while (true)
                   {
                       // cout << "process thread start" << endl;
                       while (state != 1)
                       {
                           unique_lock<mutex> lock(mtx);
                           if (state == -1)          // state=-1������ѭ��
                               break;
                           if (state == 0)
                               Q.clear();
                           cv.wait(lock);
                       }
                       if (state == -1)          // state=-1������ѭ��
                           break;

                       /* ********************************* Bug *****************************************
                        *      desc: ������������k�������߳̽�������ʱ�������Ϊ�գ���ô�get_pi�����߳����������˳���
                        *      fix: stop��������Ϣ����Q��pushһ��������Ϣ������process�߳�
                        * ******************************************************************************/

                       // �Ӷ����л�ȡ����
                       str_time task = Q.get_pi();

                       // ����Ƿ�Ϊ��ֹ����
                       if (task.first == "over")
                           break;

                       // ���ķ����л�
                       string sender;
                       vector<pair<string, float> > recv_table = str2table(task.first, sender);

                       // cout << "recv " << sender << endl;

                       // �����ھӵĳ�ʱʱ��
                       timeout_mtx.lock();
                       timeout[sender] = task.second + maxtime / 1000;
                       timeout_mtx.unlock();

                       // ȷ������״̬
                       p->connect(sender);

                       // ����·�ɱ�
                       rout.update_history(sender, recv_table);
                       rout.update_rout();
                   }

                   cout << "Process thread stop." << endl;

                   alive_num_mtx.lock();
                   this->alive_thread_num--;
                   alive_num_mtx.unlock();
                   alive_num_cv.notify_all();
               }).join();
    }

    //����ڽӱ�
    void debug()
    {
        rout.debug();
    }

    //·�ɹ���
    void fault()
    {
        state = 0;
        Q.clear();
        cout << "The router paused." << endl;
    }

    //����·��
    void stop()
    {
        state = -1;
        cv.notify_all();
        // ����Ϣ����Q�����over��Ϣ����ֹProcess�߳�
        str_time over("over", 0);
        Q.push_pi(over);

        unique_lock<mutex> lock(alive_num_mtx);
        alive_num_cv.wait(lock, [this]
        { return this->alive_thread_num == 0; });
    }

    //����
    void restart()
    {
        if (state == 1)
        {
            cout << "The router is running." << endl;
            return;
        }
        // ���³�ʼ��
        char in[500];
        ifstream f(init_file.data());
        while (f.getline(in, 1000))
        {
            auto x = split(in, set<char>({' ', ':', '\t'}));
            if (x.size() < 3)
            {
                cout << "Initial file content error" << endl;
                continue;
            }
            p->connect(x[0]);
            // ��¼ÿ���ھӵĳ�ʱʱ��
            timeout_mtx.lock();
            timeout[x[0]] = time(nullptr) + maxtime / 1000;
            timeout_mtx.unlock();
            // ��ӵ�·�ɱ�
            rout.update_history(x[0], x[0], atof(x[1].data()));
        }
        rout.update_rout();
        cout << "The router reboot." << endl;
        // �����Ϣ����
        Q.clear();
        state = 1;
        cv.notify_all();
    }
};
