import cv2
import numpy as np

def maze_to_matrix(image_path, target_size):
    # Load the maze image
    maze_image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)

    # Threshold the image to create a binary image
    _, thresholded = cv2.threshold(maze_image, 128, 255, cv2.THRESH_BINARY)

    # Invert the binary image so that paths are represented by 0s and walls by 1s
    inverted = cv2.bitwise_not(thresholded)

    # Resize the inverted image to the target size
    resized = cv2.resize(inverted, target_size, interpolation=cv2.INTER_AREA)

    # Normalize the resized image values between 0 and 1
    resized_norm = resized / 255.0

    # Apply a threshold to convert non-zero values to 1
    maze_matrix = np.where(resized_norm != 0, 1, 0)

    return maze_matrix

# Path to the maze image file
image_path = "/Users/zhangboyan/Year2-Project/maze/WechatIMG6761.png"

# Target size for resizing (e.g., (width, height))
target_size = (1000, 1000)

# Convert the maze image to a resized matrix
maze_matrix = maze_to_matrix(image_path, target_size)

# Save the matrix to a text file
np.savetxt("maze_matrix.txt", maze_matrix, fmt="%d")

print("Matrix saved to maze_matrix.txt")
