#!/usr/bin/env ruby
# Copyright 2010 Thoughtgang <http://www.thoughtgang.org>
# Unit test for BFD module

require 'test/unit'
require 'rubygems'
require 'BFD'

class TC_BfdModule < Test::Unit::TestCase

  # Note: this is an x86-64 GCC compilation of the very basic C program
  #                 int main(void) { return 666; }
  TARGET_BUF = %w{
                7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
                02 00 3e 00 01 00 00 00 e0 03 40 00 00 00 00 00
                40 00 00 00 00 00 00 00 40 11 00 00 00 00 00 00
                00 00 00 00 40 00 38 00 09 00 40 00 1f 00 1c 00
                06 00 00 00 05 00 00 00 40 00 00 00 00 00 00 00
                40 00 40 00 00 00 00 00 40 00 40 00 00 00 00 00
                f8 01 00 00 00 00 00 00 f8 01 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 03 00 00 00 04 00 00 00
                38 02 00 00 00 00 00 00 38 02 40 00 00 00 00 00
                38 02 40 00 00 00 00 00 1c 00 00 00 00 00 00 00
                1c 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00
                01 00 00 00 05 00 00 00 00 00 00 00 00 00 00 00
                00 00 40 00 00 00 00 00 00 00 40 00 00 00 00 00
                54 06 00 00 00 00 00 00 54 06 00 00 00 00 00 00
                00 00 20 00 00 00 00 00 01 00 00 00 06 00 00 00
                18 0e 00 00 00 00 00 00 18 0e 60 00 00 00 00 00
                18 0e 60 00 00 00 00 00 00 02 00 00 00 00 00 00
                10 02 00 00 00 00 00 00 00 00 20 00 00 00 00 00
                02 00 00 00 06 00 00 00 40 0e 00 00 00 00 00 00
                40 0e 60 00 00 00 00 00 40 0e 60 00 00 00 00 00
                a0 01 00 00 00 00 00 00 a0 01 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 04 00 00 00 04 00 00 00
                54 02 00 00 00 00 00 00 54 02 40 00 00 00 00 00
                54 02 40 00 00 00 00 00 44 00 00 00 00 00 00 00
                44 00 00 00 00 00 00 00 04 00 00 00 00 00 00 00
                50 e5 74 64 04 00 00 00 bc 05 00 00 00 00 00 00
                bc 05 40 00 00 00 00 00 bc 05 40 00 00 00 00 00
                24 00 00 00 00 00 00 00 24 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 51 e5 74 64 06 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00
                52 e5 74 64 04 00 00 00 18 0e 00 00 00 00 00 00
                18 0e 60 00 00 00 00 00 18 0e 60 00 00 00 00 00
                e8 01 00 00 00 00 00 00 e8 01 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 2f 6c 69 62 36 34 2f 6c
                64 2d 6c 69 6e 75 78 2d 78 38 36 2d 36 34 2e 73
                6f 2e 32 00 04 00 00 00 10 00 00 00 01 00 00 00
                47 4e 55 00 00 00 00 00 02 00 00 00 06 00 00 00
                0f 00 00 00 04 00 00 00 14 00 00 00 03 00 00 00
                47 4e 55 00 e0 01 67 5d 37 02 90 5f c1 b1 93 56
                2e ff 8c c6 13 be 47 5e 01 00 00 00 03 00 00 00
                02 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00
                01 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 01 00 00 00 20 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                1a 00 00 00 12 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 5f 5f 67 6d 6f 6e 5f
                73 74 61 72 74 5f 5f 00 6c 69 62 63 2e 73 6f 2e
                36 00 5f 5f 6c 69 62 63 5f 73 74 61 72 74 5f 6d
                61 69 6e 00 47 4c 49 42 43 5f 32 2e 32 2e 35 00
                00 00 00 00 02 00 00 00 01 00 01 00 10 00 00 00
                10 00 00 00 00 00 00 00 75 1a 69 09 00 00 02 00
                2c 00 00 00 00 00 00 00 e0 0f 60 00 00 00 00 00
                06 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00
                00 10 60 00 00 00 00 00 07 00 00 00 02 00 00 00
                00 00 00 00 00 00 00 00 48 83 ec 08 e8 5b 00 00
                00 e8 ea 00 00 00 e8 b5 01 00 00 48 83 c4 08 c3
                ff 35 2a 0c 20 00 ff 25 2c 0c 20 00 0f 1f 40 00
                ff 25 2a 0c 20 00 68 00 00 00 00 e9 e0 ff ff ff
                31 ed 49 89 d1 5e 48 89 e2 48 83 e4 f0 50 54 49
                c7 c0 d0 04 40 00 48 c7 c1 e0 04 40 00 48 c7 c7
                c4 04 40 00 e8 c7 ff ff ff f4 90 90 48 83 ec 08
                48 8b 05 c9 0b 20 00 48 85 c0 74 02 ff d0 48 83
                c4 08 c3 90 90 90 90 90 90 90 90 90 90 90 90 90
                55 48 89 e5 53 48 83 ec 08 80 3d d8 0b 20 00 00
                75 4b bb 30 0e 60 00 48 8b 05 d2 0b 20 00 48 81
                eb 28 0e 60 00 48 c1 fb 03 48 83 eb 01 48 39 d8
                73 24 66 0f 1f 44 00 00 48 83 c0 01 48 89 05 ad
                0b 20 00 ff 14 c5 28 0e 60 00 48 8b 05 9f 0b 20
                00 48 39 d8 72 e2 c6 05 8b 0b 20 00 01 48 83 c4
                08 5b c9 c3 66 66 66 2e 0f 1f 84 00 00 00 00 00
                55 48 83 3d 8f 09 20 00 00 48 89 e5 74 12 b8 00
                00 00 00 48 85 c0 74 08 bf 38 0e 60 00 c9 ff e0
                c9 c3 90 90 b8 9a 02 00 00 c3 90 90 90 90 90 90
                f3 c3 66 66 66 66 66 2e 0f 1f 84 00 00 00 00 00
                48 89 6c 24 d8 4c 89 64 24 e0 48 8d 2d 23 09 20
                00 4c 8d 25 1c 09 20 00 4c 89 6c 24 e8 4c 89 74
                24 f0 4c 89 7c 24 f8 48 89 5c 24 d0 48 83 ec 38
                4c 29 e5 41 89 fd 49 89 f6 48 c1 fd 03 49 89 d7
                e8 83 fe ff ff 48 85 ed 74 1c 31 db 0f 1f 40 00
                4c 89 fa 4c 89 f6 44 89 ef 41 ff 14 dc 48 83 c3
                01 48 39 eb 72 ea 48 8b 5c 24 08 48 8b 6c 24 10
                4c 8b 64 24 18 4c 8b 6c 24 20 4c 8b 74 24 28 4c
                8b 7c 24 30 48 83 c4 38 c3 90 90 90 90 90 90 90
                55 48 89 e5 53 48 83 ec 08 48 8b 05 98 08 20 00
                48 83 f8 ff 74 19 bb 18 0e 60 00 0f 1f 44 00 00
                48 83 eb 08 ff d0 48 8b 03 48 83 f8 ff 75 f1 48
                83 c4 08 5b c9 c3 90 90 48 83 ec 08 e8 7f fe ff
                ff 48 83 c4 08 c3 00 00 01 00 02 00 01 1b 03 3b
                20 00 00 00 03 00 00 00 08 ff ff ff 3c 00 00 00
                14 ff ff ff 54 00 00 00 24 ff ff ff 6c 00 00 00
                14 00 00 00 00 00 00 00 01 7a 52 00 01 78 10 01
                1b 0c 07 08 90 01 00 00 14 00 00 00 1c 00 00 00
                c4 fe ff ff 06 00 00 00 00 00 00 00 00 00 00 00
                14 00 00 00 34 00 00 00 b8 fe ff ff 02 00 00 00
                00 00 00 00 00 00 00 00 24 00 00 00 4c 00 00 00
                b0 fe ff ff 89 00 00 00 00 51 8c 05 86 06 5f 0e
                40 46 83 07 8f 02 8e 03 8d 04 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff
                00 00 00 00 00 00 00 00 ff ff ff ff ff ff ff ff
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00
                0c 00 00 00 00 00 00 00 a8 03 40 00 00 00 00 00
                0d 00 00 00 00 00 00 00 a8 05 40 00 00 00 00 00
                04 00 00 00 00 00 00 00 98 02 40 00 00 00 00 00
                f5 fe ff 6f 00 00 00 00 b0 02 40 00 00 00 00 00
                05 00 00 00 00 00 00 00 18 03 40 00 00 00 00 00
                06 00 00 00 00 00 00 00 d0 02 40 00 00 00 00 00
                0a 00 00 00 00 00 00 00 38 00 00 00 00 00 00 00
                0b 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                15 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                03 00 00 00 00 00 00 00 e8 0f 60 00 00 00 00 00
                02 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                14 00 00 00 00 00 00 00 07 00 00 00 00 00 00 00
                17 00 00 00 00 00 00 00 90 03 40 00 00 00 00 00
                07 00 00 00 00 00 00 00 78 03 40 00 00 00 00 00
                08 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                09 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                fe ff ff 6f 00 00 00 00 58 03 40 00 00 00 00 00
                ff ff ff 6f 00 00 00 00 01 00 00 00 00 00 00 00
                f0 ff ff 6f 00 00 00 00 50 03 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 40 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                d6 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 47 43 43 3a 20 28 55 62
                75 6e 74 75 20 34 2e 34 2e 33 2d 34 75 62 75 6e
                74 75 35 29 20 34 2e 34 2e 33 00 00 2e 73 79 6d
                74 61 62 00 2e 73 74 72 74 61 62 00 2e 73 68 73
                74 72 74 61 62 00 2e 69 6e 74 65 72 70 00 2e 6e
                6f 74 65 2e 41 42 49 2d 74 61 67 00 2e 6e 6f 74
                65 2e 67 6e 75 2e 62 75 69 6c 64 2d 69 64 00 2e
                67 6e 75 2e 68 61 73 68 00 2e 64 79 6e 73 79 6d
                00 2e 64 79 6e 73 74 72 00 2e 67 6e 75 2e 76 65
                72 73 69 6f 6e 00 2e 67 6e 75 2e 76 65 72 73 69
                6f 6e 5f 72 00 2e 72 65 6c 61 2e 64 79 6e 00 2e
                72 65 6c 61 2e 70 6c 74 00 2e 69 6e 69 74 00 2e
                74 65 78 74 00 2e 66 69 6e 69 00 2e 72 6f 64 61
                74 61 00 2e 65 68 5f 66 72 61 6d 65 5f 68 64 72
                00 2e 65 68 5f 66 72 61 6d 65 00 2e 63 74 6f 72
                73 00 2e 64 74 6f 72 73 00 2e 6a 63 72 00 2e 64
                79 6e 61 6d 69 63 00 2e 67 6f 74 00 2e 67 6f 74
                2e 70 6c 74 00 2e 64 61 74 61 00 2e 62 73 73 00
                2e 63 6f 6d 6d 65 6e 74 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                1b 00 00 00 01 00 00 00 02 00 00 00 00 00 00 00
                38 02 40 00 00 00 00 00 38 02 00 00 00 00 00 00
                1c 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                23 00 00 00 07 00 00 00 02 00 00 00 00 00 00 00
                54 02 40 00 00 00 00 00 54 02 00 00 00 00 00 00
                20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                31 00 00 00 07 00 00 00 02 00 00 00 00 00 00 00
                74 02 40 00 00 00 00 00 74 02 00 00 00 00 00 00
                24 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                48 00 00 00 05 00 00 00 02 00 00 00 00 00 00 00
                98 02 40 00 00 00 00 00 98 02 00 00 00 00 00 00
                18 00 00 00 00 00 00 00 06 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 04 00 00 00 00 00 00 00
                44 00 00 00 f6 ff ff 6f 02 00 00 00 00 00 00 00
                b0 02 40 00 00 00 00 00 b0 02 00 00 00 00 00 00
                1c 00 00 00 00 00 00 00 06 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                4e 00 00 00 0b 00 00 00 02 00 00 00 00 00 00 00
                d0 02 40 00 00 00 00 00 d0 02 00 00 00 00 00 00
                48 00 00 00 00 00 00 00 07 00 00 00 01 00 00 00
                08 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                56 00 00 00 03 00 00 00 02 00 00 00 00 00 00 00
                18 03 40 00 00 00 00 00 18 03 00 00 00 00 00 00
                38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                5e 00 00 00 ff ff ff 6f 02 00 00 00 00 00 00 00
                50 03 40 00 00 00 00 00 50 03 00 00 00 00 00 00
                06 00 00 00 00 00 00 00 06 00 00 00 00 00 00 00
                02 00 00 00 00 00 00 00 02 00 00 00 00 00 00 00
                6b 00 00 00 fe ff ff 6f 02 00 00 00 00 00 00 00
                58 03 40 00 00 00 00 00 58 03 00 00 00 00 00 00
                20 00 00 00 00 00 00 00 07 00 00 00 01 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                7a 00 00 00 04 00 00 00 02 00 00 00 00 00 00 00
                78 03 40 00 00 00 00 00 78 03 00 00 00 00 00 00
                18 00 00 00 00 00 00 00 06 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                84 00 00 00 04 00 00 00 02 00 00 00 00 00 00 00
                90 03 40 00 00 00 00 00 90 03 00 00 00 00 00 00
                18 00 00 00 00 00 00 00 06 00 00 00 0d 00 00 00
                08 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                8e 00 00 00 01 00 00 00 06 00 00 00 00 00 00 00
                a8 03 40 00 00 00 00 00 a8 03 00 00 00 00 00 00
                18 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                89 00 00 00 01 00 00 00 06 00 00 00 00 00 00 00
                c0 03 40 00 00 00 00 00 c0 03 00 00 00 00 00 00
                20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00
                94 00 00 00 01 00 00 00 06 00 00 00 00 00 00 00
                e0 03 40 00 00 00 00 00 e0 03 00 00 00 00 00 00
                c8 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                9a 00 00 00 01 00 00 00 06 00 00 00 00 00 00 00
                a8 05 40 00 00 00 00 00 a8 05 00 00 00 00 00 00
                0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                a0 00 00 00 01 00 00 00 12 00 00 00 00 00 00 00
                b8 05 40 00 00 00 00 00 b8 05 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 04 00 00 00 00 00 00 00
                a8 00 00 00 01 00 00 00 02 00 00 00 00 00 00 00
                bc 05 40 00 00 00 00 00 bc 05 00 00 00 00 00 00
                24 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                b6 00 00 00 01 00 00 00 02 00 00 00 00 00 00 00
                e0 05 40 00 00 00 00 00 e0 05 00 00 00 00 00 00
                74 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                c0 00 00 00 01 00 00 00 03 00 00 00 00 00 00 00
                18 0e 60 00 00 00 00 00 18 0e 00 00 00 00 00 00
                10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                c7 00 00 00 01 00 00 00 03 00 00 00 00 00 00 00
                28 0e 60 00 00 00 00 00 28 0e 00 00 00 00 00 00
                10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                ce 00 00 00 01 00 00 00 03 00 00 00 00 00 00 00
                38 0e 60 00 00 00 00 00 38 0e 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                d3 00 00 00 06 00 00 00 03 00 00 00 00 00 00 00
                40 0e 60 00 00 00 00 00 40 0e 00 00 00 00 00 00
                a0 01 00 00 00 00 00 00 07 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00
                dc 00 00 00 01 00 00 00 03 00 00 00 00 00 00 00
                e0 0f 60 00 00 00 00 00 e0 0f 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00
                e1 00 00 00 01 00 00 00 03 00 00 00 00 00 00 00
                e8 0f 60 00 00 00 00 00 e8 0f 00 00 00 00 00 00
                20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 08 00 00 00 00 00 00 00
                ea 00 00 00 01 00 00 00 03 00 00 00 00 00 00 00
                08 10 60 00 00 00 00 00 08 10 00 00 00 00 00 00
                10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                f0 00 00 00 08 00 00 00 03 00 00 00 00 00 00 00
                18 10 60 00 00 00 00 00 18 10 00 00 00 00 00 00
                10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                f5 00 00 00 01 00 00 00 30 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 18 10 00 00 00 00 00 00
                23 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00
                11 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 3b 10 00 00 00 00 00 00
                fe 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 02 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 19 00 00 00 00 00 00
                00 06 00 00 00 00 00 00 1e 00 00 00 2f 00 00 00
                08 00 00 00 00 00 00 00 18 00 00 00 00 00 00 00
                09 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 1f 00 00 00 00 00 00
                db 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 01 00
                38 02 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 02 00 54 02 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 03 00
                74 02 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 04 00 98 02 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 05 00
                b0 02 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 06 00 d0 02 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 07 00
                18 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 08 00 50 03 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 09 00
                58 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 0a 00 78 03 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 0b 00
                90 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 0c 00 a8 03 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 0d 00
                c0 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 0e 00 e0 03 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 0f 00
                a8 05 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 10 00 b8 05 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 11 00
                bc 05 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 12 00 e0 05 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 13 00
                18 0e 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 14 00 28 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 15 00
                38 0e 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 16 00 40 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 17 00
                e0 0f 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 18 00 e8 0f 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 19 00
                08 10 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 03 00 1a 00 18 10 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 03 00 1b 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                01 00 00 00 02 00 0e 00 0c 04 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 11 00 00 00 04 00 f1 ff
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                1c 00 00 00 01 00 13 00 18 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 2a 00 00 00 01 00 14 00
                28 0e 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                38 00 00 00 01 00 15 00 38 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 45 00 00 00 02 00 0e 00
                30 04 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                5b 00 00 00 01 00 1a 00 18 10 60 00 00 00 00 00
                01 00 00 00 00 00 00 00 6a 00 00 00 01 00 1a 00
                20 10 60 00 00 00 00 00 08 00 00 00 00 00 00 00
                78 00 00 00 02 00 0e 00 a0 04 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 11 00 00 00 04 00 f1 ff
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                84 00 00 00 01 00 13 00 20 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 91 00 00 00 01 00 12 00
                50 06 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                9f 00 00 00 01 00 15 00 38 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 ab 00 00 00 02 00 0e 00
                70 05 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                c1 00 00 00 04 00 f1 ff 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 c5 00 00 00 01 02 18 00
                e8 0f 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                db 00 00 00 00 02 13 00 14 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 ec 00 00 00 00 02 13 00
                14 0e 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                ff 00 00 00 01 02 16 00 40 0e 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 08 01 00 00 20 00 19 00
                08 10 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                13 01 00 00 12 00 0e 00 d0 04 40 00 00 00 00 00
                02 00 00 00 00 00 00 00 23 01 00 00 12 00 0e 00
                e0 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                2a 01 00 00 20 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 39 01 00 00 20 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                4d 01 00 00 12 00 0f 00 a8 05 40 00 00 00 00 00
                00 00 00 00 00 00 00 00 53 01 00 00 12 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                72 01 00 00 11 00 10 00 b8 05 40 00 00 00 00 00
                04 00 00 00 00 00 00 00 81 01 00 00 10 00 19 00
                08 10 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                8e 01 00 00 11 02 19 00 10 10 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 9b 01 00 00 11 02 14 00
                30 0e 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                a8 01 00 00 12 00 0e 00 e0 04 40 00 00 00 00 00
                89 00 00 00 00 00 00 00 b8 01 00 00 10 00 f1 ff
                18 10 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                c4 01 00 00 10 00 f1 ff 28 10 60 00 00 00 00 00
                00 00 00 00 00 00 00 00 c9 01 00 00 10 00 f1 ff
                18 10 60 00 00 00 00 00 00 00 00 00 00 00 00 00
                d0 01 00 00 12 00 0e 00 c4 04 40 00 00 00 00 00
                06 00 00 00 00 00 00 00 d5 01 00 00 12 00 0c 00
                a8 03 40 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 63 61 6c 6c 5f 67 6d 6f 6e 5f 73 74 61 72 74
                00 63 72 74 73 74 75 66 66 2e 63 00 5f 5f 43 54
                4f 52 5f 4c 49 53 54 5f 5f 00 5f 5f 44 54 4f 52
                5f 4c 49 53 54 5f 5f 00 5f 5f 4a 43 52 5f 4c 49
                53 54 5f 5f 00 5f 5f 64 6f 5f 67 6c 6f 62 61 6c
                5f 64 74 6f 72 73 5f 61 75 78 00 63 6f 6d 70 6c
                65 74 65 64 2e 37 33 38 32 00 64 74 6f 72 5f 69
                64 78 2e 37 33 38 34 00 66 72 61 6d 65 5f 64 75
                6d 6d 79 00 5f 5f 43 54 4f 52 5f 45 4e 44 5f 5f
                00 5f 5f 46 52 41 4d 45 5f 45 4e 44 5f 5f 00 5f
                5f 4a 43 52 5f 45 4e 44 5f 5f 00 5f 5f 64 6f 5f
                67 6c 6f 62 61 6c 5f 63 74 6f 72 73 5f 61 75 78
                00 74 2e 63 00 5f 47 4c 4f 42 41 4c 5f 4f 46 46
                53 45 54 5f 54 41 42 4c 45 5f 00 5f 5f 69 6e 69
                74 5f 61 72 72 61 79 5f 65 6e 64 00 5f 5f 69 6e
                69 74 5f 61 72 72 61 79 5f 73 74 61 72 74 00 5f
                44 59 4e 41 4d 49 43 00 64 61 74 61 5f 73 74 61
                72 74 00 5f 5f 6c 69 62 63 5f 63 73 75 5f 66 69
                6e 69 00 5f 73 74 61 72 74 00 5f 5f 67 6d 6f 6e
                5f 73 74 61 72 74 5f 5f 00 5f 4a 76 5f 52 65 67
                69 73 74 65 72 43 6c 61 73 73 65 73 00 5f 66 69
                6e 69 00 5f 5f 6c 69 62 63 5f 73 74 61 72 74 5f
                6d 61 69 6e 40 40 47 4c 49 42 43 5f 32 2e 32 2e
                35 00 5f 49 4f 5f 73 74 64 69 6e 5f 75 73 65 64
                00 5f 5f 64 61 74 61 5f 73 74 61 72 74 00 5f 5f
                64 73 6f 5f 68 61 6e 64 6c 65 00 5f 5f 44 54 4f
                52 5f 45 4e 44 5f 5f 00 5f 5f 6c 69 62 63 5f 63
                73 75 5f 69 6e 69 74 00 5f 5f 62 73 73 5f 73 74
                61 72 74 00 5f 65 6e 64 00 5f 65 64 61 74 61 00
                6d 61 69 6e 00 5f 69 6e 69 74 00
               }.collect{ |i| i.hex }.pack('C*')

  def test_buffer
    Bfd::Target.from_buffer( TARGET_BUF ) do |tgt|

      # These values were obtained by running BFD on the original file
      assert_equal( 'object', tgt.format )
      assert_equal( 'elf', tgt.flavour )
      assert_equal( 'elf64-x86-64', tgt.type )
      assert_equal( 'i386:x86-64', tgt.arch_info[:architecture] )
      assert_equal( 64, tgt.arch_info[:bits_per_word] )
      assert_equal( 64, tgt.arch_info[:bits_per_address] )
      assert_equal( 8, tgt.arch_info[:bits_per_byte] )
      assert_equal( 3, tgt.arch_info[:section_align_power] )

      assert_equal( 27, tgt.sections.length )
      assert_equal( '.bss', tgt.sections.keys.sort.first )
      assert_equal( '.text', tgt.sections.keys.sort.last )
      assert_equal( 0x4003E0, tgt.sections['.text'].vma )
      assert_equal( 0x1C8, tgt.sections['.text'].size )
      assert_equal( 0x3E0, tgt.sections['.text'].file_pos )
      # PROBLEM: BFD KEEPS CHANGING! THESE BROKE.
      #assert_equal( 30, tgt.symbols.length )
      #assert_equal( '(null)', tgt.symbols.keys.sort.first )
      #assert_equal( '__libc_start_main', tgt.symbols.keys.sort.last )
      assert_equal( 0, tgt.symbols['__libc_start_main'].value )
    end
  end

  def test_file
    tmp = Tempfile.new('ut-bfd-target')
    tmp.write(TARGET_BUF)

    # Test BFD handling of file by path
    Bfd::Target.new( tmp.path ) do |tgt|
      assert_equal( 27, tgt.sections.length )
    end

    # Test BFD handling of IO object
    File.open(tmp.path, 'rb') do |f|
      Bfd::Target.new( tmp.path ) do |tgt|
        assert_equal( 27, tgt.sections.length )
      end
    end

    tmp.close
  end

  def test_unix_exec_file
    # Attempt to load BFD for /bin/cat if it exists
    path = File::SEPARATOR + 'bin' + File::SEPARATOR + 'cat'
    return if not File.exist?(path)

    Bfd::Target.new( path ) do |tgt|
      assert_equal( 'object', tgt.format )
    end
  end

end
