CaLib
=====

CaLib calibration database system

Installation
------------

### Dependencies
* ROOT 5.34 (with MySQL support)
* ncurses
* MySQL database server

### Installation
* Compile the software using `make clean ; make`

Configuration
-------------

All the configuration is done in $CALIB/config/config.cfg.  
$CALIB/config/example.cfg contains comments and basic settings that should
help to understand the configuration. Rename this example file to config.cfg
and modify it according to your setup.

Documentation
-------------

The ROOT html documentation in $CALIB/htmldoc gives an overview of the 
CaLib library and its classes.
Further information and examples can be found in $CALIB/macros.

Changelog
---------

### 0.1.11
August 16, 2013
* added support for Mk2 format and xz-compressed files
* bugfixes

### 0.1.10
April 26, 2012
* several bugfixes

### 0.1.9
November 14, 2011
* added calibration cloning function
* bugfixes

### 0.1.8
October 10, 2011
* added TAPS CFD and veto LED to AcquRoot config file writer
* bugfixes

### 0.1.7
September 11, 2011
* updated PID energy calibration

### 0.1.6
August 31, 2011
* added TAPS CFD and veto LED calibration

### 0.1.5
August 24, 2011
* initial public release

