from math import log10, sqrt

# Prob 2
sent1 = 'we all live in that yellow submarine'
sent2 = 'the yellow mustard in that submarine sandwich was not yellow'
set1 = set(sent1.split())
set2 = set(sent2.split())

#print(len(set1 & set2) / len(set1 | set2))

# Prob 3
tf = (1 + log10(17))
idf = log10(806791/25235)
print(tf*idf)

# Prob 4
# Rows are terms, columns are docs
freq_counts = [
        [37, 30, 0, 3],
        [40, 10, 6, 0],
        [31, 0, 12, 17],
        [0, 5, 13, 0]
        ]

idf = []
for term_freq in freq_counts:
    idf.append(log10(len(term_freq) / sum(i > 0 for i in term_freq)))

doc2_tf = []
for i in range(len(freq_counts)):
    count = freq_counts[i][1]
    if count > 0:
        doc2_tf.append(log10(count) + 1)
    else:
        doc2_tf.append(0)

doc3_tf = []
for i in range(len(freq_counts)):
    count = freq_counts[i][2]
    if count > 0:
        doc3_tf.append(log10(count) + 1)
    else:
        doc3_tf.append(0)

num = 0
denom1 = 0
denom2 = 0
for i in range(len(doc2_tf)):
    qi = doc2_tf[i] * idf[i]
    di = doc3_tf[i] * idf[i]
    num += di * qi
    denom1 += qi ** 2
    denom2 += di ** 2
print(num / (sqrt(denom1 * denom2)))


# Prob 5
docs = list('NRRNNNRRNNNNNNR')

precisions = []
for i in range(1, len(docs)+1):
    subset = docs[:i]
    if subset[-1] != 'R':
        continue
    rel = subset.count('R')
    precisions.append(rel / len(subset))

print(sum(precisions) / len(precisions))
