import sys
import os
import re
import pprint

from HTMLParser import HTMLParser

basic = '([\w.]+) ?@ ?(\w+)[;. ](edu|com)'
basic2 = '([\w.]+) ?(?:@|[( ]at[) ]) ?(\w+)[;. ](\w+)[;. ](edu|com)'

spec1 = '(\w[^\w])+@.*(\w[^\w])+\.'
spec2 = "(?:obfuscate|hide|obscure)\('([\w.]+)', ?'([\w.]+)'\)"
spec3 = 'apache/[0-9]\.[0-9]\.[0-9]+ \(\w+\)'
spec4 = '([\w.]+) \(followed by .@(?:(\w+)\.)?(\w+)\.(\w+).\)'
spec5 = '(\w+) \([\w ]+\) (\w+)\.(\w+)\.(\w+)'

phone1 = '\(?([0-9]{3})[-) .]{0,2}([0-9]{3})[ -.]([0-9]{4})(?:[^0-9]|$)'
def find_phones(line, filename):
    res = []
    for regex in [phone1]:
        matches = re.findall(regex, line)
        for m in matches:
            if len(m[0]) == 1:
                print m
            phone = '%s-%s-%s' % m
            res.append((filename, 'p', phone))

    return res

def find_email(line, filename):
    res = []
    line = line.lower()

    if re.search(spec3, line):
        return []

    if (re.search(" do?t ", line) or re.search("e-?mail", line) or
        re.search("stanford[\w ]+.edu", line)):
        line = re.sub(" do?t ", ".", line)
        line = re.sub(" at ", "@", line)
        line = re.sub(" where ", "@", line)
        line = re.sub(" dom ", ".", line)

    test = re.findall(spec1, line)
    if len(test) > 0:
        char = line[line.index(test[0][0][1])]
        line = re.sub(char, "", line)
        print line

    matches = re.findall(basic, line)
    for m in matches:
        email = '%s@%s.%s' % m
        res.append((filename, 'e', email))

    matches = re.findall(spec4, line)
    for m in matches:
        if not all(m):
            m = (m[0], m[2], m[3])

        if len(m) > 3:
            email = '%s@%s.%s.%s' % m
        else:
            email = '%s@%s.%s' % m
        res.append((filename, 'e', email))

    matches = re.findall(basic2, line)
    for m in matches:
        email = "%s@%s.%s.%s" % m
        res.append((filename, 'e', email))

    matches = re.findall(spec5, line)
    for m in matches:
        email = "%s@%s.%s.%s" % m
        res.append((filename, 'e', email))

    matches = re.findall(spec2, line)
    for m in matches:
        email = "%s@%s" % (m[1], m[0])
        res.append((filename, 'e', email))


    return res


def process_file(name, f):
    """
    TODO
    This function takes in a filename along with the file object (actually
    a StringIO object at submission time) and
    scans its contents against regex patterns. It returns a list of
    (filename, type, value) tuples where type is either an 'e' or a 'p'
    for e-mail or phone, and value is the formatted phone number or e-mail.
    The canonical formats are:
         (name, 'p', '###-###-#####')
         (name, 'e', 'someone@something')
    If the numbers you submit are formatted differently they will not
    match the gold answers

    NOTE: ***don't change this interface***, as it will be called directly by
    the submit script

    NOTE: You shouldn't need to worry about this, but just so you know, the
    'f' parameter below will be of type StringIO at submission time. So, make
    sure you check the StringIO interface if you do anything really tricky,
    though StringIO should support most everything.
    """
    # note that debug info should be printed to stderr
    # sys.stderr.write('[process_file]\tprocessing file: %s\n' % (path))
    h = HTMLParser()
    res = []
    for line in f:
        line = re.sub("&thinsp;", " ", line)
        line = h.unescape(unicode(line, 'ISO-8859-1'))
        res += find_email(line, name)
        res += find_phones(line, name)
    return res


def process_dir(data_path):
    """
    You should not need to edit this function, nor should you alter
    its interface as it will be called directly by the submit script
    """
    # get candidates
    guess_list = []
    for fname in os.listdir(data_path):
        if fname[0] == '.':
            continue
        path = os.path.join(data_path, fname)
        f = open(path, 'r')
        f_guesses = process_file(fname, f)
        guess_list.extend(f_guesses)
    return guess_list


def get_gold(gold_path):
    """
    You should not need to edit this function.
    Given a path to a tsv file of gold e-mails and phone numbers
    this function returns a list of tuples of the canonical form:
    (filename, type, value)
    """
    # get gold answers
    gold_list = []
    f_gold = open(gold_path, 'r')
    for line in f_gold:
        gold_list.append(tuple(line.strip().split('\t')))
    return gold_list


def score(guess_list, gold_list):
    """
    You should not need to edit this function.
    Given a list of guessed contacts and gold contacts, this function
    computes the intersection and set differences, to compute the true
    positives, false positives and false negatives.  Importantly, it
    converts all of the values to lower case before comparing
    """
    guess_list = [
        (fname, _type, value.lower())
        for (fname, _type, value)
        in guess_list
    ]
    gold_list = [
        (fname, _type, value.lower())
        for (fname, _type, value)
        in gold_list
    ]
    guess_set = set(guess_list)
    gold_set = set(gold_list)

    tp = guess_set.intersection(gold_set)
    fp = guess_set - gold_set
    fn = gold_set - guess_set

    pp = pprint.PrettyPrinter()
    # print 'Guesses (%d): ' % len(guess_set)
    # pp.pprint(guess_set)
    # print 'Gold (%d): ' % len(gold_set)
    # pp.pprint(gold_set)
    print 'True Positives (%d): ' % len(tp)
    pp.pprint(tp)
    print 'False Positives (%d): ' % len(fp)
    pp.pprint(fp)
    print 'False Negatives (%d): ' % len(fn)
    pp.pprint(fn)
    print 'Summary: tp=%d, fp=%d, fn=%d' % (len(tp), len(fp), len(fn))


def main(data_path, gold_path):
    """
    You should not need to edit this function.
    It takes in the string path to the data directory and the
    gold file
    """
    guess_list = process_dir(data_path)
    gold_list = get_gold(gold_path)
    score(guess_list, gold_list)

"""
commandline interface takes a directory name and gold file.
It then processes each file within that directory and extracts any
matching e-mails or phone numbers and compares them to the gold file
"""
if __name__ == '__main__':
    if (len(sys.argv) != 3):
        print 'usage:\tSpamLord.py <data_dir> <gold_file>'
        sys.exit(0)
    main(sys.argv[1], sys.argv[2])
