import time
import serial
from PIL import Image

def main():

    PORT = 'COM13' 
    BAUD = 115200  # Стандартная скорость для USB-Serial
    IMAGE_PATH = "C:\\Repositories\\pico\\msu\\05-display\\image.png"

    DISPLAY_W = 320
    DISPLAY_H = 240

    try:
        # Открываем соединение
        ser = serial.Serial(port=PORT, baudrate=BAUD, timeout=1.0)
        print(f"Порт {ser.name} открыт. Начинаю подготовку изображения...")

        # Открываем и подготавливаем изображение
        img = Image.open(IMAGE_PATH).convert('RGB')
        
        # КЛЮЧЕВОЙ МОМЕНТ: Сжатие под размер экрана
        # Метод LANCZOS обеспечивает лучшее качество при уменьшении
        img = img.resize((DISPLAY_W, DISPLAY_H), Image.Resampling.LANCZOS)
        
        width, height = img.size
        print(f"Изображение готово к отправке ({width}x{height})")

        # Очищаем экран черным цветом перед началом
        ser.write("disp_screen 000000\n".encode('ascii'))
        time.sleep(0.1)

        start_time = time.time()

        # Цикл отрисовки
        for y in range(height):
            for x in range(width):
                r, g, b = img.getpixel((x, y))
                
                # Формируем команду в формате: disp_px x y RRGGBB
                command = f"disp_px {x} {y} {r:02x}{g:02x}{b:02x}\n"
                ser.write(command.encode('ascii'))
            
            # Выводим прогресс каждые 10 строк
            if y % 10 == 0:
                percent = int((y / height) * 100)
                print(f"Загрузка: {percent}% (строка {y}/{height})")

        duration = time.time() - start_time
        print(f"Готово! Время загрузки: {duration:.2f} сек.")

    except FileNotFoundError:
        print(f"Ошибка: Файл {IMAGE_PATH} не найден в папке со скриптом.")
    except Exception as e:
        print(f"Произошла ошибка: {e}")

    finally:
        # Пауза, чтобы последние данные успели уйти из буфера
        time.sleep(0.1)
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Порт закрыт.")

if __name__ == "__main__":
    main()