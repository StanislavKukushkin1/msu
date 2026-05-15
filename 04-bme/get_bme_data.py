import time
import serial
import matplotlib.pyplot as plt
import serial.tools.list_ports
from datetime import datetime
import re

def find_pico_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "USB Serial" in port.description or "Pico" in port.description:
            print(f"Selected Pico on {port.device}")
            return port.device
    return "COM13"

def send_command_and_read(ser, command):
    """Отправить команду и прочитать ответ"""
    # Очищаем буфер
    while ser.in_waiting:
        ser.readline()
    
    # Отправляем команду
    ser.write(f"{command}\n".encode('ascii'))
    time.sleep(0.2)
    
    # Читаем все строки ответа
    responses = []
    timeout = time.time() + 0.5
    while time.time() < timeout:
        if ser.in_waiting:
            line = ser.readline().decode('ascii', errors='ignore').strip()
            if line:
                # Пропускаем приглашение "> " и пустые строки
                if line != ">" and line != "" and not line.startswith("received string"):
                    responses.append(line)
        time.sleep(0.01)
    
    # Возвращаем последнюю непустую строку (где обычно число)
    for resp in reversed(responses):
        if resp and not resp.startswith("Command:") and not resp.startswith("Args:"):
            return resp
    return None

def extract_number(text):
    """Извлечь число из строки"""
    if not text:
        return None
    # Ищем число
    match = re.search(r'[-+]?\d*\.?\d+', text)
    if match:
        return float(match.group())
    return None

def main():
    timestamps = []
    temperatures = []
    pressures = []
    humidities = []
    
    pico_port = find_pico_port()
    print(f"Using port: {pico_port}")
    
    try:
        ser = serial.Serial(port=pico_port, baudrate=115200, timeout=1.0)
        print(f"Connected to {pico_port}")
        time.sleep(2)
        
        # Очищаем буфер
        while ser.in_waiting:
            ser.readline()
        
        print("\n" + "="*60)
        print("BME280 Data Logger")
        print("Press Ctrl+C to stop and plot graphs")
        print("Breathe on the sensor to see changes!")
        print("="*60)
        
        print("\nCollecting data...")
        print("-"*60)
        
        start_time = time.time()
        sample_count = 0
        
        while True:
            # Читаем температуру
            temp_line = send_command_and_read(ser, "temp")
            temp = extract_number(temp_line) if temp_line else None
            
            # Читаем давление
            press_line = send_command_and_read(ser, "pres")
            press = extract_number(press_line) if press_line else None
            
            # Читаем влажность
            hum_line = send_command_and_read(ser, "hum")
            hum = extract_number(hum_line) if hum_line else None
            
            if temp is not None and press is not None and hum is not None:
                current_time = time.time() - start_time
                timestamps.append(current_time)
                temperatures.append(temp)
                pressures.append(press)
                humidities.append(hum)
                sample_count += 1
                
                print(f"{sample_count:3d}: T={temp:6.2f}°C  P={press:8.2f}Pa  H={hum:6.2f}%  t={current_time:5.1f}s")
            else:
                print(f"  Failed: temp={temp_line}, press={press_line}, hum={hum_line}")
            
            time.sleep(1.0)
                
    except KeyboardInterrupt:
        print("\n\nData collection stopped by user")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Port closed")
        
        if len(timestamps) > 0:
            print(f"\nCollected {len(timestamps)} samples")
            print("Plotting graphs...")
            
            fig, axes = plt.subplots(3, 1, figsize=(12, 10))
            
            axes[0].plot(timestamps, temperatures, 'r-', linewidth=1.5, marker='o', markersize=3)
            axes[0].set_title('Temperature over time', fontsize=14)
            axes[0].set_ylabel('Temperature (°C)', fontsize=12)
            axes[0].grid(True, alpha=0.3)
            if len(temperatures) > 1:
                avg_temp = sum(temperatures) / len(temperatures)
                axes[0].axhline(y=avg_temp, color='r', linestyle='--', alpha=0.5, 
                               label=f'Average: {avg_temp:.2f}°C')
                axes[0].legend()
            
            axes[1].plot(timestamps, pressures, 'b-', linewidth=1.5, marker='s', markersize=3)
            axes[1].set_title('Pressure over time', fontsize=14)
            axes[1].set_ylabel('Pressure (Pa)', fontsize=12)
            axes[1].grid(True, alpha=0.3)
            if len(pressures) > 1:
                avg_press = sum(pressures) / len(pressures)
                axes[1].axhline(y=avg_press, color='b', linestyle='--', alpha=0.5,
                               label=f'Average: {avg_press:.0f} Pa')
                axes[1].legend()
            
            axes[2].plot(timestamps, humidities, 'g-', linewidth=1.5, marker='^', markersize=3)
            axes[2].set_title('Humidity over time', fontsize=14)
            axes[2].set_xlabel('Time (seconds)', fontsize=12)
            axes[2].set_ylabel('Humidity (%)', fontsize=12)
            axes[2].grid(True, alpha=0.3)
            if len(humidities) > 1:
                avg_hum = sum(humidities) / len(humidities)
                axes[2].axhline(y=avg_hum, color='g', linestyle='--', alpha=0.5,
                               label=f'Average: {avg_hum:.2f}%')
                axes[2].legend()
            
            plt.tight_layout()
            
            filename = f"bme280_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.png"
            plt.savefig(filename, dpi=150)
            print(f"Graph saved as {filename}")
            
            plt.show()
        else:
            print("\nNo data collected.")
            print("\nFirst, test manually in a terminal:")
            print("  1. Open PuTTY or Arduino Serial Monitor")
            print("  2. Connect to COM8 at 115200 baud")
            print("  3. Type: version")
            print("  4. Type: temp")
            print("  5. Type: pres")
            print("  6. Type: hum")

if __name__ == "__main__":
    main()