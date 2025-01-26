# ----------------------------------------------------------------------------------
# Dependencies
import time

import cv2 as cv
import numpy as np

from ultralytics import YOLO

import pyautogui  # Key presses
import mss  # sreen capturing
from playsound import playsound # Import playsound for sound playback
import pygame

# ----------------------------------------------------------------------------------
# Run settings

# run_mode = "camera_mode"
run_mode = "screen_mode"
#video_path = "../sample_media/sample-videos-master/sample-videos-master/person-bicycle-car-detection.mp4"
#video_path = "../sample_media/sample-videos-master/sample-videos-master/car-detection.mp4"
video_path = "../sample_media/coverr/coverr-highway-signs-to-california-destinations-1080p.mp4"


#yolo_model = "8s"
yolo_model = "11n"
#yolo_model = "11n-s"

# Output video file name and settings
output_video_path = "output_with_detection_1.mp4"
fourcc = cv.VideoWriter_fourcc(*'mp4v')  # Codec for saving video in MP4 format



# ----------------------------------------------------------------------------------
# Model

if yolo_model == "8s":
    model = YOLO("yolov8s.pt")
elif yolo_model == "11n":
    model = YOLO("yolo11n.pt")
elif yolo_model == "11n-s":
    model = YOLO("yolo11n-seg.pt")

# ----------------------------------------------------------------------------------
# Input function for different modes
def get_input_frame(run_mode, cap=None, monitor=None):
    if run_mode == "camera_mode":
        return cv.VideoCapture(0)  # Capture from camera
    elif run_mode == "video_mode":
        return cv.VideoCapture(video_path)  # Capture from video
    elif run_mode == "screen_mode":
        with mss.mss() as sct:
            return np.array(sct.grab(monitor))  # Capture screen
    return None

# ----------------------------------------------------------------------------------
# Shared frame processing logic
def process_frame(frame, results, fps):
    global car_count, display_emergency_message, previous_detections

    # Crop the frame
    cropped_frame = frame[top_left_y:bottom_left_y, top_left_x:top_right_x]
    
    # Downscale the frame for performance reasons
    new_width, new_height = process_resolution_width, process_resolution_height
    cropped_frame = cv.resize(cropped_frame, (new_width, new_height))

    # Ensure the frame is in BGR format (3 channels)
    if cropped_frame.shape[2] == 4:  # If the frame has 4 channels (BGRA), convert to BGR
        cropped_frame = cv.cvtColor(cropped_frame, cv.COLOR_BGRA2BGR)

    # Calculate scaling factors
    scale_x = new_width / (top_right_x - top_left_x)
    scale_y = new_height / (bottom_left_y- top_left_y)
    
    # Scale line coordinates to the processed resolution (for drawing purposes)
    scaled_top_left_x = int(0 * scale_x)
    scaled_top_left_y = int(0 * scale_y)
    scaled_top_right_x = int((top_right_x - top_left_x) * scale_x)
    scaled_top_right_y = int(0 * scale_y)
    
    scaled_bottom_left_x = int(0 * scale_x)
    scaled_bottom_left_y = int((bottom_left_y- top_left_y) * scale_y)
    scaled_bottom_right_x = int((top_right_x - top_left_x) * scale_x)
    scaled_bottom_right_y = int((bottom_left_y- top_left_y) * scale_y)


    # Run YOLO model on each frame
    results = model.predict(source=cropped_frame, show=False)

    # Get processed frame with YOLO results
    output_frame = results[0].plot()
    
    # Draw the virtual line segments (from line_start_x to line_end_x)
    cv.line(output_frame, (scaled_top_left_x, scaled_top_left_y), (scaled_bottom_left_x, scaled_bottom_left_y), (0, 255, 0), 2) # Left line
    cv.line(output_frame, (scaled_top_right_x, scaled_top_right_y), (scaled_bottom_right_x, scaled_bottom_right_y), (0, 255, 0), 2) # Right line

    # Iterate over detected objects (bounding boxes)
    for result in results[0].boxes:
        class_id = int(result.cls[0])  # Get class ID (e.g., 'car' or 'truck')
        confidence = result.conf[0]  # Confidence score

        if class_id == 2 or class_id == 7:  # Car = 2, Truck = 7 in COCO dataset
            bbox = result.xyxy[0].cpu().numpy()  # Bounding box coordinates
            x_min, y_min, x_max, y_max = map(int, bbox)
            bbox_center_y = (y_min + y_max) // 2
            bbox_center_x = (x_min + x_max) // 2

            # Check if the center of the bounding box is within the ROI vertically
            if  scaled_top_left_x <= bbox_center_x <= scaled_top_right_x : 
              if scaled_top_left_y <= bbox_center_y <= scaled_bottom_left_y: #Check if the bounding box crosses the defined lines.
                if previous_detections.get(id(result)) == "above":
                      car_count += 1  # Increment car counter when the car crosses the line
                      display_emergency_message = True  # Trigger emergency message

                if bbox_center_y < scaled_top_left_y:
                    previous_detections[id(result)] = "above"
                elif bbox_center_y > scaled_bottom_left_y:
                    previous_detections[id(result)] = "below"

    # Display FPS
    cv.putText(output_frame, f"FPS: {fps:.2f}", (10, 30), cv.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    return output_frame


# ----------------------------------------------------------------------------------
# Main loop
def run_detection(run_mode):

    if run_mode == "video_mode":
        cap = get_input_frame(run_mode)
        if not cap.isOpened():
            print(f"Error: Unable to open video {video_path}")
            return
    elif run_mode == "screen_mode":
        with mss.mss() as sct:
            monitor = sct.monitors[1]
            cap = None
            global original_width, original_height
            original_width = monitor["width"]
            original_height = monitor["height"]


    while True:
        start_time = time.time()

        if run_mode == "camera_mode" or run_mode == "video_mode":
            ret, frame = cap.read()
            if not ret:
                print("No frame captured.")
                break
        else:  # Screen mode
            frame = get_input_frame(run_mode, monitor=monitor)

        fps = 1 / (time.time() - start_time)  # Calculate FPS
        processed_frame = process_frame(frame, None, fps)

        cv.imshow("YOLO Detection", processed_frame)

        if (cv.waitKey(1) & 0xFF in [ord('q'), ord('Q')]):
            print("Exiting...")
            break

    if cap:
        cap.release()
    cv.destroyAllWindows()

# ----------------------------------------------------------------------------------
# Start the detection process based on the selected mode

top_left_x=  0
top_left_y=  202
top_right_x=  955
top_right_y=  202

bottom_left_x=  0
bottom_left_y=  918
bottom_right_x=  955
bottom_right_y=  918

#process_resolution_width =  640*2
#process_resolution_height = 360*2
process_resolution_width =  920
process_resolution_height = 700

run_mode = "screen_mode"


run_detection(run_mode)