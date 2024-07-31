def compare_files(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        f1_lines = f1.readlines()
        f2_lines = f2.readlines()

    max_lines = max(len(f1_lines), len(f2_lines))
    for i in range(max_lines):
        line1 = f1_lines[i].strip() if i < len(f1_lines) else ""
        line2 = f2_lines[i].strip() if i < len(f2_lines) else ""
        if line1 != line2:
            return i + 1, line1, line2
    return -1, "", ""  # Files are identical

file1 = 'cFS_LOG.txt'
file2 = 'ground_LOG.txt'

line_num, line1, line2 = compare_files(file1, file2)

if line_num != -1:
    print(f"Files differ starting from line {line_num}.")
    print(f"File1: {line1}")
    print(f"File2: {line2}")
else:
    print("Files are identical.")