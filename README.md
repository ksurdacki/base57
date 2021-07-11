# Base57

A [binary-to-text encoding](https://en.wikipedia.org/wiki/Binary-to-text_encoding) which uses
alphanumerical characters only and which has a space efficiency comparable 
to the [Base64](https://en.wikipedia.org/wiki/Base64).

## Rationale (Base64 comparison)

- No non-alphanumerical characters like: +, /, =, or alike
  - no problem in URLs parameters or identifiers encoding
  - double-clicking selects a whole text
- Five characters `0O1Il` which are looking alike in most fonts are removed
  to minimise a probability of human errors during retyping. 
- No padding
- Comparable space efficiency: 8/11 ~= 72.7% versus 3/4 = 75.0%
- Unexpectedly the Base57 C implementation is two times faster than the Base64 on Ubuntu:
  ```
  $ grep 'model name' /proc/cpuinfo 
  model name	: Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
  
  $ ls -sh initrd.img-5.8.0-59-generic 
  52M initrd.img-5.8.0-59-generic
  
  $ time base64 < initrd.img-5.8.0-59-generic > encoded.b64
  
  real	0m4,509s
  user	0m0,415s
  sys	0m1,106s
  
  $ time ./base57encode < initrd.img-5.8.0-59-generic > encoded.b57 
  
  real	0m2,087s
  user	0m0,283s
  sys	0m0,356s
  
  $ ls -sh encoded.b64 encoded.b57
  72M encoded.b57  70M encoded.b64
  
  $ time base64 -d < encoded.b64 > decoded.b64 
  
  real	0m5,777s
  user	0m0,693s
  sys	0m1,349s
  
  $ time ./base57decode < encoded.b57 > decoded.b57 
  
  real	0m2,578s
  user	0m0,059s
  sys	0m1,198s
  ```

## UUID encodings example:

```
         format         |              representation             | len |           comment
------------------------+-----------------------------------------+-----+-----------------------------
                decimal | 286442745952272770475347258668719650363 |  39 | lengthy
              canonical |    d77eddbd-dabc-4762-bf13-433c9e01b63b |  36 | lengthy and -
            hexadecimal |        d77eddbddabc4762bf13433c9e01b63b |  32 | lengthy
    base32 with padding |        257N3PO2XRDWFPYTIM6J4ANWHM====== |  32 | lengthy and O and I and =
 base32 without padding |              257N3PO2XRDWFPYTIM6J4ANWHM |  26 | O and I
    base64 with padding |                137dvdq8R2K/E0M8ngG2Ow== |  24 | 1, /, 0, O and =
 base64 without padding |                  137dvdq8R2K/E0M8ngG2Ow |  22 | 1, /, 0 and O
                 base57 |                  rbfS7bspKXGvaaExLgoqg6 |  22 | short and clean
                Ascii85 |                    f7)Phg:Tpb^CrssScQ8] |  20 | ), :, ^, and ]
                 binary |                        ×~Ý½Ú¼Gb¿C<¶; |  16 | inappropriate for text
```

## Why 57 ?

- A minimum number of [US-ASCII](https://en.wikipedia.org/wiki/ASCII) alphanumerical only characters
  (10+26+26 = 62) `N` needed for encoding a 64-bit integer is:
  ```
  62^N >= 2^64
  log(62^N, 62) >= log(2^64, 62)
  N >= log(2^64, 62)
  N >= 10.75
  ```
- A minimum number of symbols `S` needed for encoding 64-bit integer in 11 characters is:
  ```
  S^11 >= 2^64
  (S^11)^(1/11) >= (2^64)^(1/11)
  S >= 2^(64/11)
  S >= 56.43
  ```
- Conclusion is that a 64-bit integer may be encoded as an 11 characters long word
  using 57 unique symbols only.

## The algorithm description

- Going forward with the minimization we end up with the following inequality: `56^6 * 57^5 > 2^64`.
- Dividing by 56 leave us with one unused symbol from the 57 which are available.
  I make use of this excess to assure that no more than two consecutive symbols are the same.
  It serves human readability. For example: zero encoded in this way `ZYY22344556`
  is more human-readable than without it: `ZZZZZZZZZZZ`.
- Non-repetitive consecutive symbols feature of the encoding is provided by an alphabet rotation.
  In a carefully chosen symbol positions, where division by 56 is performed instead of 57,
  the alphabet is rotated in such a way that a lastly used symbol is out of an indexing from 0 to 55.
  In other words, the alphabet after rotation starts just after the lastly used symbol.
- An encoded 64 bit unsigned integer symbols patter is `ArArArrArAr`, where:
  - `A` is a symbol placeholder for a remainder of a division by 57 (all symbols possible, no rotation)
  - `r` is a symbol placeholder for a remainder of a division by 56 (all but lastly encoded symbol possible)

## Initial symbol values (the alphabet before any rotation)

```
  value |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18
--------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----
 symbol |  Z |  Y |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 |  a |  b |  c |  d |  e |  f |  g |  h |  i

  value | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37
--------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----
 symbol |  j |  k |  m |  n |  o |  p |  q |  r |  s |  t |  u |  v |  w |  x |  y |  z |  A |  B |  C

  value | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56
--------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----
 symbol |  D |  E |  F |  G |  H |  J |  K |  L |  M |  N |  P |  Q |  R |  S |  T |  U |  V |  W |  X

```
