#Intro

##Description

A gimmick tool/game I'm writing to 1. practice C 2. make something cool based on the terminal interactions with a Luciferian AI in [The Talos Principle](http://www.croteam.com/talosprinciple/)

##Status

At the moment prints prompt, blinks, looks up stuff on Wikipedia and returns unparsed articles. Very early stages, really. You have to match an exact title (including capital letters) to get any output, and then it's a jumbled mess of raw WikiMarkup.
- UPDATE: Now it uses the official Wikipedia API, demangles the xml and extracts what has been experimentally proven to be the summary of the article (I guessed until it worked for most ~*mature~* articles). Still raw WikiMarkup tho.

#Prep

##Dependencies
- Linux (might build on other POSIX-compliant systems but uses GNU extensions(I think only usleep() really needs those as of commit 656982a) (I _definitely_ really need that exact function, despite there being others in the standard, and I'm not just doing that to use a GNU extension and promote free open sores fotwenny one love)),
- pthreads,
- libcurl,
- libxml2
- pcre2
- ncursesw (haha good luck with that)

#Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ./
make
```

#Run

```bash
./bin/mla
```