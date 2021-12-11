import math, collections

class CustomLanguageModel:

  def __init__(self, corpus):
    """Initialize your data structures in the constructor."""
    self.bigram_counts = collections.defaultdict(lambda: 0)
    self.unigram_counts = collections.defaultdict(lambda: 0)
    self.continues = collections.defaultdict(lambda: 0)
    self.prefixes = collections.defaultdict(lambda: 0)

    self.word_count = 0

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
            self.word_count += 1

        self.unigram_counts[sentence.data[-1].word] += 1
        self.word_count += 1

    for word in iter(self.unigram_counts):
      total1 = 0
      total2 = 0
      for prefix, cont in iter(self.bigram_counts):
          if cont == word:
              total1 += 1
          if prefix == word:
              total2 += 1
      self.continues[word] = total1
      self.prefixes[word] = total2


  def _calc_cont(self, word):
      return float(self.continues[word])/len(self.bigram_counts)

  
  def _calc_lambda(self, word):
      return 0.75/self.unigram_counts[word]*self.prefixes[word]

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
        uni_count1 = self.unigram_counts[word1]
        uni_count2 = self.unigram_counts[word2]

        #import pdb
        #pdb.set_trace()
        
        if uni_count1 > 0:
            temp = max((bi_count - 0.75), 0.0) / uni_count1
            temp2 = self._calc_cont(word2) * self._calc_lambda(word1)

            if temp + temp2 != 0:
                score += math.log(temp + temp2)
            else:
                score += math.log(uni_count1 + 1) + math.log(0.75 / vocab_size)
                score -= math.log(self.word_count + vocab_size)

        else:
            score += math.log(uni_count1 + 1) + math.log(0.75 / vocab_size)
            score -= math.log(self.word_count + vocab_size)


    return score

