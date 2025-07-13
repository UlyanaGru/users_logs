import pandas as pd
from datetime import datetime, timedelta

def count_sessions():
    '''Функция подсчета количества сессий'''
    # Открыть файл для чтения
    with open('log.csv', 'r') as f:
        # Чтение файла формата csv
        df = pd.read_csv(f)
        # Преобразование времени в формат datetime
        df['date'] = pd.to_datetime(df['date'], format='%Y-%m-%d_%H:%M:%S')
        # Сортировка по пользователю и времени
        df = df.sort_values(['user', 'date'])
        # Вычисление разницы между событиями пользователя
        df['time_diff'] = df.groupby('user')['date'].diff()
        # Определяем начало новых сессий (первый эвент пользователя или разница >= 30 минут)
        df['new_session'] = (df['time_diff'].isna()) | (df['time_diff'] >= timedelta(minutes=30))
        # Считаем сессии, начавшиеся 2020-04-19
        target_date = pd.to_datetime('2020-04-19').date()
        # Условие на новую сессию пользователя в нужную дату
        sessions_count = df[df['new_session'] & (df['date'].dt.date == target_date)].shape[0]
        return sessions_count, df

# Вызов функции и вывод результатов
if __name__ == "__main__":
    print('1. Количество сессий:', count_sessions()[0])
    
    data = count_sessions()[1]
    video = data.loc[(data['event_type'] == 2) & (data['parameter'] == 'video')].copy()
    video.loc[:, 'day'] = video['date'].dt.date
    print('2. Максимум уникальных пользователей:', video.groupby('day')['user'].nunique().max())

    times = data['date'].sort_values().tolist()
    max_count = left = 0
    best_start = None
    
    for right, time in enumerate(times):
        while time - times[left] >= timedelta(minutes=5):
            left += 1
        if (count := right - left + 1) >= max_count:
            max_count, best_start = count, times[left]
    
    print('3. Начало наиболее активного 5-минутного интервала:', best_start.strftime('%Y-%m-%d_%H:%M:%S'))