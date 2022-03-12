package tech.surdacki.base57;

import java.nio.charset.StandardCharsets;

public final class Constants {
    public static final int ENCODED_LONG_LENGTH = 11;
    public static final int ENCODED_LONGS_PER_LINE = 8;
    public static final int ENCODED_LINE_LENGTH
            = ENCODED_LONGS_PER_LINE * ENCODED_LONG_LENGTH + 1;
    public static final int DECODED_LINE_LENGTH = ENCODED_LONGS_PER_LINE * Long.BYTES;
    static final byte[] PLAIN_TO_ENCODED_LENGTH_MAPPING = {
        //  0, 1, 2, 3, 4, 5, 6,  7,  8
            0, 2, 3, 5, 6, 7, 9, 10, 11
    };
    static final byte[] ENCODED_TO_PLAIN_LENGTH_MAPPING = {
        //  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
            0, 1, 1, 2, 3, 3, 4, 5, 6, 6,  7,  8
    };
    static final byte[] SYMBOLS = (
              "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
            + "ZY23456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWX"
    ).getBytes(StandardCharsets.US_ASCII);
    static final byte DELIMITER_VALUE = 57;
    static final byte[] SYMBOL_VALUES = new byte[]{
            //         x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF
            //         \0,                         \a, \b, \t, \n, \v, \f, \r,
            /* 0x0x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 57, 57, 57, 57, 57, 99, 99, /* 0x0x */
            /* 0x1x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 57, 57, 57, 57, /* 0x1x */
            //           ,  !,  ",  #,  $,  %   &,  ',  (,  ),  *,  +,  ,,  -,  .,  /,
            /* 0x2x */ 57, 99, 99, 99, 99, 99, 99, 57, 99, 99, 99, 57, 57, 57, 57, 57, /* 0x2x */
            //          0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ?,
            /* 0x3x */ 99, 99, 2, 3, 4, 5, 6, 7, 8, 9, 57, 57, 99, 99, 99, 99, /* 0x3x */
            //          @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,
            /* 0x4x */ 99, 35, 36, 37, 38, 39, 40, 41, 42, 99, 43, 44, 45, 46, 47, 99, /* 0x4x */
            //          P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _,
            /* 0x5x */ 48, 49, 50, 51, 52, 53, 54, 55, 56, 1, 0, 99, 57, 99, 99, 57, /* 0x5x */
            //          `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o,
            /* 0x6x */ 57, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 99, 21, 22, 23, /* 0x6x */
            //          p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },  ~,
            /* 0x7x */ 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 99, 99, 99, 99, 99, /* 0x7x */
            //         x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, xA, xB, xC, xD, xE, xF
            /* 0x8x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0x8x */
            /* 0x9x */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0x9x */
            /* 0xAx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xAx */
            /* 0xBx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xBx */
            /* 0xCx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xCx */
            /* 0xDx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xDx */
            /* 0xEx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, /* 0xEx */
            /* 0xFx */ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99  /* 0xFx */
    };
    static final long[] MAGNITUDES = new long[] {
            1L,
            1L * 57,
            1L * 57 * 56,
            1L * 57 * 56 * 57,
            1L * 57 * 56 * 57 * 56,
            1L * 57 * 56 * 57 * 56 * 57,
            1L * 57 * 56 * 57 * 56 * 57 * 56,
            1L * 57 * 56 * 57 * 56 * 57 * 56 * 56,
            1L * 57 * 56 * 57 * 56 * 57 * 56 * 56 * 57,
            1L * 57 * 56 * 57 * 56 * 57 * 56 * 56 * 57 * 56,
            1L * 57 * 56 * 57 * 56 * 57 * 56 * 56 * 57 * 56 * 57,
        //   A    r    A    r    A    r    r    A    r    A    r
    };
}
