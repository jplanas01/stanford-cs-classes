#!/usr/bin/env python
# -*- coding: utf-8 -*-

# PA6, CS124, Stanford, Winter 2016
# v.1.0.2
# Original Python code by Ignacio Cases (@cases)
# Ported to Java by Raghav Gupta (@rgupta93) and Jennifer Lu (@jenylu)
######################################################################
import csv
import math
import os

import numpy as np
from deps.PorterStemmer import PorterStemmer

from movielens import ratings
from random import randint, choice
from collections import defaultdict
from math import log
import re
import gzip
from operator import itemgetter


# Taken from sentiment analysis assignment
class NaiveBayes:
  class Example:
    """Represents a document with a label. klass is 'pos' or 'neg' by convention.
       words is a list of strings.
    """
    def __init__(self):
      self.klass = ''
      self.words = []


  def __init__(self):
    """NaiveBayes initialization"""
    self.FILTER_STOP_WORDS = False
    self.BOOLEAN_NB = True
    self.BEST_MODEL = False

    self.megadoc = defaultdict(lambda: [])
    self.megadoc_count = defaultdict(lambda: defaultdict(lambda :0))
    self.vocab = set()
    self.word_counts = {}
    self.doc_counts = defaultdict(lambda: 0)
    self.total_docs = 0

  def filter_punctuation(self, words):
      filtered = []
      for word in words:
          temp = string.translate(word, None, "!?,.;:()\"'-")
          if temp:
              filtered.append(temp)
      return filtered
    
  def classify(self, words):
    """ 
      'words' is a list of words to classify. Return 'pos' or 'neg' classification.
    """

    alpha = 5.0

    if self.FILTER_STOP_WORDS:
        words =  self.filterStopWords(words)

    if self.BOOLEAN_NB:
        words = set(words)

    if self.BEST_MODEL:
        words = set(words)
        pass

    vocab_size = len(self.vocab)

    best_prob = float("-inf")
    best_class = ""
    for clas in self.megadoc:
        class_prob = 0
        for word in words:
            freq = self.megadoc_count[clas][word]
            class_prob += log((freq + alpha) / 
                              (vocab_size * alpha + self.word_counts[clas]))
        class_prob += log(float(self.doc_counts[clas]) / self.total_docs)

        if class_prob > best_prob:
            best_class = clas
            best_prob = class_prob

    return best_class

  def addExample(self, klass, words):
    """
     * Train your model on an example document with label klass ('pos' or 'neg') and
     * words, a list of strings.
     * You should store whatever data structures you use for your classifier 
     * in the NaiveBayes class.
     * Returns nothing
    """
    if self.FILTER_STOP_WORDS:
        words =  self.filterStopWords(words)

    if self.BOOLEAN_NB:
        words = set(words)

    if self.BEST_MODEL:
        words = set(words)

    self.doc_counts[klass] += 1
    power_words = ['transcends',  'jaw-dropping', 'amazing', 'awful',
    'horrible', 'loathe', 'miraculously', 'terrible', 'perfect', 'excellent']

    for word in words:
        self.megadoc[klass].append(word)
        if word in power_words:
            self.megadoc_count[klass][word] += 3
        else:
            self.megadoc_count[klass][word] += 1

        self.vocab.add(word)

    for key in self.megadoc:
        self.word_counts[key] = len(self.megadoc[key])

    self.total_docs += 1

  def segmentWords(self, s):
    """
     * Splits lines on whitespace for file reading
    """
    return s.split()
  
  def filterStopWords(self, words):
    """Filters stop words."""
    filtered = []
    for word in words:
      if not word in self.stopList and word.strip() != '':
        filtered.append(word)
    return filtered

