import sys
import re

from pyspark import SparkContext,SparkConf

def flatMapFunc(document):
    """
    document[0] is the document ID (distinct for each document)
    document[1] is a string of all text in that document

    You will need to modify this code.
    """
    documentID = document[0]
    words = re.findall(r"\w+", document[1])
    return [(word, documentID, index) for index, word in enumerate(words)]

def mapFunc(arg):
    """
    You may need to modify this code.
    """
    return ((arg[0], arg[1]), arg[2])

def reduceFunc(arg1, arg2):
    """
    You may need to modify this code.
    """
    return (arg1, arg2)

def createIndices(file_name, output="spark-wc-out-createIndices"):
    sc = SparkContext("local[8]", "CreateIndices", conf=SparkConf().set("spark.hadoop.validateOutputSpecs", "false"))
    file = sc.sequenceFile(file_name)

    indices = file.flatMap(flatMapFunc).map(mapFunc).groupByKey().map(lambda arg : (arg[0], tuple(arg[1]))).sortByKey()

    indices.coalesce(1).saveAsTextFile(output)

""" Do not worry about this """
if __name__ == "__main__":
    argv = sys.argv
    if len(argv) == 2:
        createIndices(argv[1])
    else:
        createIndices(argv[1], argv[2])
