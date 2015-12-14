#Intro

##Description

A gimmick tool/game I'm writing to 1. practice C 2. make something cool based on the terminal interactions with a Luciferian AI in [The Talos Principle](http://www.croteam.com/talosprinciple/)

##Status

At the moment prints prompt, blinks, looks up stuff on Wikipedia and returns unparsed articles. Very early stages, really. You have to match an exact title (including capital letters) to get any output, and then it's a jumbled mess of raw WikiMarkup.

#Prep

##Dependencies
- Linux (might build on other POSIX-compliant systems but uses GNU extensions(I think only usleep() really needs those as of commit 656982a)),
- pthreads,
- libcurl,
- ncurses

#Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ./
make
```

#Run

```bash
./bin/mla
```