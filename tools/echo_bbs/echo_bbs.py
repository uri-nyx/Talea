import socket
import sys

# Configuration
HOST = '127.0.0.1'
PORT = 9000

print(f"--- MOCK BBS LISTENING ON {HOST}:{PORT} ---")

try:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print(f"Connection from {addr}")
            conn.sendall(b"Welcome to TALEA TEST BBS!\r\n")
            conn.sendall(b"Echo Mode Enabled. Type 'bye' to hangup.\r\n")
            
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                
                text = data.decode('utf-8', errors='ignore')
                print(f"Received: {text}")
                
                if "bye" in text.lower():
                    print("Client requested disconnect.")
                    break
                
                if "\r" in text:
                    conn.sendall(b"\n")
                    
                # Echo back to the emulator
                conn.sendall(data)
                
    print("--- CONNECTION CLOSED ---")

except KeyboardInterrupt:
    print("\nServer stopped.")
except Exception as e:
    print(f"Error: {e}")