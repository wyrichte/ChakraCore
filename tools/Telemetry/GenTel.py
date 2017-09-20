#!/usr/bin/python
# SCOPE job generation script

import os
import re
import sys
import string
import shutil

def grabAndFilterFileContents(filename):
    data = []

    # grab the file to process
    with open(filename, "r") as infile:
        data = infile.readlines()

    # sanity check 1 - no /* */ comments
    blockCommentChecker = re.compile("\/\*")
    endBlockCommentCatcher = re.compile("\*\/")
    for line in data:
        if blockCommentChecker.search(line) != None: # I know this is weak as far as validation goes
            if endBlockCommentCatcher.search(line) == None:
                print("block comments longer than one line (in "+filename+"not supported (line follows)")
                print(line)
                exit(1)

    # cleanup 1 - remove // comments
    lineCommentChecker = re.compile("\s*\/\/.*$")
    data = map(lambda x: lineCommentChecker.sub("", x), data)

    # cleanup 1.5 - remove /* comments by treating as //
    blockCommentChecker = re.compile("\s*\/\*.*$")
    data = map(lambda x: blockCommentChecker.sub("", x), data)

    # cleanup 2 - remove # preprocessor statements
    # note - this means that if you just #if 0 a block out, it'll still get parsed
    preProcessorChecker = re.compile("^\s*#")
    data = filter(lambda x: preProcessorChecker.search(x) == None, data)

    # cleanup 3 - strip all lines
    data = map(lambda x: x.strip(), data)

    # cleanup 4 - remove blank lines
    data = filter(lambda x: x != "", data)

    # convert back to a list, otherwise iteration destroys it -_-
    data = list(data)

    # sanity check 2 - no // comments remaining
    lineCommentChecker2 = re.compile("\/\/")
    for line in data:
        if lineCommentChecker2.search(line) != None:
            print("non-line-starting comments not supported")
            exit(1)

    return data

# Most complex of the three file types as it contains 
def parseBuiltinHeaderFile(filename):
    data = grabAndFilterFileContents(filename)
    
    # group by block, and assemble langfeatures and telpoints into arrays
    blocks = {}
    langFeatures = []
    telPoints = []
    currentBlock = ""
    numSeenInBlock = 0
    numExpectedInBlock = 0

    blockStartMatcher = re.compile("^BLOCK_START\(\s*([a-zA-Z0-9_]+)\s*,\s*([0-9]+)\s*\)\s*$")
    entryBuiltinMatcher = re.compile("^ENTRY_BUILTIN\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\)\s*$")
    blockEndMatcher = re.compile("^BLOCK_END\(\s*\)\s*$")
    langFeatureMatcher = re.compile("^ENTRY_LANGFEATURE\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\)\s*$")
    telPointMatcher = re.compile("^ENTRY_TELPOINT\(s*([a-zA-Z0-9_]+)\s*\)\s*$")

    for line in data:
        blockStart = blockStartMatcher.match(line)
        entryBuiltin = entryBuiltinMatcher.match(line)
        blockEnd = blockEndMatcher.match(line)
        langFeature = langFeatureMatcher.match(line)
        telPoint = telPointMatcher.match(line)
        if blockStart:
            if currentBlock != "":
                print("Error: block_start should not be in a block!")
                exit(1)
            currentBlock = blockStart.group(1)
            numSeenInBlock = 0
            numExpectedInBlock = int(blockStart.group(2))
            blocks[currentBlock] = []
        elif entryBuiltin:
            if currentBlock == "":
                print("Error: entry_builtin should be in a block!")
                exit(1)
            if numSeenInBlock >= numExpectedInBlock:
                print("Error: too many builtin entries in block "+currentBlock+"!")
                exit(1)
            blocks[currentBlock].append({
                "EMCAVersion": entryBuiltin.group(1),
                "baseObject": entryBuiltin.group(2),
                "functionResidence": entryBuiltin.group(3),
                "functionName": entryBuiltin.group(4)
            })
            numSeenInBlock = numSeenInBlock + 1
        elif blockEnd:
            if currentBlock == "":
                print("Error: block_end should be in a block!")
                exit(1)
            if numSeenInBlock < numExpectedInBlock:
                print("Error: insufficiently many entries in block "+currentBlock+"!")
                exit(1)
            currentBlock = ""
        elif langFeature:
            if currentBlock != "":
                print("Error: entry_langfeature should not be in a block!")
                exit(1)
            langFeatures.append({
                "EMCAVersion": langFeature.group(1),
                "pointName": langFeature.group(2)
            })
        elif telPoint:
            if currentBlock != "":
                print("Error: entry_telpoint should not be in a block!")
                exit(1)
            telPoints.append({
                "pointName": telPoint.group(1)
            })
        else:
            print("Error: unknown line in "+filename+" (printed below)!")
            print(line)
            exit(1)

    return (data, blocks, telPoints, langFeatures)

