from ultralytics import YOLO

model = YOLO('yolov8s-pose.pt')
print(f"Model type: {model.model.task}")