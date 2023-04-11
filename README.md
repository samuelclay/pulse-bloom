# Pulse & Bloom

![Pulse & Bloom at Night](http://static.newsblur.com.s3.amazonaws.com/ofbrooklyn/Pulse%20%26%20Bloom%20-%20Night.jpg)

This was my second year attending Burning Man. Many use Burning Man as a week to detach from their workweek and experience a new life of intense leisure. Not me, I come to Burning Man to build.

Pulse & Bloom is a [2014 honorarium installation](http://www.burningman.com/installations/art_honor.html). The core team of 6 people — [Saba Ghole](https://twitter.com/sabarani), [Shilo Shiv Suleman](https://twitter.com/shilo1221), [Rohan Dixit](https://twitter.com/rd108), [Heather Stewart](https://www.facebook.com/heather.stewart.3388), [Luke Iseman](https://twitter.com/liseman), and [myself](http://twitter.com/samuelclay) — built 20 interactive lotus flowers made out of steel and rowlux. Each lotus flower ranges from 8 to 18 feet tall, each of which lights up with your pulse. You and another person can put your hands on a couple of Hamsa hands at the base of the lotus flower and your respective heartbeats will light up the flower.

We've gotten some great press coverage at [the BBC](http://www.bbc.com/news/in-pictures-29059374), [The Guardian](http://www.theguardian.com/artanddesign/gallery/2014/sep/06/photography-new-york-fashion-gaza-islamic-state?CMP=fb_gu), [The Atlantic's Big Picture](http://www.theatlantic.com/infocus/2014/09/burning-man-2014/100802/#img07) [twice](http://www.theatlantic.com/infocus/2014/09/burning-man-2014/100802/#img22), [CBS News](http://www.cbsnews.com/pictures/burning-man-2014/9/), [NBC News](http://www.nbcnews.com/pop-culture/pop-culture-news/desert-dwellers-burning-man-festival-full-swing-n192541#ember903), and [MSNBC](http://on.msnbc.com/1qkUN4c).

Read the entire writeup at http://www.ofbrooklyn.com/2014/09/6/building-pulse-bloom-biofeedback-burning-man-2014/.

## Setup and installation

Download the AVR GCC toolchain. You can use this homebrew-avr library to install on macOS: https://github.com/osx-cross/homebrew-avr

## Firmware

Here you can find the firmware used to program the main circuit board's ATmega328p chip, as well as designs for the laser cut cowl that sits over the pulse sensor's Si1143x IC. 

The easiest way to read the firmware is to dive into `src/pulse.cpp`. It has all of the pulse and rest states and handles the high level logic.

The state machine that runs the entire routine is determined by a couple of critical measurements. The program takes a measurement of both pulse sensors and determines whether a finger is covering either of them and whether or not that finger has an active pulse going.

    // Modes
    void determinePlayerMode();
    void determineFingerMode(int sensor1On, int sensor2On);
    void resetStem(PulsePlug *pulse);
    uint8_t adjustBpm(PulsePlug *pulse);
    
When there are no fingers on either of the pulse sensors, the program runs the rest state, which is a blue-topped lotus with a small blue snake running up and down the stem. The program is merely waiting for somebody to put their finger on a sensor.

    // State: resting
    void runResting();
    void runRestStem();
    void runRestStem(PulsePlug *pulse, int16_t currentLed);
    void clearStemLeds(PulsePlug *pulse);
    void beginPetalResting();
    bool runPetalResting();

Before a heartbeat is shown an accurate measurement of a heartbeat needs to be read. In order to determine whether a heartbeat is found and at what speed, a number of light detection measurements are taken and analyzed. I simply find the high frequency curve (the heartbeat) and the low frequency curve (the light sensors leveling out) and then use a simple peak and valley hueristic to determine the moment a heartbeat begins. This can take up to two or three seconds at first, so a bit of unwinding animation prepares you when you first put your finger on the sensor.

When a finger is detected, the stem snake lights shoot backwards up and down the lotus in prep for a heartbeat. This looks seamless as it clears the stem. This way a heartbeat doesn't interrupt the rest state.

    // State: end resting
    void beginSplittingStem();
    void runSplittingStem();
    void runSplittingStem(PulsePlug *pulse, int16_t currentLed);

When a heartbeat is detected, it is first shot up the stem. The color is determined by how many pulse sensors are being used. 1 heartbeat shows amber. 2 heartbeats show in white and light red.

    // State: stem rising
    bool runStemRising(PulsePlug *pulse, PulsePlug *shadowPulse);

Once the heartbeat reaches the top of the stem, the petals then fill with light.

    // State: petal rising
    void beginPetalRising();
    bool runPetalRising();

After a set delay, the petal slowly lose light while waiting for the next heartbeat.

    // State: petal falling
    void beginPetalFalling();
    bool runPetalFalling();

Lots of debugging messages have been left in the code and are accessible using the Serial pin on the board. Just turn on `USE_SERIAL`, which is commented out at top. Note that this increases binary code size by about 2.5K. I wrote a lot of logging.

    // Debugging
    void printHeader();
    void blink(int loops, int loopTime, bool half);
    int freeRam ();

## Contact 

I'm happy to [answer any questions on Twitter](https://twitter.com/samuelclay) or [over email](mailto:pulsebloom@ofbrooklyn.com).
