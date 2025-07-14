#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <ctime>
#include <unordered_set>

using namespace std;

// Структура для хранения данных о событии
struct LogEntry {
    tm date;          // Время события
    int user;         // ID пользователя
    int event_type;   // Тип события
    string parameter; // Параметр события
};

// Функция чтения данных из лог-файла
vector<LogEntry> read_log(const string& filename) {
    vector<LogEntry> entries;
    ifstream file(filename);
    string line;
    
    // Пропускаем строку с заголовками
    getline(file, line);
    
    // Читаем файл построчно
    while (getline(file, line)) {
        // Разбиваем строку на части по запятым
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        size_t pos3 = line.find(',', pos2 + 1);
        
        LogEntry entry;
        
        // Парсим дату из строки в структуру tm
        string date_str = line.substr(0, pos1);
        strptime(date_str.c_str(), "%Y-%m-%d_%H:%M:%S", &entry.date);
        
        // Извлекаем ID пользователя
        entry.user = stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
        // Извлекаем тип события
        entry.event_type = stoi(line.substr(pos2 + 1, pos3 - pos2 - 1));
        // Извлекаем параметр события
        entry.parameter = line.substr(pos3 + 1);
        
        entries.push_back(entry);
    }
    
    return entries;
}

// Функция подсчета сессий пользователей
pair<int, vector<LogEntry>> count_sessions() {
    // Читаем данные из файла
    auto entries = read_log("log.csv");
    
    // Сортируем записи по пользователю и времени
    sort(entries.begin(), entries.end(), [](const LogEntry& a, const LogEntry& b) {
        if (a.user != b.user) return a.user < b.user;
        return difftime(mktime(const_cast<tm*>(&a.date)), mktime(const_cast<tm*>(&b.date))) < 0;
    });
    
    // Целевая дата для подсчета сессий (2020-04-19)
    tm target_date = {};
    strptime("2020-04-19", "%Y-%m-%d", &target_date);
    int sessions_count = 0;
    
    // Подсчитываем сессии
    for (size_t i = 0; i < entries.size(); ++i) {
        bool new_session = false;
        
        // Проверяем, начинается ли новая сессия:
        // 1. Это первая запись для пользователя
        // 2. Прошло больше 30 минут с предыдущего события
        if (i == 0 || entries[i].user != entries[i-1].user) {
            new_session = true;
        } else {
            double diff = difftime(mktime(&entries[i].date), mktime(&entries[i-1].date));
            if (diff >= 1800) { // 1800 секунд = 30 минут
                new_session = true;
            }
        }
        
        // Если новая сессия началась в целевую дату, увеличиваем счетчик
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
    // Задание 1: Подсчет количества сессий, начавшихся 2020-04-19
    auto [sessions_count, data] = count_sessions();
    cout << "1. Количество сессий: " << sessions_count << endl;
    
    // Задание 2: Поиск дня с максимальным числом уникальных пользователей, смотревших видео
    // (события с типом 2 и параметром "video")
    map<string, unordered_set<int>> day_users;
    
    for (const auto& entry : data) {
        if (entry.event_type == 2 && entry.parameter == "video") {
            // Преобразуем дату в строку формата "YYYY-MM-DD"
            char day_str[11];
            strftime(day_str, sizeof(day_str), "%Y-%m-%d", &entry.date);
            // Добавляем пользователя в множество для этого дня
            day_users[day_str].insert(entry.user);
        }
    }
    
    // Находим максимальное количество уникальных пользователей
    size_t max_users = 0;
    for (const auto& [day, users] : day_users) {
        if (users.size() > max_users) {
            max_users = users.size();
        }
    }
    cout << "2. Максимум уникальных пользователей: " << max_users << endl;
    
    // Задание 3: Поиск 5-минутного интервала с максимальным числом событий
    vector<tm> times;
    for (const auto& entry : data) {
        times.push_back(entry.date);
    }
    
    // Сортируем временные метки
    sort(times.begin(), times.end(), [](const tm& a, const tm& b) {
        return difftime(mktime(const_cast<tm*>(&a)), mktime(const_cast<tm*>(&b))) < 0;
    });
    
    size_t max_count = 0;    // Максимальное количество событий
    size_t left = 0;         // Левая граница окна
    tm best_start = {};      // Время начала лучшего интервала
    
    // Используем алгоритм скользящего окна
    for (size_t right = 0; right < times.size(); ++right) {
        // Сдвигаем левую границу, если интервал превышает 5 минут (300 секунд)
        while (difftime(mktime(&times[right]), mktime(&times[left])) > 300) {
            left++;
        }
        
        // Вычисляем количество событий в текущем окне
        size_t count = right - left + 1;
        
        // Обновляем максимальный интервал
        if (count > max_count) {
            max_count = count;
            best_start = times[left];
        }
    }
    
    // Форматируем результат для вывода
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H:%M:%S", &best_start);
    cout << "3. Начало наиболее активного 5-минутного интервала: " << buffer << endl;
    
    return 0;
}