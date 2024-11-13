import serial
import time
import keyboard

# DEKLARASI PORT
port = 'COM12'  # Lihat nama port dalam Arduino IDE: Tools > Port:
baud = 115200  # Default 115200
timeout = None  # Biarkan seperti ini

try:
    ser = serial.Serial(port, baud, timeout=timeout)
    time.sleep(1)
except serial.SerialException as e:
    print(f"Error: Tidak dapat membuka port {port}. Pastikan port sudah benar dan coba lagi.")
    exit()

# DAFTAR PERINTAH YANG AKAN DIEKSEKUSI OLEH ROBOT, UBAH SESUAI KEBUTUHAN
cmdList1 = [
    "G0 X0 Y220 Z180 E0",
    "G0 X80 Y255 Z150 E0",
    # "G0 X80 Y255 Z85 E0 F30",
    # "M6",
    # "G0 X80 Y255 Z150 E0 F30",
    # "G0 X0 Y220 Z180 E0",
    # "G0 X0 Y220 Z180 E280 F50",
    # "G0 X-295 Y110 Z100 E280",
    # "G0 X-295 Y110 Z50 E280 F30",
    # "M7",
    # "G0 X-295 Y110 Z100 E280 F30",
    # "G0 X0 Y220 Z180 E280",
    # "G0 X0 Y220 Z180 E0 F50",
]

cmdList2 = [
    "G0 X0 Y220 Z180 E0",
    "G0 X-105 Y250 Z150 E0",
    # "G0 X-105 Y250 Z85 E0 F30",
    # "M6",
    # "G0 X-105 Y250 Z150 E0 F30",
    # "G0 X0 Y220 Z180 E0",
    # "G0 X0 Y220 Z180 E280 F50",
    # "G0 X-310 Y0 Z100 E280",
    # "G0 X-310 Y0 Z50 E280 F30",
    # "M7",
    # "G0 X-310 Y0 Z100 E280 F30",
    # "G0 X0 Y220 Z180 E280",
    # "G0 X0 Y220 Z180 E0 F50",
]

# Encode commands for serial transmission
bCmdList1 = [f"{cmd}\r".encode('utf-8') for cmd in cmdList1]
bCmdList2 = [f"{cmd}\r".encode('utf-8') for cmd in cmdList2]

def send_command(command_list):
    """Kirim daftar perintah satu per satu dan tunggu hingga selesai."""
    for cmd in command_list:
        ser.write(cmd)
        tunggu_selesai()

def tunggu_selesai():
    """Menunggu hingga robot mengirim sinyal 'ok' yang menandakan tugas selesai."""
    while True:
        response = ser.readline()
        print(response.decode('utf-8').strip())
        if "ok" in response.decode("utf-8"):
            break

try:
    # ROBOT MELAKUKAN HOME
    ser.write(b'G28\r')
    print("\nHome sedang dalam proses\n")
    time.sleep(3)
    
    # Mencetak hasil homing
    for _ in range(2):
        print(ser.readline().decode('utf-8').strip())

    # Memulai operasi robot berdasarkan input pengguna
    mulai = input('Tekan y untuk mulai: ').strip().lower()
    if mulai == 'y':
        print("\nRobot Mulai Bekerja\n")
        print("Tahan q untuk berhenti")

        while True:
            data = ser.readline().decode('utf-8').strip()
            print(f"Data yang diterima: {data}")

            if "S1 OFF" in data:
                print("S1 NONMETAL terdeteksi, menjalankan cmdList1")
                send_command(bCmdList1)
            
            elif "S2 OFF" in data:
                print("S2 METAL terdeteksi, menjalankan cmdList2")
                send_command(bCmdList2)

            if keyboard.is_pressed('q'):
                print("\nRobot Berhenti\n")
                ser.write(b'G0 X0 Y140 Z31\r')
                time.sleep(1)
                print("Stepper akan off dalam 3 detik\n")
                time.sleep(3)
                ser.write(b'M18\r')
                tunggu_selesai()
                break

except Exception as e:
    print(f"Error terjadi: {e}")
finally:
    if ser.is_open:
        ser.close()
        print("Port serial telah ditutup.")
