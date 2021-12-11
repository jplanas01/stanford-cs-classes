import math, collections

class LaplaceBigramLanguageModel:

  def __init__(self, corpus):
    """Initialize your data structures in the constructor."""
    self.bigram_counts = collections.defaultdict(lambda: 0)
    self.unigram_counts = collections.defaultdict(lambda: 0)

    self.train(corpus)

  def train(self, corpus):
    """ Takes a corpus and trains your language model. 
        Compute any counts or other corpus statistics in this function.
    """  
    for sentence in corpus.corpus:
        for i in xrange(0, len(sentence.data)-1):
            word1 = sentence.data[i].word
            word2 = sentence.data[i+1].word
            self.bigram_counts[(word1, word2)] += 1

            self.unigram_counts[word1] += 1

        self.unigram_counts[sentence.data[-1].word] += 1

             
  def score(self, sentence):
    """ Takes a list of strings as argument and returns the log-probability of the 
        sentence using your language model. Use whatever data you computed in train() here.
    """
    score = 0.0
    vocab_size = len(self.unigram_counts)
    for i in xrange(1, len(sentence)):
        word1 = sentence[i-1]
        word2 = sentence[i]
        bi_count = self.bigram_counts[(word1, word2)]
        uni_count = self.unigram_counts[word1]

        score += math.log(bi_count + 1)
        score -= math.log(uni_count + vocab_size)
    return score

