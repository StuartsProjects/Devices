### HAB Tracker 2 RFM9x

This is a board intended to be used as a low component count high altitude balloon tracker. The board uses a Hope RFM9x LoRa module and an Arduino Pro Mini (must be the 3.3V 8Mhz version) and a Ublox MAXM8Q GPS. 

This is a board only, the RFM9x, Arduino Pro Mini and Ublox MAXM8Q GPS. are not supplied. The 2mm pin headers for fitting the RFM9x are supplied. 

The completed tracker will transmits its location using the GPS co-ordinates and will be received over hundreds of kilometres when at altitudes above 2km or so. The tracker sends the tracker payload as FSK RTTY as well as LoRa so can be received with standard amateur radio equipment or a PC and SDR combination. The tracker also puts out a very long range location only packet, which is useful for searching for the landed balloon. 

When built following the instructions 'HAB2 Minimum Parts Tracker Build.pdf' the tracker when powered by two AAA lithium energizer batteries has an all up weight of circa 20g and is thus suitable for use with low cost foil party balloons filled with helium. 

There is sample balloon tracker software in the \examples folder of the at the documentation link. Other tracker software can be found at the SX12xx-LoRa library here; [https://github.com/StuartsProjects/SX12XX-LoRa](https://github.com/StuartsProjects/SX12XX-LoRa)

The original code and instructions for the tracker PCB using a previous LoRa library is to be found here;

[https://github.com/StuartsProjects/HAB2](https://github.com/StuartsProjects/HAB2 "https://github.com/StuartsProjects/HAB2")
