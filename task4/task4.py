import logging
import time
import cv2
import os
import queue
import threading

def sensor_worker(sensor, data_queue, stop_event):
    while not stop_event.is_set():
        data = sensor.get()
        if data is not None:
            if not data_queue.empty():
                try:
                    data_queue.get_nowait()
                except queue.Empty:
                    pass
            data_queue.put(data)

class SensorCam:
    def __init__(self, name, resolution):
        if(name):
            self.name = name
        else:
            logging.error("Error: Camera name not specified.")

        target_w, target_h = resolution

        self.cap = cv2.VideoCapture(name)
        
        if not self.cap.isOpened():
            logging.critical(f"Critical error: Camera {name} not found in the system.")
            return

        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, target_w)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, target_h)

        real_w = self.cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        real_h = self.cap.get(cv2.CAP_PROP_FRAME_HEIGHT)

        if real_w != target_w or real_h != target_h:
            logging.warning(f"Warning: Camera does not support {target_w}x{target_h}. "
                            f"Set to nearest available: {int(real_w)}x{int(real_h)}")

    def get(self):
        if not self.cap.isOpened():
            logging.error("Attempt to read from closed device") 
            return None
        
        ret, frame = self.cap.read()

        if not ret:
            logging.error("Failed to get frame. Check camera connection.")
            return None

        return frame
    
    def __del__(self):
        if hasattr(self, 'cap') and self.cap.isOpened():
            self.cap.release()
            logging.info(f"Camera resource {self.name} successfully released.")

class Sensor:
    def get(self):
        logging.error("Get method not defined")
        raise NotImplementedError("Get method must be implemented in subclass")
    
class SensorX(Sensor):
    def __init__(self, delay : float):
        self.delay = delay
        self._data = 0
        logging.info(f"SensorX initialized with delay {self.delay} seconds")
    def get(self):
        time.sleep(self.delay)
        self._data += 1
        return self._data 
    def __del__(self):
        logging.info(f"SensorX resource with delay {self.delay} successfully released.")
        
class WindowImage:
    def __init__(self, frequency):
        self.frequency = frequency
        self.last_update_time = 0
        logging.info(f"Image display window initialized with update frequency {self.frequency} frames per second")
    def show(self, image):
        cv2.imshow("Camera Feed", image)
        cv2.waitKey(self.frequency)
    def __del__(self):
        cv2.destroyAllWindows()
        logging.info("Image display window successfully closed.")

if __name__ == "__main__":
    if not os.path.exists('log'): 
        os.makedirs('log')
    logging.basicConfig(filename='log/app.log', level=logging.INFO, 
                        format='%(asctime)s - %(levelname)s - %(message)s')

    name_cam = 0
    resolution = (640, 480)
    fps = 30

    stop_event = threading.Event()
    
    cam = SensorCam(name_cam, resolution)
    s0 = SensorX(0.01) 
    s1 = SensorX(0.1)  
    s2 = SensorX(1.0)  
    
    all_sensors = [cam, s0, s1, s2]
    queues = [queue.Queue(maxsize=1) for _ in all_sensors]
     
    threads = []
    for s, q in zip(all_sensors, queues):
        t = threading.Thread(target=sensor_worker, args=(s, q, stop_event), daemon=True)
        t.start()
        threads.append(t)

    window = WindowImage(fps)
    last_vals = [None, 0, 0, 0]

    try:
        while True:
            for i, q in enumerate(queues):
                if not q.empty():
                    last_vals[i] = q.get()
            
            frame = last_vals[0]
            if frame is not None:
                display_frame = frame.copy()
                cv2.putText(display_frame, f"Sensor0: {last_vals[1]}", (10, 30), 1, 1.5, (0,255,0), 2)
                cv2.putText(display_frame, f"Sensor1: {last_vals[2]}", (10, 60), 1, 1.5, (0,255,0), 2)
                cv2.putText(display_frame, f"Sensor2: {last_vals[3]}", (10, 90), 1, 1.5, (0,255,0), 2)
                
                window.show(display_frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        stop_event.set()