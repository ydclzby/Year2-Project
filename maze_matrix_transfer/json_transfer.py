def process_matrix_file(input_file_path, output_file_path):
    with open(input_file_path, 'r') as input_file:
        lines = input_file.readlines()

    matrix = []
    for line in lines:
        # Split the line by spaces and convert each value to an integer
        numbers = [int(num) for num in line.strip().split()]
        matrix.append(numbers)

    with open(output_file_path, 'w') as output_file:
        for i, row in enumerate(matrix):
            # Convert each row to a string of comma-separated values
            row_str = ', '.join(str(num) for num in row)
            if i < len(matrix) - 1:
                output_file.write(f'[{row_str}],\n')  # Add a comma after the closing square bracket
            else:
                output_file.write(f'[{row_str}]\n')  # Last line without a comma

    print(f"Converted matrix saved to '{output_file_path}'.")


# Example usage
input_file_path = '/Users/zhangboyan/Year2-Project/maze_matrix.txt'
output_file_path = '/Users/zhangboyan/Year2-Project/output.txt'
process_matrix_file(input_file_path, output_file_path)
