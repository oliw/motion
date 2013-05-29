# Motion Stabilisation of Robotic Assisted Heart Surgery

Part of my final year studies at the Department of Computing and Imperial College London. 

-----------------

## Overview
This is in an investigation into particular motion stabilisation techniques and their efficiancy when applied to video-guided keyhole heart surgery situations. For much greater detail please see the paper to be presented late June.

## Languages/Tools Used
This project was written in C++ and takes advantage of the following libraries.

*   QT
*   Matlab
*   OpenCV
*   CoinLP Solver

## Special Thanks
Thanks to Professor Daniel Rueckert: http://www.doc.ic.ac.uk/~dr/
Thanks to Matthias Grundmann http://www.cc.gatech.edu/cpl/projects/videostabilization/ (whose work inspired this undergraduate investigation)

## Notes
This will *not* be readily usable in its current state. It has been configured for my Labs and for my personal computer. When the project is finished I will investigate tidying things up so that it can be easily installed elsewhere.

There is a BUG in OpenCV 2.3 whereby not all frames are read in. Use 2.4 and above.

This project requires QT5.

## Useful Tools
There is also a PERL script for batch processing which cycles through all possible parameters (defined in the script).
