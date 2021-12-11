#!/usr/bin/env python
# CS124 Homework 5 Jeopardy
# Original written in Java by Sam Bowman (sbowman@stanford.edu)
# Ported to Python by Milind Ganjoo (mganjoo@stanford.edu)

import itertools as it
import NaiveBayes
import re
from collections import defaultdict

class ClueParser:
    def __init__(self):
        # TODO: if your implementation requires a trained classifier, you should declare it here.
        # Remember to import the class at the top of the file (from NaiveBayes import NaiveBayes)
        # e.g. self.classifier = NaiveBayes()
        self.classifier = NaiveBayes.NaiveBayes()

    def parseClues(self, clues):
        """Parse each clue and return a list of parses, one for each clue."""
        parses = []
        for clue in clues:
            # TODO: modify this to actually parse each clue and represent in relational form.
            feat = self.get_features(clue)
            cat = self.classifier.classify(feat)
            entity = self.get_entity(clue, cat)
            
            parses.append('{}:{}'.format(cat, entity))
        return parses

    def train(self, clues, parsed_clues):
        """Trains the model on clues paired with gold standard parses."""
        labels = []
        features = []
        for i in xrange(len(clues)):
            #print(clues[i], parsed_clues[i])
            label = parsed_clues[i].split(':')[0]
            feature = self.get_features(clues[i])
            labels.append(label)
            features.append(feature)
        self.classifier.addExamples(features, labels)

    def get_features(self, clue):
        features = []
        entities = defaultdict(lambda: 0)
        matches = re.findall('</([A-Z]+)>', clue)
        for match in matches:
            entities[match] += 1

        for key in entities:
            feat = 'HAS_{}_{}'.format(entities[key], key)
            features.append(feat)

        for keyword in ['husband', 'wife', 'led', 'based', 'born', 'birth',
                'university', 'degree', 'headquarter', 'college', 'passed',
                'school', 'alma mater', 'married', 'mayor', 'this year',
                'this city', 'run by', 'the year', 'undergrad', 'headed',
                'located', 'president', 'head of', 'parent organization',
                'died', 'town', 'he\'s', 'she\'s', 'she is', 'he is'
                'he married', 'she married', 'his wife', 'her husband',
                'offshoot', 'wing', ' man ', 'here', 'home to',
                'woman', 'led by', 'head of', 'where', 'married him',
                'married her', 'organization', 'parent']:
            if keyword in clue.lower():
                if keyword == 'he is' and 'she is' in features:
                    continue
                features.append(keyword)
        return features
    
    def get_entity(self, clue, category):
        # Match just a single person
        result = ''
        if category in ['wife_of', 'husband_of', 'year_of_birth',
                'year_of_death', 'born_in', 'college_of']:
            regex = '<PERSON>(.*?)</PERSON>'
            matches = re.findall(regex, clue)
            if len(matches) > 0:
                result = matches[0]
        if category in ['univ_president_of', 'parent_org_of', 'headquarters_loc']:
            regex = '<ORGANIZATION>(.*?)</ORGANIZATION>'
            matches = re.findall(regex, clue)
            if len(matches) > 0:
                result = matches[0]
        if category in ['univ_in', 'mayor_of']:
            regex = '<LOCATION>(.*?)</LOCATION>'
            matches = re.findall(regex, clue)
            if len(matches) == 1:
                match2 = re.search('LOCATION>, ([A-Za-z]+)', clue)
                match3 = re.search('([A-Za-z]+), <LOCATION', clue)
                if match2:
                    result = '{}, {}'.format(matches[0], match2.group(1))
                elif match3:
                    result = '{}, {}'.format(match3.group(1), matches[0])
            elif len(matches) > 1:
                print('COCKS', matches)
                result = '{}, {}'.format(matches[0], matches[1])
        
        # Answer isn't tagged, find it
        # Add case for initials (eg, AmBAR)?
        if result == '':
            import pdb
            #pdb.set_trace()
            res = []
            tokens = clue.split()
            in_entity = False
            for i in xrange(len(tokens)):
                if tokens[i][0].isupper() and '<' not in tokens[i]:
                    in_entity = True
                    res.append(tokens[i])
                else:
                    if tokens[i] in ['for', 'in'] and in_entity:
                        res.append(tokens[i])
                        continue
                    in_entity = False
                    if len(res) >= 2:
                        break
                    elif len(res) > 0:
                        res.pop()
            result = ' '.join(res).rstrip('\'s')


        return result
        

    #### You should not need to change anything after this point. ####

    def evaluate(self, parsed_clues, gold_parsed_clues):
        """Shows how the ClueParser model will score on the training/development data."""
        correct_relations = 0
        correct_parses = 0
        for parsed_clue, gold_parsed_clue in it.izip(parsed_clues, gold_parsed_clues):
            split_parsed_clue = parsed_clue.split(":")
            split_gold_parsed_clue = gold_parsed_clue.split(":")
            if split_parsed_clue[0] == split_gold_parsed_clue[0]:
                correct_relations += 1
                if (split_parsed_clue[1] == split_gold_parsed_clue[1] or
                        split_parsed_clue[1] == "The " + split_gold_parsed_clue[1] or
                        split_parsed_clue[1] == "the " + split_gold_parsed_clue[1]):
                    correct_parses += 1
                else:
                    print('Parse error:', (parsed_clue, gold_parsed_clue))
            else:
                print('Relation error:', (parsed_clue, gold_parsed_clue))
        print "Correct Relations: %d/%d" % (correct_relations, len(gold_parsed_clues))
        print "Correct Full Parses: %d/%d" % (correct_parses, len(gold_parsed_clues))
        print "Total Score: %d/%d" % (correct_relations + correct_parses, 2 * len(gold_parsed_clues))

def loadList(file_name):
    """Loads text files as lists of lines. Used in evaluation."""
    with open(file_name) as f:
        l = [line.strip() for line in f]
    return l

def main():
    """Tests the model on the command line. This won't be called in
        scoring, so if you change anything here it should only be code 
        that you use in testing the behavior of the model."""

    clues_file = "data/part1-clues.txt"
    parsed_clues_file = "data/part1-parsedclues.txt"
    cp = ClueParser()

    clues = loadList(clues_file)
    gold_parsed_clues = loadList(parsed_clues_file)
    assert(len(clues) == len(gold_parsed_clues))

    cp.train(clues, gold_parsed_clues)
    parsed_clues = cp.parseClues(clues)
    cp.evaluate(parsed_clues, gold_parsed_clues)

if __name__ == '__main__':
    main()
