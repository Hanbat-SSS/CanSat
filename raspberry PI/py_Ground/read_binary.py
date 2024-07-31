def read_binary_file(file_name):
    try:
        with open(file_name, 'rb') as file:
            content = file.read()
            return content
    except FileNotFoundError:
        print(f"file {file_name} not found")
        return None
    
def print_hex(data):
    hex_data = data.hex()
    for i in range(0, len(hex_data),64):
        print(hex_data[i:i+64])
                   
def save_to_text_file(data, output_file):
    try:
        with open(output_file, 'w') as file:
            for i in range(0, len(data),64):
                line = data[i:i+64]
                file.write(line.hex() + '\n')
    except IOEror as e:
        print("An error occurred while writing to the file: {e}")
                   
file_path = 'cFS_to_LOG.bin'
output_file_path = 'cFS_to_LOG.txt'

binary_data = read_binary_file(file_path)
if binary_data:
    save_to_text_file(binary_data, output_file_path)
    print(f"Binary data saved to {output_file_path}")