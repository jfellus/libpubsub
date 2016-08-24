# libpubsub
A generic library implementing a Publish/Subscribe communication model


# Performances

#### TCP transport

* localhost (port to port) : 
    * 1.13Go/s (1 subs), 2.05Go/s (2 subs), 2.65Go/s (3 subs), 2.79Go/s (4 subs) - bufsize = 1024
    * bufsize : 1500 -> 950Mo/s, 4096 -> 977Mo/s, 512 -> 1.32Go/s, 256 -> 1.69Go/s, 128 -> 10Mo/s, 64 -> 7Mo/s, 32 -> 2.9Mo/s, 16 -> 900Ko/s, 8 -> 600Ko/s, 4 -> 400Ko/s, 2 -> 165Ko/s, 1 -> 25Ko/s
