import logging
import time
import cv2
import os
import queue
import threading
from ultralytics import YOLO

class Parall:
    def __init__(self, model_path = 'yolov8s-pose.pt', workers = 5):
        self.model = YOLO(model_path)
        self.inp_buffer = queue.Queue(maxsize=10)
        self.out_buffer = {}
        self.lock = threading.Lock()
        self.stop_event = threading.Event()
        self.threads = []
        self.num_workers = workers
        self.max_buffer_size = 30

    def add_frame(self, id_frame, frame):
        if not self.inp_buffer.full():
            self.inp_buffer.put((id_frame, frame))

    def worker(self):
        while not self.stop_event.is_set():
            if not self.inp_buffer.empty():
                id_frame, frame = self.inp_buffer.get()
                results = self.model(frame)
                with self.lock:
                    self.out_buffer[id_frame] = results
                    
                    if len(self.out_buffer) > self.max_buffer_size:
                        oldest_id = min(self.out_buffer.keys())
                        del self.out_buffer[oldest_id]   
    
    def start(self):
        for i in range(self.num_workers):
            t = threading.Thread(target=self.worker, daemon=True)
            t.start()
            self.threads.append(t)

    def get_any_latest(self):
        with self.lock:
            if not self.out_buffer:
                return None
        
            latest_id = max(self.out_buffer.keys())
            res = self.out_buffer[latest_id]
        
            self.out_buffer.clear()
            return res
    
    def stop(self):
        self.stop_event.set()
        for t in self.threads:
            t.join()
    
proc = Parall()
proc.start()

cam = cv2.VideoCapture(0)
frame_id = 0

last_res = None

while True:

    ret, frame = cam.read()

    proc.add_frame(frame_id, frame)

    new_res = proc.get_any_latest()
    if new_res is not None:
        last_res = new_res

    if last_res is not None:
        annotated_frame = last_res[0].plot() 
        cv2.imshow("Camera Feed", annotated_frame)
    else:
        cv2.imshow("Camera Feed", frame)

    frame_id += 1

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

proc.stop()
cam.release()
cv2.destroyAllWindows()
        

    


    



