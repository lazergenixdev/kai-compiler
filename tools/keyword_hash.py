keywords = [
    "break",
    "case",
    "cast",
    "continue",
    "defer",
    "else",
    "enum",
    "for",
    "if",
    "loop",
    "ret",
    "struct",
    "union",
    "using",
    "while",
]

N = 24;
missing = [i for i in range(24)];

def ordbin(c): return bin(ord(c));

#for kw in keywords:
#    print(f"{ordbin(kw[-2])[2:]} {ordbin(kw[-1])[2:]} {bin(len(kw))[2:].rjust(4, "0")}")
    
def get_bit(s, n):
	if n < 4:
		return 1&(len(s) >> n);
	if n < 10:
		return 1&(ord(s[-1]) >> (n-4));
	if n < 16:
		return 1&(ord(s[-2]) >> (n-10));
	return 1&(ord(s[0]) >> (n-16));

def ii(n):
	if n < 4:
		return (0,n);
	if n < 10:
		return (1,n-4);
	if n < 16:
		return (2,n-10);
	return (3,n-16);

def hsh(s, i, j, k, w, x):
	return (get_bit(s,x) << 4) | (get_bit(s,w) << 3) | (get_bit(s,i) << 2) | (get_bit(s,j) << 1) | get_bit(s,k);

def good(i,j,k,w,x):
	a = set(hsh(kw,i,j,k,w,x) for kw in keywords);
	return len(a) == len(keywords);

def hash(s):
	h = 0;
	for c in s:
	#	h = ((h << 4)) + ord(c); # 33
		h = ((h << 4) + h) + ord(c) + ord('a'); # 26
	return h;

def count(n):
	return len(set(hash(k)%n for k in keywords))

#for i in range(len(keywords),33):
#	if count(i) == len(keywords):
#		print(i)

m = [8] * 32;

for (i,kw) in enumerate(keywords):
	h = hash(kw)%27
	print(i,kw,h)
	m[h] = i

for (i,x) in enumerate(m):
	print(f"{x}, // {keywords[x]}")

exit()

for i in range(22):
	for j in range(i+1,22):
		for k in range(j+1,22):
			for w in range(k+1,22):
				for x in range(k+1,22):
					if good(i,j,k,w,x):
						print("WOW")
						print(i,j,k,w,x)
						print(ii(i),ii(j),ii(k),ii(w),ii(x))
						show()
						exit()

print("NONE");

"""

def hash(s, n):
	return (ord(s[-2]) - ord(s[-1]) * len(s)) % n;

def test(n):
	map = {hash(kw, n): [] for kw in keywords}

	for kw in keywords:
		map[hash(kw, n)].append(kw);

	for k,v in map.items():
		print(k, v);

	return len(map) == len(keywords)

for i in range(1, 100):
	if test(i):
		print("\x1b[92mFOUND", i);
		for k,v in {hash(kw, i): keywords.index(kw) for kw in keywords}.items():
			print(f"[{k}] = {v},")
		#	missing.remove(k);
		#for i in range(len(missing)):
		#	print(f"[{missing[i]}] = {7},")
		break;
"""