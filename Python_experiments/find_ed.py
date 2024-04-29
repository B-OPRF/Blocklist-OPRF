def read_file_to_string(file_path):
    try:
        with open(file_path, 'r') as file:
            file_content = file.read()
        return file_content
    except FileNotFoundError:
        print("File not found.")
        return ""


file_path = '1.txt'
file = read_file_to_string(file_path)

def edit_distance_1(word):
  alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
  ret = set()
  for i in range(len(word)):
    ret.add(word[:i]+word[i+1:])
    for j in range(len(alphabet)):
      ret.add(word[:i]+alphabet[j]+word[i:])
      ret.add(word[:i]+alphabet[j]+word[i+1:])
  return ret

edit_1 = set()
edit_1.update(edit_distance_1(file))

# edit_2 = set(edit_1)
# for i in range(len(edit_1)):
#   edit_2.update(edit_distance_1(list(edit_1)[i]))

print(len(edit_1))

def write_set_to_file(set_data, file_path):
    try:
        with open(file_path, 'w') as file:
            for item in set_data:
                file.write(str(item) + '\n')
        print("Set has been written to the file successfully.")
    except IOError:
        print("Error writing to file.")

file_path = 'edit_1.txt'  # Name of the file to write
write_set_to_file(edit_1, file_path)