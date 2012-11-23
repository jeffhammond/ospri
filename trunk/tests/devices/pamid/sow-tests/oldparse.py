#!/usr/bin/python

import os
import sys
#import string

def parse(filename):
  file = open(filename,'r')

  # for each target + message type: total count, total bytes, contig count, strided count, total stride (>1)
  l,w = 2, 5
  getStats = [[0] * w for i in range(l)]
  putStats = [[0] * w for i in range(l)]
  accStats = [[0] * w for i in range(l)]

  for line in file.readlines():
    if 'Get' in line:
      temp = line.split(' ')
      for something in temp:
        if something != '':
          timestamp = float(something)
          break
      
      #print 'temp = ',temp
      for something in temp:
        if something != '':
          if something[0:2] == 't=':
            target = int(something.split('=')[1])
          elif something[0:2] == 'b=':
            bytes  = int(something.split('=')[1])
          elif something[0:3] == 'c0=':
            count0 = int(something.split('=')[1])
          elif something[0:3] == 'c1=':
            count1 = int(something.split('=')[1])
          elif something[0:3] == 'dt=':
            dt     = float(something.split('=')[1])
          elif something[0:3] == 'bw=':
            bw     = float(something.split('=')[1])
      getStats[target][0] += 1          # count
      getStats[target][1] += bytes      # bytes
      if count1>1:
        getStats[target][3] += 1        # strided count
        getStats[target][4] += count1   # total stride
      else:
        getStats[target][2] += 1        # contig count

    elif 'Put' in line:
      temp = line.split(' ')
      for something in temp:
        if something != '':
          timestamp = float(something)
          break
      
      #print 'temp = ',temp
      for something in temp:
        if something != '':
          if something[0:2] == 't=':
            target = int(something.split('=')[1])
          elif something[0:2] == 'b=':
            bytes  = int(something.split('=')[1])
          elif something[0:3] == 'c0=':
            count0 = int(something.split('=')[1])
          elif something[0:3] == 'c1=':
            count1 = int(something.split('=')[1])
          elif something[0:3] == 'dt=':
            dt     = float(something.split('=')[1])
          elif something[0:3] == 'bw=':
            bw     = float(something.split('=')[1])
      putStats[target][0] += 1          # count
      putStats[target][1] += bytes      # bytes
      if count1>1:
        putStats[target][3] += 1        # strided count
        putStats[target][4] += count1   # total stride
      else:
        putStats[target][2] += 1        # contig count

    elif 'Acc' in line:
      temp = line.split(' ')
      for something in temp:
        if something != '':
          timestamp = float(something)
          break
      
      #print 'temp = ',temp
      for something in temp:
        if something != '':
          if something[0:2] == 't=':
            target = int(something.split('=')[1])
          elif something[0:2] == 'b=':
            bytes  = int(something.split('=')[1])
          elif something[0:3] == 'c0=':
            count0 = int(something.split('=')[1])
          elif something[0:3] == 'c1=':
            count1 = int(something.split('=')[1])
          elif something[0:3] == 'dt=':
            dt     = float(something.split('=')[1])
          elif something[0:3] == 'bw=':
            bw     = float(something.split('=')[1])
      accStats[target][0] += 1          # count
      accStats[target][1] += bytes      # bytes
      if count1>1:
        accStats[target][3] += 1        # strided count
        accStats[target][4] += count1   # total stride
      else:
        accStats[target][2] += 1        # contig count
      
  file.close()

  sumCount       = 0
  sumBytes       = 0
  sumStrideCount = 0
  sumStrides     = 0
  for i in range(l):
    sumCount       += accStats[target][0]
    sumBytes       += accStats[target][1]
    sumStrideCount += accStats[target][3]
    sumStrides     += accStats[target][4]
  print 'Acc bytes/msg  = ',sumBytes/sumCount
  print 'Acc stride/msg = ',sumStrides/sumStrideCount

filename = str(sys.argv[1])
print filename
parse(filename)
print 'THE END'
