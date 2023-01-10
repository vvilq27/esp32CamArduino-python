import cv2
import os

# from ..imgUtility import rotateImage

images = [img for img in os.listdir() if img.endswith(".png")]

video_name = 'video2.avi'

frame = cv2.imread(images[0])
height, width, layers = frame.shape

video = cv2.VideoWriter(video_name, 0, 10, (width,height))

for image in images:

    video.write(cv2.imread( image))

cv2.destroyAllWindows()
video.release()