hamcpp
==========


DESCRIPTION:
------------

Wrapper around fast (SSE based) Hamming distance routines using the [libpopcnt](https://github.com/kimwalisch/libpopcnt) C++ library.

FEATURES/PROBLEMS:
------------------

 - computation of Hamming distance between two string packed bitarrays
 - optimization if strings are multiples of 8 bytes long (ie bitarrays multiples of 64 bits)
 - binding pairs of strings in a array whose Hammings distance does not exceed a given threshols

SYNOPSIS:
---------------

##Given a pair of bitvectors compute their Hamming distance
```
    require 'hamcpp'
    sa = '01000010'
    sb = '01001000'
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    res = Hamcpp.new.hamming(a, b)
    # => 3
```
###When  bitvector bitlengths are multiples of 64, *hamming64* should be used in place of *hamming* for better performance.


##Given array of bitvectors, find ones whose Hamming distance falls at or below a given threshold.
```
    require 'hamcpp'
    sa = '00000000' * 2
    sb = '01000010' * 2
    sc = '01000000' * 2
    a = [sa].pack('B*')
    b = [sb].pack('B*')
    c = [sc].pack('B*')
    # expected distances:
    # a <-> b : 4  # this one will be above the chosen threshold of 2 so it should not be selected
    # a <-> c : 2
    # b <-> c : 2
    res = Hamcpp.new.hamloop([a, b, c], 2)

    #2.66667  mean dist for array of size 3
    # => [[2, 0, 2], [2, 1, 2]]
    #
    # Each row of output array contains
    #   distance, i, j
    #
    #   where i < j are the indices of the pair satisfying threshold constraint
```
###When  bitvector bitlengths are multiples of 64, *hamloop64* should be used in place of *hamloop* for better performance.

REQUIREMENTS:
--------------

Linux (linpopcount might not build on OSX)


INSTALL:
------------

 *  sudo gem install rake-compiler hoe rice
 *  rake compile
 *  rake test
 *  sudo gem build hamcpp.gemspec
 *  sudo gem install hamcpp-1.0.0.gem


CREDITS:
------------

 *  libpopcnt is licensed under BSD 2-Clause License