def parseBailOutHeaderFile(filename):
    data = grabAndFilterFileContents(filename)

    # here we need two straightforward sets of data - BailOutKinds and BailOutKindValues
    # we only need their names - the allowed bits and the actual bit mappings are irrelevant
    bailOutKinds = []
    bailOutKindValues = []

    bailOutKindMatcher = re.compile("^BAIL_OUT_KIND\(\s*([a-zA-Z0-9_]+),.*\)\s*$")
    bailOutKindValueMatcher = re.compile("^BAIL_OUT_KIND_VALUE(?:_LAST)?\(\s*([a-zA-Z0-9_]+),.*\)\s*$")

    for line in data:
        bailOutKind = bailOutKindMatcher.match(line)
        bailOutKindValue = bailOutKindValueMatcher.match(line)
        if bailOutKind:
            bailOutKinds.append({
                "kindName": bailOutKind.group(1)
            })
        elif bailOutKindValue:
            bailOutKindValues.append({
                "valueName": bailOutKindValue.group(1)
            })
        else:
            print("Error: unknown line in "+filename+" (printed below)!")
            print(line)
            exit(1)
    return (data, bailOutKinds, bailOutKindValues)

def parseRejitHeaderFile(filename):
    data = grabAndFilterFileContents(filename)

    # we just need to get the tokens out - pretty straightforward
    rejitReasons = []

    rejitReasonMatcher = re.compile("^REJIT_REASON\(\s*([a-zA-Z0-9_]+)\s*\)\s*$")

    for line in data:
        rejitReason = rejitReasonMatcher.match(line)
        if rejitReason:
            rejitReasons.append({
                "reasonName": rejitReason.group(1)
            })
        else:
            print("Error: unknown line in "+filename+" (printed below)!")
            print(line)
            exit(1)

    return (data, rejitReasons)

