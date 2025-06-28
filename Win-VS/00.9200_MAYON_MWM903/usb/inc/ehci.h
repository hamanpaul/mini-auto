

#include <general.h>

#define USBMODE		    0x68		/* USB Device mode */
#define USBMODE_SDIS	(1 << 3)	/* Stream disable */
#define USBMODE_BE	    (1 << 2)	/* BE/LE endiannes select */
#define USBMODE_CM_HC	(3 << 0)	/* host controller mode */
#define USBMODE_CM_IDLE	(0 << 0)	/* idle state */

/* Low level init functions */
int ehci_hcd_init(void);
int ehci_hcd_stop(void);

