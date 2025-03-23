import sys

# Check if correct number of arguments is provided
if len(sys.argv) != 4:
    print("Usage: python script.py <input_file> <output_file> <replacement>")
    sys.exit(1)

# Get filenames from command-line arguments
input_file = sys.argv[1]
output_file = sys.argv[2]
replacement = sys.argv[3]

def first_upper(s: str) -> str:
    return s[0].upper() + s[1:]

try:
    # Open the input file and read its contents
    with open(input_file, "r", encoding="utf-8") as file:
        content = file.read()

    # Replace
    modified_content = content.replace("Kai_", first_upper(replacement)) \
                              .replace("kai_", replacement.lower()) \
                              .replace("KAI_", replacement.upper())

    # Save the modified content to the output file
    with open(output_file, "w", encoding="utf-8") as file:
        file.write(modified_content)

    print(f"Replacement complete. Modified file saved as '{output_file}'.")
except FileNotFoundError:
    print(f"Error: The file '{input_file}' was not found.")
except Exception as e:
    print(f"An error occurred: {e}")
