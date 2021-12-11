import math, collections

class StupidBackoffLanguageModel:

  def __init__(self, corpus):
    """Initialize your data structures in the constructor."""
    self.trigram_counts = collections.defaultdict(lambda: 0)
    self.bigram_counts = collections.defaultdict(lambda: 0)
    self.unigram_counts = collections.defaultdict(lambda: 0)
    self.word_count = 0

    self.train(corpus)

  def train(self, corpus):
      """ Takes a corpus and trains your language model. 
        Compute any counts or other corpus statistics in this function.
      """  
      for sentence in corpus.corpus:
          for i in xrange(0, len(sentence.data)-2):
              word1 = sentence.data[i].word
              word2 = sentence.data[i+1].word
              word3 = sentence.data[i+2].word

              self.trigram_counts[(word1, word2, word3)] += 1
              self.bigram_counts[(word1, word2)] += 1

              self.unigram_counts[word1] += 1
              self.word_count += 1

          word1 = sentence.data[-2].word
          word2 = sentence.data[-1].word
          self.unigram_counts[word1] += 1
          self.unigram_counts[word2] += 1
          self.bigram_counts[(word1, word2)] += 1
          self.word_count += 2

  def score(self, sentence):
    """ Takes a list of strings as argument and returns the log-probability of the 
        sentence using your language model. Use whatever data you computed in train() here.
    """
    score = 0.0
    
    """
    word2 = sentence[1]
    word1 = sentence[0]
    bi_count = self.bigram_counts[(word1, word2)]
    uni_count1 = self.unigram_counts[word1]
    uni_count2 = self.unigram_counts[word2]
    if bi_count > 0:
        score += math.log(0.4*bi_count)
        score -= math.log(uni_count1)
    elif uni_count2 > 0:
        score += math.log(0.4*uni_count2)
        score -= math.log(self.word_count)
        """

    vocab_size = len(self.unigram_counts)
    for i in xrange(2, len(sentence)):
        word1 = sentence[i-2]
        word2 = sentence[i-1]
        word3 = sentence[i]
        tri_count = self.trigram_counts[(word1, word2, word3)]

        bi_count1 = self.bigram_counts[(word1, word2)]
        bi_count2 = self.bigram_counts[(word2, word3)]
        uni_count1 = self.unigram_counts[word2]
        uni_count2 = self.unigram_counts[word3]

        """
        if tri_count > 0:
            score += math.log(tri_count)
            score -= math.log(bi_count1)
        """
        if bi_count2 > 0:
            score += math.log(bi_count2) + math.log(0.4)
            score -= math.log(uni_count1)
        elif uni_count2 > 0:
            score += math.log(uni_count2) + math.log(0.4*0.4)
            score -= math.log(self.word_count)

    return score
