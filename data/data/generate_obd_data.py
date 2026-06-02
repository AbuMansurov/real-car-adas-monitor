import csv
import random

# Создаем файл в папке data/
with open('data/obd_data.csv', 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    # Заголовки столбцов (строго как в задании)
    writer.writerow(['speed_kmh', 'engine_rpm', 'throttle_pos', 'coolant_temp', 'fuel_level', 'intake_air_temp', 'label'])
    
    # Генерируем 5000 строк
    for _ in range(5000):
        # Случайный выбор стиля вождения
        label = random.choice(['SLOW', 'NORMAL', 'AGGRESSIVE'])
        
        if label == 'SLOW':
            speed = random.randint(20, 60)
            rpm = random.randint(1000, 2500)
            throttle = random.randint(5, 30)
        elif label == 'NORMAL':
            speed = random.randint(60, 90)
            rpm = random.randint(2500, 3500)
            throttle = random.randint(30, 60)
        else:  # AGGRESSIVE
            speed = random.randint(90, 140)
            rpm = random.randint(3500, 6000)
            throttle = random.randint(60, 100)
        
        # Остальные параметры (реалистичные значения)
        coolant_temp = random.randint(80, 110)
        fuel_level = random.randint(10, 100)
        intake_air_temp = random.randint(15, 40)
        
        writer.writerow([speed, rpm, throttle, coolant_temp, fuel_level, intake_air_temp, label])

print("✅ Успешно сгенерировано 5000 строк в data/obd_data.csv")