import json
import urllib2
import StringIO
from base64 import b64encode as bar
from base64 import b64decode as baz

import SpamLord

URL = (
    'aHR0cHM6Ly9zdWNsYXNzLnN0YW5mb3JkLmVkdS9hc3NldC12MTpFbmdpbmVlcm'
    'luZytDUzEyNCtXaW50ZXIyMDE3K3R5cGVAYXNzZXQrYmxvY2tAcGExLnR4dA=='
)
HONOR_INPUT_MESSAGE = (
    'Enter your Student ID number (8 digits) to confirm your '
    'agreement of the Stanford Honor Code: '
)
TRAIN_COPY_MESSAGE = (
    '\n^^^ Copy and Paste this into the training data field in '
    'Open edX ^^^\n'
)
TEST_COPY_MESSAGE = (
    '\n^^^ Copy and Paste this into the test data field in Open edX ^^^\n'
)
ID_NUMBER_LENGTH_MESSAGE = (
    '\nPlease enter your 8 digit Student ID number.'
)
HONOR_CODE_AGREE_MESSAGE = (
    '\nYou must agree to the Stanford Honor Code to run your code.'
)


def run_spam_lord(partId, ch_aux):
    """
    Intentionally adding '\n' to print output for readability.
    """
    res = []

    print(
        "== Running your code with the {partId} data ...".format(
            partId=partId,
        )
    )

    if partId == 'training':
        res = SpamLord.process_dir('../data/dev')
    elif partId == 'test':
        res = SpamLord.process_file('foo', StringIO.StringIO(ch_aux))

    print('== Finished running your code:\n')

    res_json = json.dumps(res)
    return res_json

honor_code = raw_input(HONOR_INPUT_MESSAGE)

if honor_code:
    if len(honor_code) == 8:
        foo = urllib2.urlopen(
            baz(URL)
        ).read()

        train_output = bar(
            '{{"{honor_code}": {train_values}}}'.format(
                honor_code=honor_code,
                train_values=run_spam_lord('training', ""),
            )
        )
        print(train_output)
        print(TRAIN_COPY_MESSAGE)

        test_output = bar(
            '{{"{honor_code}": {test_values}}}'.format(
                honor_code=honor_code,
                test_values=run_spam_lord('test', baz(foo)),
            )
        )
        print(test_output)
        print(TEST_COPY_MESSAGE)
    else:
        print(ID_NUMBER_LENGTH_MESSAGE)
else:
    print(HONOR_CODE_AGREE_MESSAGE)
