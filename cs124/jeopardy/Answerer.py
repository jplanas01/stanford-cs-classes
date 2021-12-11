#!/usr/bin/env python
# CS124 Homework 5 Jeopardy
# Original written in Java by Sam Bowman (sbowman@stanford.edu)
# Ported to Python by Milind Ganjoo (mganjoo@stanford.edu)

from ClueParser import ClueParser
import itertools as it
import re
import sys

class Answerer:
    wiki_filename = "data/wiki-text-ner.txt"
    def answer(self, parsed_clues):
        """Answer each clue and return a list of answers."""
        answers = []
        
        # Add 'No answer.' as the answer when you don't find an answer you are confident of.
        # We recommend using Hearst style patterns to find answers within the Wiki text.
        for parsed_clue in parsed_clues:
            clue_type, clue_entity = parsed_clue.split(":")
            name = clue_entity.split()[0] + clue_entity.split()[-1]
            if clue_type == 'born_in':
                answers.append(self.do_born_in(name, clue_entity))
            elif clue_type == 'year_of_birth':
                answers.append(self.do_yob(name, clue_entity))
            else:
                answers.append('No answer.')

        return answers
    
    def do_born_in(self, name, clue_entity):
        lines = loadList(self.wiki_filename)
        watching = False
        for line in lines:
            
            if watching:
                match = re.search(
                '<LOCATION>([a-zA-Z .]+)</LOCATION>, <LOCATION>([a-zA-Z .]+)</LOCATION>',
                line)
                if match:
                    return 'What is {}, {}?'.format(match.group(1), match.group(2))
                    
            # Find person
            person = re.search('<TITLE><PERSON>([A-Za-z. ]+)</PERSON>', line)
            if not person:
                continue
            cur_name = person.group(1).split()[0] + person.group(1).split()[-1]
            if cur_name == name:
                # No good answer found...
                if watching:
                    break
                watching = True

        # Not a titled person, search through all people.
        for line in lines:
            persons = re.findall('<PERSON>([A-Za-z. ]+)<', line)
            if len(persons) < 1:
                continue

            for person in persons:
                cur_name = person.split()[0] + person.split()[-1]
                if cur_name == name:
                    pos = line.find(clue_entity)
                    if pos < 0:
                        break
                    line = line[pos:]
                    match = re.search(
                    '<LOCATION>([a-zA-Z .]+)</LOCATION>, <LOCATION>([a-zA-Z .]+)</LOCATION>',
                    line)
                    if match:
                        return 'What is {}, {}?'.format(match.group(1), match.group(2))

        return 'No answer.'
    
    def do_yob(self, name, clue_entity):
        lines = loadList(self.wiki_filename)
        watching = False
        
        for line in lines:
            if watching:
                match = re.search(' [0-9]{1,2}, ([0-9]{4})', line)
                if match:
                    return 'What is {}?'.format(match.group(1))


            # Find person
            person = re.search('<PERSON>([A-Za-z. ]+)<', line)
            if not person:
                continue
            cur_name = person.group(1).split()[0] + person.group(1).split()[-1]
            if cur_name == name:
                # No good answer found...
                if watching:
                    break
                watching = True

        for line in lines:
            persons = re.findall('<PERSON>([A-Za-z. ]+)<', line)
            if len(persons) < 1:
                continue
            for person in persons:
                cur_name = person.split()[0] + person.split()[-1]
                if cur_name == name:
                    pos = line.find(clue_entity)
                    if pos < 0:
                        break
                    line = line[pos:]
                    match = re.search(' ([0-9]{4})[^0-9]', line)
                    if match:
                        return 'What is {}?'.format(match.group(1))

        return 'No answer.'



    # TODO: This method is merely a piece of starter code that we think you will find useful. If you use it,
    # you may want to modify it to make it more useful. You may modify it however you like, and you may also
    # change the return type or the argument list, since we won't call this function directly during grading.
    def searchForPatterns(self, pattern_strings, pattern_positions, filename):
        """Searches a text file using several regular expressions at the same time, and returns one match. If more than
            one regular expression matches a piece of the file, the order of the regular expressions in the list
            is used to decide which match to return, so you should place more precise regular expressions earlier
            in the list. If more than one piece of the file matches the same regular expression, the last match
            in the file will be returned.
            
            patternStrings should be a list of regular experessions, ordered from most precise to least precise.
            
            patternPositions should be a list of integers of the same length as the list of regular expressions. and
            should indicate which group within each regular expression should be returned. Group 0 contains the entire
            match, group 1 contains the piece of the match inside first set of parantheses of the regular expression,
            group 2 contains the piece inside the second set of parentheses, etc.
            
            For example, for the regular expression "He was (maybe|probably) born in (\\d\\d\\d\\d)" and a file with
            the line "He was maybe born in 1899, and died in 1867.", these will be the groups you can choose from:
            - Group 0: "He was maybe born in 1899"
            - Group 1: "maybe"
            - Group 2: "1899"
            
            So if "1899" is the string that you want to include in your Jeopardy answer, you set the entry in patternPositions
            corresponding to this regular exrpession to 2.
            
            filename should be the filename for the text file being searched."""
        patterns = []
        for pattern_string in pattern_strings:
            patterns.append(re.compile(pattern_string, re.IGNORECASE))
        
        # Iterate over the lines of the file.
        lines = loadList(filename)
        matched_groups = {}
        for line in lines:
            for i, p in enumerate(patterns):
                m = p.search(line)
                if m:
                    matched_groups[i] = m.group(pattern_positions[i])
        
        # Return the match corresponding to the first regular expression in the list.
        if len(matched_groups) > 0:
            return matched_groups[min(matched_groups.keys())]
        else:
            return None
    
    #### You should not need to change anything after this point. ####

    def evaluate(self, guessed_answers, guessed_answers_from_parses, gold_answers):
        """Shows you how your model will score on the training/development data."""
        print "Guessed answers from clues:"
        score1 = self.evaluateAnswerSet(guessed_answers, gold_answers);
        print "Guessed answers from gold parses:"
        score2 = self.evaluateAnswerSet(guessed_answers_from_parses, gold_answers);
        print "Total score: {0}".format(score1 + score2)
    
    def evaluateAnswerSet(self, guessed_answers, gold_answers):
        """Scores one set of answers."""
        correct = 0
        wrong = 0
        no_answers = 0
        score = 0
        
        for guessed_answer, gold_answer_line in it.izip(guessed_answers, gold_answers):
            example_wrong = True # guilty until proven innocent
            if guessed_answer == "No answer.":
                example_wrong = False
                no_answers += 1
            else:
                golds = gold_answer_line.split("|")
                for gold in golds:
                    if guessed_answer == gold:
                        correct += 1
                        score += 1
                        example_wrong = False
                        break
            if example_wrong:
                wrong += 1
                score -= 0.5
        print "Correct Answers: {0}".format(correct)
        print "No Answers: {0}".format(no_answers)
        print "Wrong Answers: {0}".format(wrong)
        print "Score: {0}".format(score)
        
        return score

