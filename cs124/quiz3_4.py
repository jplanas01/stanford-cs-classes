from collections import defaultdict
docs = [
        ("good good good great great great", "pos"),
        ("poor great great", "pos"),
        ("good poor poor poor", "neg"),
        ("good poor poor poor poor poor great great", "neg"),
        ("poor poor", "neg"),
        ]

megadoc = defaultdict(lambda: [])
vocab = set("")
word_counts = {}
doc_counts = defaultdict(lambda: 0)

for doc, clas in docs:
    doc_counts[clas] += 1
    # For Boolean, iterate over set(doc.split())
    # For Normal, iterate over doc.split()
    for word in set(doc.split()):
        megadoc[clas].append(word)
        vocab.add(word)

for key in iter(megadoc):
    word_counts[key] = len(megadoc[key])

total = 1
for word in "good acting poor plot".split():
    freq = megadoc["comedy"].count(word)
    p_word = (freq + 1) / (len(vocab) + word_counts["pos"])
    total *= p_word

print(total, total * doc_counts["pos"] / len(docs))

total = 1
for word in "good acting poor plot".split():
    freq = megadoc["neg"].count(word)
    p_word = (freq + 1) / (len(vocab) + word_counts["neg"])
    total *= p_word

print(total, total * doc_counts["neg"] / len(docs))