# simple template replacement on a string
def replacetemplates(filestring, bidata, blocks, langFeatures, telPoints, bodata, bailOutKinds, bailOutKindValues, rjdata, rejitReasons):
    replacementTokenMatcher = re.compile("<<<\s*([^>]*)\s*>>>")
    match = replacementTokenMatcher.search(filestring)
    if not match:
        return False

    matchedstring = match.group(1)

    # figure out what the replacement string should be
    replstring = "UNMATCHED"
    forallprint = re.compile("^FORALL_PRINTF\(\s*([a-zA-Z]+)\s*,\s*\"([^\"]+)\"\s*,\s*\"([^\"]*)\"\s*\)")

    if matchedstring == "HEADER_FILE":
        # we want the whole header file, but comments can be ignored, so this suffices:
        replstring = "\n".join(bidata)
    elif forallprint.match(matchedstring):
        groups = forallprint.match(matchedstring)
        #print(groups.group(0), groups.group(1), groups.group(2), groups.group(3))

        # figure out what the replacement tokens and iterands are
        replacements = []
        iterands = []
        if groups.group(1) == "blocks":
            iterands = list(map(lambda x: {"name":x}, blocks.keys()))
        elif groups.group(1) == "builtins":
            iterands = []
            for block in blocks:
                iterands.extend(blocks[block])
        elif groups.group(1) == "telpoints":
            iterands = telPoints
        elif groups.group(1) == "langfeatures":
            iterands = langFeatures
        elif groups.group(1) == "rejitReasons":
            iterands = rejitReasons
        elif groups.group(1) == "bailOutKinds":
            iterands = bailOutKinds
        else:
            print("Error: Unrecognized forall type "+groups.group(1)+" used in file!")
            exit(1)
        
        # sanity check
        if iterands == []:
            print("Error: forall block specifier "+groups.group(1)+" got an empty iterand set, this invalidates some assumptions!")
            exit(1)

        # replace a couple of complicated tokens in the format string
        formatstring = groups.group(2).replace("&quot;","\"")

        # iterate, doing the replacements
        #print(iterands)
        percentTokenMatcher = re.compile("%([a-zA-Z]+)")

        replstring = ""
        resultstrings = []
        for iterand in iterands:
            localstring = formatstring
            while percentTokenMatcher.search(localstring):
                localgroups = percentTokenMatcher.search(localstring)
                (localstring, count) = percentTokenMatcher.subn(iterand[localgroups.group(1)], localstring, count=1)
                if count != 1:
                    print("Error: regex integrity failure!")
                    exit(1)
            resultstrings.append(localstring)
        replstring = groups.group(3).join(resultstrings)


    # validity check, then do the actual replacement and return
    if replstring == "UNMATCHED":
        print("Error: unable to handle replacement rule (follows below)!", matchedstring)
        exit(1)
    (newstring, count) = replacementTokenMatcher.subn(replstring, filestring, count=1)
    if count != 1:
        print("Error: regex integrity failure!")
        exit(1)
    return newstring

def main():
    # resolve any environment weirdness
    basepath = os.path.dirname(os.path.realpath(__file__))
    
    # parse builtin info
    (bidata, blocks, telPoints, langFeatures) = parseBuiltinHeaderFile(os.path.join(basepath, "../../private/lib/Telemetry/ESBuiltIns/ESBuiltinFields.h"))

    # parse bailout info
    (bodata, bailOutKinds, bailOutKindValues) = parseBailOutHeaderFile(os.path.join(basepath, "../../core/lib/Backend/BailOutKind.h"))

    # parse rejit info
    (rjdata, rejitReasons) = parseRejitHeaderFile(os.path.join(basepath, "../../core/lib/Common/Common/RejitReasons.h"))

    # loop over the files, replacing tokens as specified
    directdir = os.path.join(basepath, "direct")
    templatedir = os.path.join(basepath, "templates")
    outputdir = os.path.join(basepath, "output")
    if not os.path.isdir(templatedir):
        print("Error: template directory not found at "+templatedir+" !")
        exit(1)

    if not os.path.isdir(outputdir):
        os.mkdir(outputdir)

    for root, dirs, files in os.walk(directdir):
        for file in files:
            print("Direct-copying "+os.path.join(os.path.relpath(root,directdir), file)+"...")
            shutil.copyfile(os.path.join(root, file), os.path.join(outputdir, os.path.relpath(root, directdir), file))

    for root, dirs, files in os.walk(templatedir):
        for file in files:
            relpath = os.path.relpath(os.path.join(root, file), templatedir)
            print("Generating "+relpath+"...", end='')
            basecontents = ""
            with open(os.path.join(templatedir, relpath), "r") as inputfile:
                basecontents = inputfile.read()

            with open(os.path.join(outputdir, relpath), "w") as outputfile:
                replaced = True
                while replaced:
                    replaced = replacetemplates(basecontents, bidata, blocks, langFeatures, telPoints, bodata, bailOutKinds, bailOutKindValues, rjdata, rejitReasons)
                    if replaced:
                        print(".", end='')
                        basecontents = replaced
                outputfile.write(basecontents)
            print('')

if __name__ == "__main__":
    main()
