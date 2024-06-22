from sys import argv as args

TOTAL_WIDTH = 80

def put(s):
	print(s, end="")

if __name__ == "__main__":
	name = " ".join(args[1:])
	without = TOTAL_WIDTH - len(name) - 2
	left = without // 2
	right = without - left
	put("// ")
	for i in range(left - 3 - 1):
		put("=")
	put("> ")
	put(name)
	put(" <")
	for i in range(right - 2):
		put("=")
