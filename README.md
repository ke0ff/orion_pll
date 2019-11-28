# orion_pll
Orion PLL Synthesizer application

SiLabs C8051 and Kiel (V5) Project for the C8051F530/ADF-4351. Configures the ADF-4351 via a SPI interface. Uses either a BCD logic interface to set a channel (00 - 99) or can accept serial commands (9600 baud) to set channel. Serial command line format is used to set channels and perform FLASH maintenance (programming and erasure). Requires channel data to be previously determined using either the ADF PLL application software, or a spreadsheet (available at the hardware project URL, below).

See http://www.rollanet.org/~joeh/projects/Orion/ for project hardware details.
