import collections
import itertools
import operator
import sys

def groupEquivs(mappings):
    charToEquivs = collections.defaultdict(set)
    for mapping in mappings:
        (char, mappedChar) = mapping
        group = charToEquivs[mappedChar]
        group.add(char)
    for char in charToEquivs:
        charToEquivs[char].add(char)
    allEquivs = set(map(frozenset, charToEquivs.values()))
    return allEquivs

def generateUnicodeDataEquivs():
    #UPPERCASE_COLUMN = 12
    #LOWERCASE_COLUMN = 13
    TITLECASE_COLUMN = 14
    caseColumn = TITLECASE_COLUMN
    def readUnicodeDataFile():
        lines = open(sys.argv[1]).readlines()
        records = (list(map(str.strip, line.split(';'))) for line in lines)
        return records
    records = readUnicodeDataFile()
    lowercaseRecords = filter(lambda record: record[2][:2] == 'Ll', records)
    def isValidRecord(record):
        char = record[0]
        mappedChar = record[caseColumn]
        if not mappedChar:
            return False
        charValue = int(char, 16)
        mappedCharValue = int(mappedChar, 16)
        return (charValue <= 0xFFFF
            and mappedCharValue <= 0xFFFF
            and not (charValue >= 128 and mappedCharValue < 128))
    validRecords = filter(isValidRecord, lowercaseRecords)
    mappings = map(lambda record: (int(record[0], 16), int(record[caseColumn], 16)), validRecords)
    allEquivs = groupEquivs(mappings)
    return allEquivs

def generateCaseFoldingEquivs():
    def readCaseFoldingFile():
        lines = open(sys.argv[2]).readlines()
        lines = map(str.strip, lines) # Remove newlines
        lines = filter(lambda line: line[:1] not in ('', '#'), lines)
        records = (list(map(str.strip, line.split(';'))) for line in lines)
        return records
    records = readCaseFoldingFile()
    recordsWithAllowedStatuses = filter(lambda x: x[1] in {'C', 'S'}, records)
    mappings = map(lambda record: (int(record[0], 16), int(record[2], 16)), recordsWithAllowedStatuses)
    allEquivs = groupEquivs(mappings)
    return allEquivs

def printEquivs(allEquivs):
    print('Length: %d' % len(allEquivs))
    sortedAllEquivs = sorted(allEquivs, key = min)
    for equivs in sortedAllEquivs:
        sortedEquivs = list(sorted(equivs))
        hexValues = list(map(hex, sortedEquivs))
        chars = list(map(chr, sortedEquivs))
        print(hexValues, chars)

def printTransforms(allEquivs, source):
    allEquivs = list(allEquivs)
    numDeltas = max(map(len, allEquivs))
    def calculateDeltas(char, equivs):
        equivs = list(sorted(equivs))
        return list(map(lambda equiv: equiv - char, equivs))
    def padDeltas(deltas):
        return deltas + ([deltas[-1]] * (numDeltas - len(deltas)))
    def convertToCanonicalHex(number):
        return '0x' + hex(number)[2:].zfill(4)
    Transform = collections.namedtuple('Transform', 'char deltas')
    def generateTransforms(equivs):
        equivs = set(equivs)
        for char in equivs:
            deltas = calculateDeltas(char, equivs)
            paddedDeltas = padDeltas(deltas)
            yield Transform(char, paddedDeltas)
    def rangifyTransforms(transforms):
        sortedTransforms = sorted(transforms, key = lambda transform: transform.char)
        transformGroupsWithSameDelta = itertools.groupby(sortedTransforms, key = lambda transform: transform.deltas)
        for _, transformsWithSameDelta in transformGroupsWithSameDelta:
            transformsWithSameDelta = list(transformsWithSameDelta)
            transformGroupsInSameRange = itertools.groupby(enumerate(transformsWithSameDelta), lambda y: y[1].char - y[0])
            transformGroupsInSameRange = map(operator.itemgetter(1), transformGroupsInSameRange) # We don't need the group key
            for transformsInSameRange in transformGroupsInSameRange:
                transformsInSameRange = map(lambda x: list(x)[1], transformsInSameRange) # We don't need the enumeration value
                transformsInSameRange = list(transformsInSameRange)
                rangeStart = min(map(lambda x: x.char, transformsInSameRange))
                rangeEnd = max(map(lambda x: x.char, transformsInSameRange))
                yield (rangeStart, rangeEnd, transformsInSameRange[0].deltas)
    def printRangifiedTransforms(transforms):
        for transform in transforms:
            rangeStart, rangeEnd, deltas = transform
            paddedDeltas = padDeltas(deltas)
            arrayEntry = list(map(str, [1, convertToCanonicalHex(rangeStart), convertToCanonicalHex(rangeEnd), source] + list(paddedDeltas)))
            ## ',' is there at the end because the line is going to be inserted into an array
            arrayEntry = ', '.join(arrayEntry) + ','
            print(arrayEntry)

    transforms = itertools.chain.from_iterable(map(generateTransforms, allEquivs))
    rangifiedTransforms = rangifyTransforms(transforms)
    printRangifiedTransforms(rangifiedTransforms)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        usage = 'Usage: %s <UnicodeData.txt> <CaseFolding.txt>' % sys.argv[0]
        print(usage, file = sys.stderr)
        sys.exit(-1)

    unicodeDataEquivs = generateUnicodeDataEquivs()
    caseFoldingEquivs = generateCaseFoldingEquivs()
    caseFoldingOnlyEquivs = caseFoldingEquivs - unicodeDataEquivs
    printTransforms(caseFoldingOnlyEquivs, 'MappingSource::CaseFolding')
