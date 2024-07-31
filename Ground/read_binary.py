def read_binary_file(file_name):
    try:
        with open(file_name, 'rb') as file:
            content = file.read()
            return content
    except FileNotFoundError:
        print(f"File {file_name} not found.")
        return None

def save_to_text_file(data, output_file):
    try:
        with open(output_file, 'w') as file:
            for i in range(0, len(data), 64):
                line = data[i:i+64]
                file.write(line.hex() + '\n')
    except IOError as e:
        print(f"An error occurred while writing to the file: {e}")

file_path = 'cFS_to_receive_LOG.bin'
output_file_path = 'cFS_to_receive_LOG.txt'

binary_data = read_binary_file(file_path)
if binary_data:
    save_to_text_file(binary_data, output_file_path)
    print(f"Binary data saved to {output_file_path}")