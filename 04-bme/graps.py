import serial
import time
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque
import threading
import re

# Настройки
SERIAL_PORT = 'COM11'  # Измените на ваш порт
BAUDRATE = 115200
MAX_POINTS = 100  # Количество точек на графике
UPDATE_INTERVAL = 2000  # Интервал обновления в миллисекундах (2 секунды)

# Хранение данных
time_data = deque(maxlen=MAX_POINTS)
temp_data = deque(maxlen=MAX_POINTS)
pres_data = deque(maxlen=MAX_POINTS)
hum_data = deque(maxlen=MAX_POINTS)
counter = 0

# Глобальная переменная для serial соединения
ser = None

def connect_serial():
    """Подключение к Serial порту"""
    global ser
    try:
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=2)
        time.sleep(2)  # Даем время на инициализацию
        print(f"Connected to {SERIAL_PORT}")
        return True
    except Exception as e:
        print(f"Error connecting to {SERIAL_PORT}: {e}")
        return False

def read_bme280_data():
    """Отправляет команды и читает данные с BME280"""
    global ser
    
    if not ser or not ser.is_open:
        return None
    
    try:
        # Очищаем буфер перед отправкой
        ser.reset_input_buffer()
        
        # Запрашиваем температуру
        ser.write(b"temp\n")
        time.sleep(0.1)
        
        temp_line = ser.readline().decode('utf-8', errors='ignore').strip()
        temperature = None
        if "Temperature:" in temp_line:
            # Парсим: "Temperature: 23.45 °C"
            match = re.search(r"Temperature:\s*([\d.-]+)", temp_line)
            if match:
                temperature = float(match.group(1))
        
        # Запрашиваем давление
        ser.write(b"pres\n")
        time.sleep(0.1)
        
        pres_line = ser.readline().decode('utf-8', errors='ignore').strip()
        pressure = None
        if "Pressure:" in pres_line:
            # Парсим: "Pressure: 101325.00 Pa (1013.25 hPa)"
            match = re.search(r"Pressure:\s*([\d.-]+)", pres_line)
            if match:
                pressure = float(match.group(1))
        
        # Запрашиваем влажность
        ser.write(b"hum\n")
        time.sleep(0.1)
        
        hum_line = ser.readline().decode('utf-8', errors='ignore').strip()
        humidity = None
        if "Humidity:" in hum_line:
            # Парсим: "Humidity: 45.67 %"
            match = re.search(r"Humidity:\s*([\d.-]+)", hum_line)
            if match:
                humidity = float(match.group(1))
        
        return {
            'temperature': temperature,
            'pressure': pressure,
            'humidity': humidity
        }
        
    except Exception as e:
        print(f"Error reading data: {e}")
        return None

def update_data():
    """Функция для обновления данных в отдельном потоке"""
    global counter, time_data, temp_data, pres_data, hum_data
    
    while True:
        data = read_bme280_data()
        
        if data and data['temperature'] is not None:
            counter += 1
            current_time = counter * (UPDATE_INTERVAL / 1000)  # время в секундах
            
            time_data.append(current_time)
            temp_data.append(data['temperature'])
            pres_data.append(data['pressure'] / 100.0)  # переводим в гПа (гектопаскали)
            hum_data.append(data['humidity'])
            
            print(f"[{current_time:.1f}s] T: {data['temperature']:.2f}°C, "
                  f"P: {data['pressure']/100:.1f} hPa, "
                  f"H: {data['humidity']:.1f}%")
        else:
            print("Failed to read data")
        
        time.sleep(UPDATE_INTERVAL / 1000)

def animate(i):
    """Функция обновления графиков"""
    plt.clf()
    
    # График температуры
    plt.subplot(3, 1, 1)
    plt.plot(list(time_data), list(temp_data), 'r-', linewidth=2)
    plt.title('Temperature', fontsize=12)
    plt.ylabel('Temperature (°C)', fontsize=10)
    plt.grid(True, alpha=0.3)
    if time_data:
        plt.xlim(min(time_data), max(time_data) + 1)
    if temp_data:
        plt.ylim(min(temp_data) - 2, max(temp_data) + 2)
    
    # График давления
    plt.subplot(3, 1, 2)
    plt.plot(list(time_data), list(pres_data), 'b-', linewidth=2)
    plt.title('Pressure', fontsize=12)
    plt.ylabel('Pressure (hPa)', fontsize=10)
    plt.grid(True, alpha=0.3)
    if time_data:
        plt.xlim(min(time_data), max(time_data) + 1)
    if pres_data:
        plt.ylim(min(pres_data) - 5, max(pres_data) + 5)
    
    # График влажности
    plt.subplot(3, 1, 3)
    plt.plot(list(time_data), list(hum_data), 'g-', linewidth=2)
    plt.title('Humidity', fontsize=12)
    plt.xlabel('Time (seconds)', fontsize=10)
    plt.ylabel('Humidity (%)', fontsize=10)
    plt.grid(True, alpha=0.3)
    if time_data:
        plt.xlim(min(time_data), max(time_data) + 1)
    if hum_data:
        plt.ylim(0, 100)
    
    plt.tight_layout()

def main():
    """Основная функция"""
    print("=== BME280 Data Logger ===")
    print(f"Port: {SERIAL_PORT}")
    print(f"Baudrate: {BAUDRATE}")
    print(f"Update interval: {UPDATE_INTERVAL} ms")
    print("Press Ctrl+C to stop")
    print("-" * 40)
    
    # Подключаемся к Serial
    if not connect_serial():
        return
    
    # Запускаем поток для сбора данных
    data_thread = threading.Thread(target=update_data, daemon=True)
    data_thread.start()
    
    # Настраиваем график
    plt.ion()  # Включаем интерактивный режим
    fig = plt.figure(figsize=(12, 8))
    plt.subplots_adjust(hspace=0.4)
    
    try:
        while True:
            animate(0)
            plt.pause(0.1)
            
            # Проверяем, закрыто ли окно
            if not plt.fignum_exists(fig.number):
                print("\nGraph window closed. Exiting...")
                break
                
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Serial port closed")
        plt.close()

if __name__ == "__main__":
    main()