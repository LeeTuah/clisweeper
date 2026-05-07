# CLI Sweeper
---


![CLISweeper](assets/clisweeper.png)
CLI Sweeper is my attempt of making a multiplayer command-line version of the classic game, Minesweeper.

## Features
---
* Tested and works well on both linux and windows.
* Uses TCP Sockets to connect players in a server-client setup. (WIP)

## Screenshots
---
TBD.

## Project Structure
---
```
├── README.md
└── src
    ├── client
    │   ├── intro.cpp
    │   ├── main.cpp
    │   ├── minesweeper.cpp
    │   ├── multiplayer.cpp
    │   └── utils.cpp
    ├── include
    │   ├── conio_linux_port.h
    │   └── Menu.h
    └── server
        ├── main.cpp
        └── utils.cpp
```

## Installation
---

TBD.

## Usage
---

TBD.

## Libraries Used
---

* [conio for linux](https://github.com/zoelabbb/conio.h)
* [Effortless menus](https://github.com/LeeTuah/Effortless-Menus)

## Todo
---

1. Multiplayer support (co-op)
2. A timewatch