class Chatbot:
    """Simple class to implement the chatbot for PA 6."""

    #############################################################################
    # `moviebot` is the default chatbot. Change it to your chatbot's name       #
    #############################################################################
    def __init__(self, is_turbo=False):
      self.name = 'Antonia'
      self.is_turbo = is_turbo
      self.stemmer = PorterStemmer()
      self.read_data()
      self.binarize()
      self.movie_queue = []

    def greeting(self):
      """chatbot greeting message"""

      greeting_message = 'Hello and welcome!'
      return greeting_message

    def goodbye(self):
      """chatbot goodbye message"""

      goodbye_message = 'Have an above average afternoon!'
      return goodbye_message
    
    def get_sentiment(self, mesg):
        stemmed = [self.stemmer.stem(word) for word in mesg.split()]

        if not any(x in self.sentiment for x in stemmed):
            print 'I\'m not sure how you feel about this movie.'
            print 'Enter "yes" if you liked it, otherwise enter "no".'
            x = ''
            while x != 'yes' or 'no':
                x = raw_input().strip()
                if x == 'yes':
                    return 'pos'
                else:
                    return 'neg'

        sent = self.classifier.classify(stemmed)
        for negative in ['not', 'never', 'n\'t']:
            if negative in mesg:
                if sent == 'pos':
                    sent = 'neg'
                else:
                    sent = 'pos'

        return sent

    def process_starter(self, input):
        flip_resp = [
                ('OK, I flipped the rating for the last movie '
                    'you told me about. Sorry about that!'),
                'My bad, I\'ve updated your preference for that movie.',
                'I apologize for the inconvenience.'
                ]
        if input.lower() == 'flip':
            if len(self.movie_queue) < 1:
                response = 'You haven\'t told me about any movies yet!'
            else:
                pass
                response = choice(flip_resp)
                movie = self.movie_queue.pop()
                if movie[1] == 1:
                    self.movie_queue.append((movie[0], -1))
                else:
                    self.movie_queue.append((movie[0], 1))
            return response

        rec_resp = ['Based on what you told me, I recommend {}.',
                'I think you\'ll enjoy watching {}.',
                'Try one of these movies next: {}.']
        if input.lower() == 'recommend':
            return choice(rec_resp).format(self.recommend())

        movie_re = re.compile(r'"([\w ]+) \(([0-9]{4})\)"')
        sent_re = re.compile(r'".*?"')
        response = 'What?'
        pos_resp = ['OK, so you liked {}.', 
                'Alright, I\'m glad you enjoyed {}',
                'You enjoyed {}, got it.',
                '{}? I like that movie too!']
        neg_resp = ['Oh, so you didn\'t like {}',
                'I\'m sorry you didn\'t enjoy {}',
                'You disliked {}? But it\'s one of my favorites!',
                'I guess SOME people have to hate {}...']


        movs = movie_re.findall(input)

        if not movs:
            return self.non_movie_input(input)
        elif len(movs) > 1:
            return ('I apologize, I can only handle one movie at a '
                    'time right now...')
        else:
            movs = movs[0]
            title = '{} ({})'.format(*movs)
            if title not in self.movies:
                title = self.spellcheck(title)
                if not title:
                    return choice(['Let\'s try that again.',
                            'I\'m not sure what movie you are referring to.',
                            'Hm, I did not catch that, sorry.'])

            sent = self.get_sentiment(sent_re.sub('', input))
            if sent == 'pos':
                sent = 1.5
                response = choice(pos_resp).format(movs[0])
            else:
                sent = -1.5
                response = choice(neg_resp).format(movs[0])

            for tit, se in self.movie_queue:
                if tit == title:
                    return choice(['You already told me about that movie!',
                        'Wait, you\'ve already let me know about that one.'])
            self.movie_queue.append((title, sent))


        return response

    def non_movie_input(self, input):
        input = input.lower()

        if 'can you' in input:
            return 'I don\'t know, can I?'
        if 'can i ' in input:
            return 'I don\'t know, can you?'
        if 'would you' in input:
            return 'Nah, I\'d rather not right now.'

        if 'what is love' in input:
            return 'BABY DON\'T HURT ME NO MORE'
        if 'what is this' in input:
            return 'The same thing as that behind you, of course.'

        if 'what is' in input:
            return ('I have no idea what '
                    + input.replace('what is', '').replace('?', '') + ' is...')

        if 'who is' in input:
            return (input.replace('who is', '') + ' I used to ride bikes ' +
                    'him when I was younger!')

        return choice([('I\'m sorry, I didn\'t catch what you were '
            'talking about. Can we talk about movies again?'),
            'What?',
            'Come again?'
            'I think this isn\'t related to movies...'])

    def levenshtein(self, source, target):
        """Optimized Levenshtein distance calculation.
        Taken from https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#Python.
        """
        if len(source) < len(target):
            return self.levenshtein(target, source)

        if len(target) == 0:
            return len(source)

        source = np.array(tuple(source))
        target = np.array(tuple(target))

        previous_row = np.arange(target.size + 1)
        for s in source:
            current_row = previous_row + 1
            current_row[1:] = np.minimum(
                    current_row[1:],
                    np.add(previous_row[:-1], target != s))

            current_row[1:] = np.minimum(
                    current_row[1:],
                    current_row[0:-1] + 1)

            previous_row = current_row

        return previous_row[-1]

    def spellcheck(self, title):
        best = ''
        best_dist = float('inf')
        for movie in self.movies:
            edit_dist = self.levenshtein(movie, title)
            if edit_dist < best_dist:
                best_dist = edit_dist
                best = movie
        print 'Did you mean to input this title:', best
        res = raw_input('Enter "yes" to confirm, anything else to cancel: ')
        if res == 'yes':
            return best
        return None


    def process(self, input):
      """Takes the input string from the REPL and call delegated functions
      that
        1) extract the relevant information and
        2) transform the information into a response to the user
      """
      # Funny how 'starter' turns into 'the real thing'
      response = self.process_starter(input)

      return response


    #############################################################################
    # 3. Movie Recommendation helper functions                                  #
    #############################################################################

    def read_data(self):
      """Reads the ratings matrix from file
      Source for positive and negative sentiment files:
      Minqing Hu and Bing Liu. "Mining and Summarizing Customer Reviews." 
          Proceedings of the ACM SIGKDD International Conference on Knowledge 
          Discovery and Data Mining (KDD-2004), Aug 22-25, 2004, Seattle, 
          Washington, USA"""
      # This matrix has the following shape: num_movies x num_users
      # The values stored in each row i and column j is the rating for
      # movie i by user j
      self.classifier = NaiveBayes()
      self.titles, self.ratings = ratings()
      reader = csv.reader(open('data/sentiment.txt', 'rb'))
      self.sentiment = {}

      for row in reader:
          word = self.stemmer.stem(row[0])
          self.sentiment[word] = row[1]
          self.classifier.addExample(row[1], [word])
      #with gzip.open('deps/positive-words.txt.gz', 'rb') as f:
      with open('deps/positive-words.txt', 'r') as f:
          for line in f:
              word = self.stemmer.stem(line.strip())
              self.sentiment[word] = 'pos'
              self.classifier.addExample('pos', [word])

      #with gzip.open('deps/negative-words.txt.gz', 'rb') as f:
      with open('deps/positive-words.txt', 'r') as f:
          for line in f:
              word = self.stemmer.stem(line.strip())
              self.sentiment[word] = 'neg'
              self.classifier.addExample('neg', [word])

      article_re = re.compile(r'([\w+ ]+), (The) (\([0-9]{4}\))|([\w+ ]+), (An) (\([0-9]{4}\))|([\w ]+), (A) (\([0-9]{4}\))')
      self.movies, self.genres = zip(*self.titles)
      self.movies = list(self.movies)
      for i, mov in enumerate(self.movies):
        matc = article_re.match(mov)
        if matc:
            text = tuple(x for x in matc.groups() if x)
            self.movies[i] = '{1} {0} {2}'.format(*text)

    def binarize(self):
      """Modifies the ratings matrix to make all of the ratings binary"""
      #self.ratings[np.where((self.ratings > 0) & (self.ratings < cutoff))] = -1
      #self.ratings[np.where(self.ratings >= cutoff)] = 1
      for row in range(len(self.ratings)):
          if all(self.ratings[row] == 0):
              continue
          mean = np.mean(self.ratings[row][np.where(self.ratings[row] > 0)])
          self.ratings[row][np.where(self.ratings[row] > 0)] -= mean


    def distance(self, u, v):
      """Calculates a given distance function between vectors u and v"""
      # Note: you can also think of this as computing a similarity measure
      # Use dot product (no normalization) as recommended in lecture

      # Works better without normalization...
      return np.dot(u, v)

      #return sum([i * j for (i, j) in zip(u, v)])

    def recommend(self):
      """Generates a list of movies based on the input vector u using
      collaborative filtering"""
      # TODO: Implement a recommendation function that takes a user vector u
      # and outputs a list of movies recommended by the chatbot
      indices = []
      user_mat = np.zeros((len(self.movies), 1))
      for movie in self.movie_queue:
          pos = self.movies.index(movie[0])
          indices.append(pos)
          user_mat[pos] = movie[1]
      rec_ratings = np.concatenate((self.ratings,  user_mat), axis=1)
      
      ratings = []

      for mov_ind in range(len(user_mat)):
          # Don't recommend rated movies
          if mov_ind in indices:
              # Make sure to never recommend seen movies.
              ratings.append((mov_ind, -10000))
              continue
          tot = 0
          for seen in indices:
              rated = user_mat[seen][0]
              sim = self.distance(self.ratings[seen], self.ratings[mov_ind])
              tot += rated * sim
          ratings.append((mov_ind, tot))
      ratings.sort(key=itemgetter(1), reverse=True)

      top_ind = [x for x, _ in ratings[0:3]]
      top_movies = [self.movies[x] for x in top_ind]

      return '{}, {}, and {}'.format(*top_movies)

    def debug(self, input):
      """Returns debug information as a string for the input string from the REPL"""
      # Pass the debug information that you may think is important for your
      # evaluators
      debug_info = 'debug info'
      return debug_info


    #############################################################################
    # 5. Write a description for your chatbot here!                             #
    #############################################################################
    def intro(self):
      return """Hello! I\'m good at recommending movies.
            Give me at least 4 movies and I\'ll give you other movies to 
            watch. I will also let you know if I need more information, like
            information on more movies or how you felt about a movie.
            To change the sentiment on the latest movie you told me about, type
            in "flip" and to get recommendations type in "recommend"
            (without quotes).
            I can also guess the next best title if you misspell something, I can
            (sort of) answer questions in who-is, can-i, what-is form.
            (Data set is non-binary, sentiment extraction uses Bayesian classifier)
      """


    #############################################################################
    # Auxiliary methods for the chatbot.                                        #
    #                                                                           #
    # DO NOT CHANGE THE CODE BELOW!                                             #
    #                                                                           #
    #############################################################################

    def bot_name(self):
      return self.name


if __name__ == '__main__':
    Chatbot()
