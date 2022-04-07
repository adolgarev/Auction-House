HOW TO RUN

Open solution in VC2019, click Build, then Start. VC2019 comes with NuGet pre-installed and nuget.org source pre-configured out of the box. These are required to download the Boost library.

LIBRARIES

Boost.Asio is used for Async TCP server implementation. Under Windows, it uses IOCPs, under Linux epoll, thus can handle >10k simultaneous users. Also, it provides timers on the event loop.
Boost.Test is used for Unit Tests.
Easylogging++ is used for easy logging. Boost.Log is too complicated and requires additional libs. Easylogging++ is distributed as a source code and included in the project itself.
I've implemented my persistent storage with a write-ahead log just for the sake of the test task. In an actual project, I encourage you to use in such a scenario the Rocksdb.
I've implemented my client protocol parser - ChunkTokenizer - which parses incoming data stream into commands and protects from malicious input. For such a simple case, this will do. For more sophisticated cases, one should use LEX and YACC or Boost.Qi (EBNF and PEG parsers).

SOLUTION OVERVIEW

Required functionality and bonus items are implemented.
Sell and buy orders (Orderbook) are transient. Items on hold are ephemeral too. Inventory (items and funds) are persistent with ACID guarantees (D partly as I haven't added fsync, which also doesn't guarantee Durability, it is better to do a distributed database). This means in case of crash or restart, orders are cleared, and all items on hold are released, funds are safe.
One cannot buy immediately, only make a bid, then matching happens, and after expiration time order completes.
The choices above are based on my experience with EPEX exchange and Markedskraft.
There are not many comments in the code. This is done intentionally. I prefer to write a code to increase Cohesion and decrease Coupling. Classes do one thing, their names, method names, and argument names and types are self-explanatory (except in a few places where I've added comments). I'm ok with any other coding standard, though.

CLASSES

Auction implements the business logic of an Auction
Db, DbTransaction, InMemoryDb, FileBackedDB (uses FileBackedMap) are to organize in memory and persistent storage.
DummyTimer, AsyncTimer implement Timer interface and are used in UTs and in an event loop respectively.
AsyncTcpServer and AsyncTcpSession implement the communication with clients. AsyncTcpSession uses ChunkTokenizer to handle input.
And a few auxiliary classes.
