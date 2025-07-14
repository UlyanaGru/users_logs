#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <ctime>
#include <unordered_set>

using namespace std;

struct LogEntry {
    tm date;
    int user;
    int event_type;
    string parameter;
};

vector<LogEntry> read_log(const string& filename) {
    vector<LogEntry> entries;
    ifstream file(filename);
    string line;
    
    // Пропуск заголовка
    getline(file, line);
    
    while (getline(file, line)) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        size_t pos3 = line.find(',', pos2 + 1);
        
        LogEntry entry;
        
        //Поиск даты
        string date_str = line.substr(0, pos1);
        strptime(date_str.c_str(), "%Y-%m-%d_%H:%M:%S", &entry.date);
        
        // Поиск других параметров
        entry.user = stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
        entry.event_type = stoi(line.substr(pos2 + 1, pos3 - pos2 - 1));
        entry.parameter = line.substr(pos3 + 1);
        
        entries.push_back(entry);
    }
    
    return entries;
}

pair<int, vector<LogEntry>> count_sessions() {
    auto entries = read_log("log.csv");
    
    // Сортировка по пользователю и дате
    sort(entries.begin(), entries.end(), [](const LogEntry& a, const LogEntry& b) {
        if (a.user != b.user) return a.user < b.user;
        return difftime(mktime(const_cast<tm*>(&a.date)), mktime(const_cast<tm*>(&b.date))) < 0;
    });
    
    // Количество сессий
    int sessions_count = 0;
    tm target_date = {};
    strptime("2020-04-19", "%Y-%m-%d", &target_date);
    
    for (size_t i = 0; i < entries.size(); ++i) {
        bool new_session = false;
        
        if (i == 0 || entries[i].user != entries[i-1].user) {
            new_session = true;
        } else {
            double diff = difftime(mktime(&entries[i].date), mktime(&entries[i-1].date));
            if (diff >= 1800) { // 30 минут в секунды
                new_session = true;
            }
        }
        
        if (new_session && 
            entries[i].date.tm_year == target_date.tm_year &&
            entries[i].date.tm_mon == target_date.tm_mon &&
            entries[i].date.tm_mday == target_date.tm_mday) {
            sessions_count++;
        }
    }
    
    return {sessions_count, entries};
}

int main() {
    // Task 1: Количество сессий
    auto [sessions_count, data] = count_sessions();
    cout << "1. Количество сессий: " << sessions_count << endl;
    
    // Task 2: Максимальное количество уникальных пользователей
    map<string, unordered_set<int>> day_users;
    for (const auto& entry : data) {
        if (entry.event_type == 2 && entry.parameter == "video") {
            char day_str[11];
            strftime(day_str, sizeof(day_str), "%Y-%m-%d", &entry.date);
            day_users[day_str].insert(entry.user);
        }
    }
    
    size_t max_users = 0;
    for (const auto& [day, users] : day_users) {
        if (users.size() > max_users) {
            max_users = users.size();
        }
    }
    cout << "2. Максимум уникальных пользователей: " << max_users << endl;
    
    // Task 3: Самый активный 5-и минутный промежуток
    vector<tm> times;
    for (const auto& entry : data) {
        times.push_back(entry.date);
    }
    sort(times.begin(), times.end(), [](const tm& a, const tm& b) {
        return difftime(mktime(const_cast<tm*>(&a)), mktime(const_cast<tm*>(&b))) < 0;
    });
    
    size_t max_count = 0;
    size_t left = 0;
    tm best_start = {};
    
    for (size_t right = 0; right < times.size(); ++right) {
        while (difftime(mktime(&times[right]), mktime(&times[left])) > 300) { // 5 минут в сенудах
            left++;
        }
        size_t count = right - left + 1;
        if (count > max_count) {
            max_count = count;
            best_start = times[left];
        }
    }
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H:%M:%S", &best_start);
    cout << "3. Начало наиболее активного 5-минутного интервала: " << buffer << endl;
    
    return 0;
}