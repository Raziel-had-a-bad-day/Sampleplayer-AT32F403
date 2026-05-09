#!/usr/bin/env python3
import sys
import os
import time
import serial
import subprocess
import threading
import tkinter as tk
from tkinter import Menu, messagebox, simpledialog
from tkinterdnd2 import *

# ================== DEFAULT CONFIG ==================
config = {
    "port": "/dev/ttyACM1",
    "baud": 115200,
    "pause": 3,
    "samplerate": 44100
}

ser = None
rx_running = False

# ====================== FUNCTIONS ======================
def serial_rx_listener():
    global rx_running
    while rx_running and ser and ser.is_open:
        try:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if line:
                    print(f"RX: {line}")
        except:
            break

def send_file(filepath):
    try:
        print(f"→ Sending: {os.path.basename(filepath)}")
        temp_raw = "/tmp/temp.raw"
        
        subprocess.run(["ffmpeg", "-y", "-i", filepath,
                        "-ac", "1", "-ar", str(config["samplerate"]),
                        "-sample_fmt", "s16", "-f", "s16le", temp_raw],
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        with open(temp_raw, "rb") as f:
            data = f.read()
            ser.write(data)
            ser.flush()

        print(f"   Sent {len(data):,} bytes")
        os.remove(temp_raw)
    except Exception as e:
        print(f"   Error: {e}")

def process_files(file_list):
    global ser, rx_running
    if not file_list: return

    try:
        ser = serial.Serial(config["port"], config["baud"], timeout=0.1)
        print(f"Connected → {config['port']} @ {config['baud']}\n")
        
        rx_running = True
        threading.Thread(target=serial_rx_listener, daemon=True).start()

        for i, f in enumerate(file_list):
            send_file(f)
            if i < len(file_list)-1:
                print(f"Waiting {config['pause']} seconds...\n")
                time.sleep(config['pause'])

        print("=== All files sent ===\n")
    except Exception as e:
        messagebox.showerror("Error", str(e))
    finally:
        rx_running = False
        if ser and ser.is_open:
            ser.close()

# ====================== RIGHT CLICK MENU ======================
def show_context_menu(event):
    menu = Menu(root, tearoff=0)
    menu.add_command(label="Settings...", command=edit_settings)
    menu.add_separator()
    menu.add_command(label="Exit", command=root.quit)
    menu.post(event.x_root, event.y_root)

def edit_settings():
    global config
    new_port = simpledialog.askstring("Port", "Serial Port:", initialvalue=config["port"])
    new_baud = simpledialog.askinteger("Baudrate", "Baud Rate:", initialvalue=config["baud"])
    new_pause = simpledialog.askinteger("Pause", "Pause between files (seconds):", initialvalue=config["pause"])
    new_sr = simpledialog.askinteger("Sample Rate", "Sample Rate (Hz):", initialvalue=config["samplerate"])

    if new_port: config["port"] = new_port
    if new_baud: config["baud"] = new_baud
    if new_pause is not None: config["pause"] = new_pause
    if new_sr: config["samplerate"] = new_sr

    status_label.config(text=f"Port: {config['port']}  |  {config['baud']} baud  |  Pause: {config['pause']}s")

# ====================== GUI ======================
root = TkinterDnD.Tk()
root.title("Serial Raw Sender")
root.geometry("500x220")
root.resizable(False, False)

# Right-click binding
root.bind("<Button-3>", show_context_menu)   # Right click

tk.Label(root, text="Drag & Drop Audio Files Here", font=("Arial", 14, "bold"), pady=20).pack()

status_label = tk.Label(root, text=f"Port: {config['port']}  |  {config['baud']} baud  |  Pause: {config['pause']}s", 
                        fg="gray", font=("Arial", 10))
status_label.pack(pady=10)

tk.Label(root, text="Right-click window for Settings", fg="darkgray", font=("Arial", 9)).pack(pady=5)

root.drop_target_register(DND_FILES)
root.dnd_bind('<<Drop>>', lambda e: process_files(root.tk.splitlist(e.data)))

root.mainloop()
