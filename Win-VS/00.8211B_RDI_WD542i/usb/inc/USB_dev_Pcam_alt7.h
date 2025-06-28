        /* if0Alt0 */
        {
            USB_DESC_LEN_INTERFACE,		/* Size of this descriptor */
            USB_DESC_TYPE_INTERFACE,	/* Interface descriptor type */
            0,	/* Interface number */
            7,	/* Alternate setting */
            7,		/* Number of endpoints */
            0xFF,	/* Class code - mass storage class */
            0xFF,	/* Subclass code */
            0,	/* Protocol code */
            0,		/* Index of string descriptor of this interface */
        },
        /* if0Alt0Ep1 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	    /* Endpoint descriptor type  */
            0x81,	/* Endpoint address */
            0x01,	/* Endpoint attributes */
            900,	/* Max. packet size */
            1,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep2 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	/* Endpoint descriptor type  */
            0x82,	/* Endpoint address */
            0x02,	/* Endpoint attributes */
            0x40,	/* Max. packet size */
            0,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep3 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	/* Endpoint descriptor type  */
            0x83,	/* Endpoint address */
            0x02,	/* Endpoint attributes */
            0x10,	/* Max. packet size */
            0,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep4 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	/* Endpoint descriptor type  */
            0x04,	/* Endpoint address */
            0x02,	/* Endpoint attributes */
            0x10,	/* Max. packet size */
            0,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep5 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	/* Endpoint descriptor type  */
            0x85,	/* Endpoint address */
            0x02,	/* Endpoint attributes */
            0x20,	/* Max. packet size */
            1,	/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep6 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	/* Endpoint descriptor type  */
            0x86,	/* Endpoint address */
            0x01,	/* Endpoint attributes */
            0x00,	/* Max. packet size */
            0x01,	/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep7 */
        {
            USB_DESC_LEN_ENDPOINT,		/* Size of this descriptor */
            USB_DESC_TYPE_ENDPOINT,	/* Endpoint descriptor type  */
            0x07,	/* Endpoint address */
            0x02,	/* Endpoint attributes */
            0x40,	/* Max. packet size */
            0,		/* Interval for polling endpoint for data transfer */
        }