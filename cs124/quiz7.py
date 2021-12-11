import numpy as np
from scipy.spatial.distance import cosine
from math import log2

# ITEM ITEM FILTERING:
max_val = 10
mat = [
    [8, 9, None, None, 0, 5, 7],
    [1, None, 4, 8, 7, None, 2],
    [0, 9, None, 3, None, 8, 3],
    [None, 8, 5, None, 3, 4, None]]
org_mat = np.array(mat).transpose()
ut_matrix = np.array(mat).transpose()
means = []
for row in ut_matrix:
    count = 0
    accum = 0
    for col in row:
        if col is not None:
            accum += col
            count += 1
    means.append(accum / count)
for i in range(len(ut_matrix)):
    for j in range(len(ut_matrix[0])):
        if ut_matrix[i][j] is not None:
            ut_matrix[i][j] = ut_matrix[i][j] - means[i]
        else:
            ut_matrix[i][j] = 0

#print(ut_matrix)
similarities = []
# Stuff here changes depending on chosen user/movie
movie = 2
user = 2
for i in range(len(ut_matrix)):
    similarities.append(1 - cosine(ut_matrix[movie], ut_matrix[i]))
#print(similarities)

nom = 0
denom = 0
for i in range(len(org_mat)):
    if similarities[i] < 0 or i == movie or org_mat[i][user] is None:
        continue
    nom += similarities[i] * org_mat[i][user]
    denom += similarities[i]
print(nom / denom)



# PMI/PPMI
# words are rows, docs are columns
doc = 0
word = 0
org_mat = np.array([
        [34, 38, 23, 16],
        [27, 32, 17, 1],
        [35, 12, 3, 7],
        [0, 0, 0, 3]
        ])
total = 0
total_wc = []
for row in org_mat:
    total += sum(row)
    total_wc.append(sum(row))

doc_wc = []
for row in np.array(org_mat).transpose():
    doc_wc.append(sum(row))

pmi = log2((org_mat[word][doc] / total) / ((doc_wc[doc] / total) *
    (total_wc[word] / total)))

if pmi > 0:
    ppmi = pmi
else:
    ppmi = 0

# With Laplace smoothing
smoothed_mat = org_mat + 10
s_total = 0
s_total_wc = []
for row in smoothed_mat:
    s_total += sum(row)
    s_total_wc.append(sum(row))

s_doc_wc = []
for row in smoothed_mat.transpose():
    s_doc_wc.append(sum(row))

spmi = log2((smoothed_mat[word][doc] / s_total) / 
        ((s_doc_wc[doc] / s_total) * (s_total_wc[word] / s_total)))

print(pmi, ppmi, spmi)
