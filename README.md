#Intro
A gimmick tool/game I'm writing to 1. practice C 2. make something cool based on the terminal interactions with a Luciferian AI in [The Talos Principle](http://www.croteam.com/talosprinciple/)
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