def loadList(file_name):
    """Loads text files as lists of lines. Used in evaluation."""
    with open(file_name) as f:
        l = [line.strip() for line in f]
    return l

def main():
    """Tests the model on the command line. This won't be called in
        scoring, so if you change anything here it should only be code
        that you use in testing the behavior of the model."""
    clues_file = "data/part2-clues.txt"
    parses_file = "data/part2-parses.txt"
    gold_file = "data/part2-gold.txt"

    clues_val_file = "data/part2-clues-val.txt"
    parses_val_file = "data/part2-parses-val.txt"
    gold_val_file = "data/part2-gold-val.txt"
    
    training_clues_file = "data/part1-clues.txt"
    training_parsed_clues_file = "data/part1-parsedclues.txt"
    
    a = Answerer()
    
    # Test on unparsed questions.
    clues = loadList(clues_file)
    training_clues = loadList(training_clues_file)
    training_parsed_clues = loadList(training_parsed_clues_file)
    
    cp = ClueParser()
    cp.train(training_clues, training_parsed_clues)
    guessed_parses = cp.parseClues(clues)
    guessed_answers = a.answer(guessed_parses)
    # Test on parsed questions.
    gold_parses = loadList(parses_file)
    guessed_answers_from_parses = a.answer(gold_parses)
    
    # Evaluate both sets of answers.
    gold_answers = loadList(gold_file)
    a.evaluate(guessed_answers, guessed_answers_from_parses, gold_answers)

    validation_flag = False
    if len(sys.argv) > 1 and sys.argv[1] == '-v':
        print "\nValidation results:"
        clues_val = loadList(clues_val_file)
        guessed_parses_val = cp.parseClues(clues_val)
        guessed_answers_val = a.answer(guessed_parses_val)
        # Test on parsed questions.
        gold_parses_val = loadList(parses_val_file)
        guessed_answers_val_from_parses = a.answer(gold_parses_val)
        
        # Evaluate both sets of answers.
        gold_answers_val = loadList(gold_val_file)
        a.evaluate(guessed_answers_val, guessed_answers_val_from_parses, gold_answers_val)



if __name__ == '__main__':
    main()
