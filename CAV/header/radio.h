#ifndef APP_SAMPLE_RADIO_H_
#define APP_SAMPLE_RADIO_H_

#include <tk/tkernel.h>

#define RADIO_BASE 0x40001000

#define RADIO(p) (RADIO_BASE + p)

#define TASKS_TXEN 0x000        // Enable RADIO in TX mode
#define TASKS_RXEN 0x004        // Enable RADIO in RX mode
#define TASKS_START 0x008       // Start RADIO
#define TASKS_STOP 0x00C        // Stop RADIO
#define TASKS_DISABLE 0x010     // Disable RADIO
#define TASKS_RSSISTART 0x014   // Start the RSSI and take one single sample of the receive signal strength
#define TASKS_RSSISTOP 0x018    // Stop the RSSI measurement
#define TASKS_BCSTART 0x01C     // Start the bit counter
#define TASKS_BCSTOP 0x020      // Stop the bit counter
#define TASKS_EDSTART 0x024     // Start the energy detect measurement used in IEEE 802.15.4 mode
#define TASKS_EDSTOP 0x028      // Stop the energy detect measurement
#define TASKS_CCASTART 0x02C    // Start the clear channel assessment used in IEEE 802.15.4 mode
#define TASKS_CCASTOP 0x030     // Stop the clear channel assessment
#define EVENTS_READY 0x100      // RADIO has ramped up and is ready to be started
#define EVENTS_ADDRESS 0x104    // Address sent or received
#define EVENTS_PAYLOAD 0x108    // Packet payload sent or received
#define EVENTS_END 0x10C        // Packet sent or received
#define EVENTS_DISABLED 0x110   // RADIO has been disabled
#define EVENTS_DEVMATCH 0x114   // A device address match occurred on the last received packet
#define EVENTS_DEVMISS 0x118    // No device address match occurred on the last received packet
#define EVENTS_RSSIEND 0x11C    // Sampling of receive signal strength complete
#define EVENTS_BCMATCH 0x128    // Bit counter reached bit count value
#define EVENTS_CRCOK 0x130      // Packet received with CRC ok
#define EVENTS_CRCERROR 0x134   // Packet received with CRC error
#define EVENTS_FRAMESTART 0x138 // IEEE 802.15.4 length field received
#define EVENTS_EDEND 0x13C      // Sampling of energy detection complete. A new ED sample is ready for readout from the RADIO.EDSAMPLE register.
#define EVENTS_EDSTOPPED 0x140  // The sampling of energy detection has stopped
#define EVENTS_CCAIDLE 0x144    // Wireless medium in idle - clear to send
#define EVENTS_CCABUSY 0x148    // Wireless medium busy - do not send
#define EVENTS_CCASTOPPED 0x14C // The CCA has stopped
#define EVENTS_RATEBOOST 0x150  // Ble_LR CI field received, receive mode is changed from Ble_LR125Kbit to Ble_LR500Kbit.
#define EVENTS_TXREADY 0x154    // RADIO has ramped up and is ready to be started TX path
#define EVENTS_RXREADY 0x158    // RADIO has ramped up and is ready to be started RX path
#define EVENTS_MHRMATCH 0x15C   // MAC header match found
#define EVENTS_SYNC 0x168       // Preamble indicator
#define EVENTS_PHYEND 0x16C     // Generated when last bit is sent on air, or received from air
#define EVENTS_CTEPRESENT 0x170 // CTE is present (early warning right after receiving CTEInfo byte)
#define SHORTS 0x200            // Shortcuts between local events and tasks
#define INTENSET 0x304          // Enable interrupt
#define INTENCLR 0x308          // Disable interrupt
#define CRCSTATUS 0x400         // CRC status
#define RXMATCH 0x408           // Received address
#define RXCRC 0x40C             // CRC field of previously received packet
#define DAI 0x410               // Device address match index
#define PDUSTAT 0x414           // Payload status
#define CTESTATUS 0x44C         // CTEInfo parsed from received packet
#define DFESTATUS 0x458         // DFE status information
#define PACKETPTR 0x504         // Packet pointer
#define FREQUENCY 0x508         // Frequency
#define TXPOWER 0x50C           // Output power
#define MODE 0x510              // Data rate and modulation
#define PCNF0 0x514             // Packet configuration register 0
#define PCNF1 0x518             // Packet configuration register 1
#define BASE0 0x51C             // Base address 0
#define BASE1 0x520             // Base address 1
#define PREFIX0 0x524           // Prefixes bytes for logical addresses 0-3
#define PREFIX1 0x528           // Prefixes bytes for logical addresses 4-7
#define TXADDRESS 0x52C         // Transmit address select
#define RXADDRESSES 0x530       // Receive address select
#define CRCCNF 0x534            // CRC configuration
#define CRCPOLY 0x538           // CRC polynomial
#define CRCINIT 0x53C           // CRC initial value
#define TIFS 0x544              // Interframe spacing in Âµs
#define RSSISAMPLE 0x548        // RSSI sample
#define STATE 0x550             // Current radio state
#define DATAWHITEIV 0x554       // Data whitening initial value
#define BCC 0x560               // Bit counter compare AB[n] 0x600 Device address base segment n DAP[n] 0x620 Device address prefix n
#define DACNF 0x640             //Device address match configuration
#define MHRMATCHCONF 0x644      // Search pattern configuration
#define MHRMATCHMAS 0x648       // Pattern mask
#define MODECNF0 0x650          // Radio mode configuration register 0
#define SFD 0x660               // IEEE 802.15.4 start of frame delimiter
#define EDCNT 0x664             // IEEE 802.15.4 energy detect loop count
#define EDSAMPLE 0x668          // IEEE 802.15.4 energy detect level
#define CCACTRL 0x66C           // IEEE 802.15.4 clear channel assessment control
#define DFEMODE 0x900           // Whether to use Angle-of-Arrival (AOA) or Angle-of-Departure (AOD)
#define CTEINLINECONF 0x904     // Configuration for CTE inline mode
#define DFECTRL1 0x910          // Various configuration for Direction finding
#define DFECTRL2 0x914          // Start offset for Direction finding
#define SWITCHPATTERN 0x928     // GPIO patterns to be used for each antenna

EXPORT void print_packet();
EXPORT void radio_setup();

#endif /* APP_SAMPLE_RADIO_H_ */
