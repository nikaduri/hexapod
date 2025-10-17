#!/usr/bin/python3
import socket
import threading
import time
import logging
from queue import Queue

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class HexapodServer:
    def __init__(self, host='0.0.0.0', port=8080):
        self.host = host
        self.port = port
        self.server_socket = None
        self.running = False
        self.clients = []
        self.command_queue = Queue()
        self.current_command = None
        self.last_command_time = 0

    def start(self):
        """Start the server"""
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Allow port reuse
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # Set keep alive options
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(1)
        self.running = True
        
        # Start command processor thread
        command_thread = threading.Thread(target=self.process_command_queue)
        command_thread.daemon = True
        command_thread.start()
        
        logger.info(f"Server started on {self.host}:{self.port}")
        
        while self.running:
            try:
                client_socket, address = self.server_socket.accept()
                logger.info(f"New connection from {address}")
                
                # Configure client socket
                client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
                client_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                
                # Start a new thread to handle this client
                client_thread = threading.Thread(
                    target=self.handle_client,
                    args=(client_socket, address)
                )
                client_thread.daemon = True
                client_thread.start()
                
                self.clients.append((client_socket, address))
            except Exception as e:
                logger.error(f"Error accepting connection: {e}")
                if not self.running:
                    break

    def handle_client(self, client_socket, address):
        """Handle individual client connections"""
        try:
            while self.running:
                try:
                    # Set a timeout for receiving data
                    client_socket.settimeout(5.0)
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    
                    # Check if it's a ping (single byte with value 0)
                    if len(data) == 1 and data[0] == 0:
                        # Respond to ping
                        try:
                            client_socket.send(b'\x00')
                        except Exception as e:
                            logger.error(f"Error sending ping response: {e}")
                            break
                        continue
                    
                    # Decode the command
                    command = data.decode('utf-8')
                    logger.info(f"Received command from {address}: {command}")
                    
                    # Add command to queue
                    self.command_queue.put(command)
                    
                except socket.timeout:
                    # Check if client is still connected
                    try:
                        client_socket.send(b'\x00')
                    except:
                        break
                except Exception as e:
                    logger.error(f"Error receiving data from {address}: {e}")
                    break
                    
        except Exception as e:
            logger.error(f"Error handling client {address}: {e}")
        finally:
            logger.info(f"Client {address} disconnected")
            try:
                client_socket.close()
                self.clients.remove((client_socket, address))
                # Stop current command when client disconnects
                self.command_queue.put("STOP")
            except:
                pass

    def process_command_queue(self):
        """Process commands from the queue"""
        while self.running:
            try:
                # Get command from queue, wait up to 100ms
                try:
                    command = self.command_queue.get(timeout=0.1)
                except:
                    continue
                
                # Process the command
                self.execute_command(command)
                
                # Small delay to prevent CPU overload
                time.sleep(0.01)
            except Exception as e:
                logger.error(f"Error processing command queue: {e}")

    def execute_command(self, command):
        """Execute a single command"""
        try:
            # Update command state
            self.current_command = command
            self.last_command_time = time.time()
            
            if command == "FORWARD":
                self.move_forward()
            elif command == "BACKWARD":
                self.move_backward()
            elif command == "LEFT":
                self.turn_left()
            elif command == "RIGHT":
                self.turn_right()
            elif command == "STOP":
                self.stop()
            elif command == "STAND":
                self.stand()
            elif command == "LAY_DOWN":
                self.lay_down()
            elif command == "DANCE":
                self.dance()
            # Gait mode commands
            elif command == "TRIPOD_GAIT":
                self.set_tripod_gait()
            elif command == "WAVE_GAIT":
                self.set_wave_gait()
            elif command == "RIPPLE_GAIT":
                self.set_ripple_gait()
            elif command == "STAIRCASE_MODE":
                self.set_staircase_mode()
            else:
                logger.warning(f"Unknown command: {command}")
        except Exception as e:
            logger.error(f"Error executing command {command}: {e}")
            # Don't stop on movement errors, just log them
            if command not in ["FORWARD", "BACKWARD", "LEFT", "RIGHT"]:
                self.stop()

    # Movement functions - Implement these according to your hexapod's control system
    def move_forward(self):
        logger.info("Moving forward")
        # TODO: Implement forward movement

    def move_backward(self):
        logger.info("Moving backward")
        # TODO: Implement backward movement

    def turn_left(self):
        logger.info("Turning left")
        # TODO: Implement left turn

    def turn_right(self):
        logger.info("Turning right")
        # TODO: Implement right turn

    def stop(self):
        logger.info("Stopping")
        # TODO: Implement stop

    def stand(self):
        logger.info("Standing")
        # TODO: Implement stand

    def lay_down(self):
        logger.info("Laying down")
        # TODO: Implement lay down

    def dance(self):
        logger.info("Dancing")
        # TODO: Implement dance

    # Gait mode functions
    def set_tripod_gait(self):
        logger.info("Setting Tripod Gait")
        # TODO: Implement tripod gait mode
        # Tripod gait: legs 1,3,5 and 2,4,6 move in alternating phases

    def set_wave_gait(self):
        logger.info("Setting Wave Gait")
        # TODO: Implement wave gait mode
        # Wave gait: legs move in sequence 1,2,3,4,5,6

    def set_ripple_gait(self):
        logger.info("Setting Ripple Gait")
        # TODO: Implement ripple gait mode
        # Ripple gait: legs move in pairs (1,4), (2,5), (3,6)

    def set_staircase_mode(self):
        logger.info("Setting Staircase Mode")
        # TODO: Implement staircase mode
        # Staircase mode: specialized movement for climbing stairs

    def shutdown(self):
        """Shutdown the server"""
        self.running = False
        # Stop any current movement
        self.stop()
        # Clear command queue
        while not self.command_queue.empty():
            self.command_queue.get()
        # Close all client connections
        for client_socket, _ in self.clients:
            try:
                client_socket.close()
            except:
                pass
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass
        logger.info("Server shutdown complete")

if __name__ == "__main__":
    server = HexapodServer()
    try:
        server.start()
    except KeyboardInterrupt:
        logger.info("Shutting down server...")
        server.shutdown() 